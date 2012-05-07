#ifndef DUNE_SPGRID_REFERENCECUBE_HH
#define DUNE_SPGRID_REFERENCECUBE_HH

#include <vector>

#include <dune/common/fvector.hh>

#include <dune/geometry/genericgeometry/codimtable.hh>

#include <dune/grid/spgrid/multiindex.hh>
#include <dune/grid/spgrid/normal.hh>

namespace Dune
{

  namespace SPReferenceCubeHelper
  {

    inline unsigned int
    numSubEntities ( const unsigned int dimension, const unsigned int codim )
    {
      assert( codim <= dimension );
      if( codim > 0 )
      {
        const unsigned int n0 = (codim < dimension ? numSubEntities( dimension-1, codim ) : 0);
        const unsigned int n1 = numSubEntities( dimension-1, codim-1 );
        return n0 + 2*n1;
      }
      else
        return 1;
    }

  } // namespace SPReferenceCubeHelper



  // SPReferenceCube
  // ---------------

  template< class ct, int dim >
  class SPReferenceCube
  {
    typedef SPReferenceCube< ct, dim > This;

  public:
    typedef ct ctype;

    static const int dimension = dim;

    typedef FieldVector< ctype, dimension > GlobalVector;
    typedef SPNormalVector< ctype, dimension > NormalVector;
    typedef SPMultiIndex< dimension > MultiIndex;

    static const int numCorners = (1 << dimension);
    static const int numFaces = 2*dimension;

    SPReferenceCube ();

    const MultiIndex &subId ( const int codim, const int i ) const
    {
      assert( (i >= 0) && (i < count( codim )) );
      return subId_[ codim ][ i ];
    }

    int count ( const int codim ) const
    {
      assert( (codim >= 0) && (codim <= dimension) );
      return subId_[ codim ].size();
    }

    /** \brief return i-th corner */
    static GlobalVector corner ( int i );

    /** \brief return center */
    static GlobalVector center ();


    NormalVector normal ( const int i ) const
    {
      assert( (i >= 0) && (i < numFaces) );
      NormalVector normal( i / 2, 2*(i&1)-1 );
      assert( normal == normal_[ i ] );
      return normal;
    }

  private:
    void subId ( const unsigned int dimension, const unsigned int codim,
                 const unsigned int i, MultiIndex &sId ) const
    {
      using SPReferenceCubeHelper::numSubEntities;
      assert( i < numSubEntities( dimension, codim ) );
      if( dimension == 0 )
        return;

      const unsigned int n0 = (codim < dimension ? numSubEntities( dimension-1, codim ) : 0);
      if( i < n0 )
      {
        subId( dimension-1, codim, i, sId );
        sId[ dimension-1 ] = 0;
      }
      else
      {
        const unsigned int n1 = numSubEntities( dimension-1, codim-1 );
        subId( dimension-1, codim-1, (i-n0)%n1, sId );
        sId[ dimension-1 ] = 2*((i-n0)/n1) - 1;
      }
    }

    std::vector< MultiIndex > subId_[ dimension+1 ];
    GlobalVector normal_[ numFaces ];
  };



  template< class ct, int dim >
  inline SPReferenceCube< ct, dim >::SPReferenceCube ()
  {
    for( int codim = 0; codim <= dimension; ++codim )
    {
      const unsigned int size = SPReferenceCubeHelper::numSubEntities( dimension, codim );
      subId_[ codim ].resize( size );
      for( unsigned int i = 0; i < size; ++i )
        subId( dimension, codim, i, subId_[ codim ][ i ] );
    }

    for( int i = 0; i < numFaces; ++i )
    {
      for( int j = 0; j < dimension; ++j )
      {
        const MultiIndex &sid = subId( 1, i );
        normal_[ i ][ j ] = ctype( sid[ j ] );
      }
    }
  }


  template< class ct, int dim >
  inline typename SPReferenceCube< ct, dim >::GlobalVector
  SPReferenceCube< ct, dim >::corner ( int i )
  {
    assert( (i >= 0) && (i < numCorners) );

    GlobalVector corner;
    for( int j = 0; j < dimension; ++j )
    {
      corner[ j ] = ctype( i & 1 );
      i /= 2;
    }
    return corner;
  }


  template< class ct, int dim >
  inline typename SPReferenceCube< ct, dim >::GlobalVector
  SPReferenceCube< ct, dim >::center ()
  {
    GlobalVector center;
    for( int j = 0; j < dimension; ++j )
      center[ j ] = ctype( 1 ) / ctype( 2 );
    return center;
  }



  // SPReferenceCube (for dim = 0)
  // -----------------------------

  template< class ct >
  class SPReferenceCube< ct, 0 >
  {
    typedef SPReferenceCube< ct, 0 > This;

  public:
    typedef ct ctype;

    static const int dimension = 0;

    typedef FieldVector< ctype, 0 > GlobalVector;
    typedef SPMultiIndex< dimension > MultiIndex;

    static const int numCorners = 1;

    int count ( const int codim ) const
    {
      assert( (codim >= 0) && (codim <= dimension) );
      return 1;
    }

    static GlobalVector corner ( const int i )
    {
      assert( (i >= 0) && (i < numCorners) );
      return GlobalVector();
    }

    static GlobalVector center ()
    {
      return GlobalVector();
    }
  };



  // SPReferenceCubeContainer
  // ------------------------

  template< class ct, int dim >
  class SPReferenceCubeContainer
  {
    typedef SPReferenceCubeContainer< ct, dim > This;

    // friend class SPIntersection< const This >;
    // friend class SPGridLevel< const This >;

  public:
    typedef SPReferenceCube< ct, dim > ReferenceCube;

    typedef typename ReferenceCube::ctype ctype;

    static const int dimension = ReferenceCube::dimension;

    template< int codim >
    struct Codim
    {
      typedef SPReferenceCube< ct, dim-codim > ReferenceCube;
    };

  public:
    const ReferenceCube &get () const
    {
      return get< 0 >();
    }

    template< int codim >
    const typename Codim< codim >::ReferenceCube &get () const
    {
      integral_constant< int, codim > codimVariable;
      return refCubes_[ codimVariable ];
    }

  private:
    template< int codim >
    struct RefCube
    : public Codim< codim >::ReferenceCube
    {};
  
    GenericGeometry::CodimTable< RefCube, dimension > refCubes_;
  };

} // namespace Dune

#endif // #ifndef DUNE_SPGRID_REFERENCECUBE_HH
