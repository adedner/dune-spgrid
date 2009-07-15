#ifndef DUNE_SPGRID_GRIDLEVEL_HH
#define DUNE_SPGRID_GRIDLEVEL_HH

#include <cassert>
#include <vector>

#include <dune/common/smartpointer.hh>

#include <dune/grid/genericgeometry/misc.hh>

#include <dune/grid/spgrid/cube.hh>
#include <dune/grid/spgrid/misc.hh>
#include <dune/grid/spgrid/domain.hh>

namespace Dune
{

  template< class ct, int dim >
  class SPGridLevel
  {
    typedef SPGridLevel< ct, dim > This;

  public:
    typedef SPCube< ct, dim > Cube;
    typedef SPDomain< ct, dim > Domain;
    typedef SPGridLevel< ct, dim > GridLevel;

    typedef typename Cube::ctype ctype;
    static const int dimension = Cube::dimension;

    typedef typename Cube::GlobalVector GlobalVector;
    typedef typename Cube::MultiIndex MultiIndex;

    static const unsigned int numDirections = Cube::numCorners;

    template< int codim >
    struct GeometryCache;
    
    SPGridLevel ( const Domain &domain, const MultiIndex &n );

    SPGridLevel ( GridLevel &father, const unsigned int refDir );

  private:
    SPGridLevel ( const This &other );

  public:
    ~SPGridLevel ()
    {
      for( unsigned int dir = 0; dir < numDirections; ++dir )
        delete geometryCache_[ dir ];
    }

    const Domain &domain () const
    {
      return *domain_;
    }

    const Cube &cube () const
    {
      return *cube_;
    }

    const GridLevel &fatherLevel () const
    {
      assert( father_ != 0 );
      return *father_;
    }

    const GridLevel &childLevel () const
    {
      assert( !isLeaf() );
      return *child_;
    }

    bool isLeaf () const
    {
      return (child_ == 0);
    }

    const GlobalVector &h () const
    {
      return h_;
    }

    unsigned int level () const
    {
      return level_;
    }

    unsigned int refinementDirection () const
    {
      return refDir_;
    }

    const MultiIndex &cells () const
    {
      return cells_;
    }

    const MultiIndex begin ( const unsigned int dir ) const
    {
      // ...
    }

    const MultiIndex end ( const unsigned int dir ) const
    {
      // ...
    }

    template< int codim >
    const GeometryCache< codim > &geometryCache ( const unsigned int dir ) const
    {
      assert( bitcount( dir ) == dimension - codim );
      return (const GeometryCache< codim > &)( *geometryCache_[ dir ] );
    }

    const GlobalVector &volumeNormal ( const int i ) const
    {
      assert( (i >= 0) && (i < Cube::numFaces) );
      return normal_[ i ];
    }

  private:
    void buildGeometry ();

    GridLevel *father_;
    GridLevel *child_;
    SmartPointer< const Cube > cube_;
    const Domain *domain_;
    unsigned int level_;
    unsigned int refDir_;
    MultiIndex cells_;
    GlobalVector h_;
    void *geometryCache_[ numDirections ];
    GlobalVector normal_[ Cube::numFaces ];
  };


  template< class ct, int dim >
  inline SPGridLevel< ct, dim >
    ::SPGridLevel ( const Domain &domain, const MultiIndex &n )
  : father_( 0 ),
    child_( 0 ),
    domain_( &domain ),
    level_( 0 ),
    refDir_( 0 )
  {
    const GlobalVector &width  = domain.width();
    for( int i = 0; i < dimension; ++i )
    {
      cells_[ i ] = n[ i ];
      h_[ i ] = width[ i ] / (ctype)n[ i ];
    }
    buildGeometry();
  }


  template< class ct, int dim >
  inline SPGridLevel< ct, dim >
    ::SPGridLevel ( GridLevel &father, const unsigned int refDir )
  : father_( &father ),
    child_( 0 ),
    domain_( father.domain_ ),
    cube_( father.cube_ ),
    level_( father.level_ + 1 ),
    refDir_( refDir )
  {
    assert( father.child_ == 0 );
    father.child_ = *this;
    for( int i = 0; i < dimension; ++i )
    {
      unsigned int factor = 2*((refDir >> i) & 1);
      cells_[ i ] = factor * father.cells_[ i ];
      h_[ i ] = father.h_[ i ] / ctype( factor );
    }
    buildGeometry();
  }


  template< class ct, int dim >
  inline void SPGridLevel< ct, dim >::buildGeometry ()
  {
    GenericGeometry::ForLoop< GeometryCache, 0, dimension >::apply( h_, geometryCache_ );

    const ctype volume = geometryCache( numDirections-1 ).volume();
    for( int face = 0; face < Cube::numFaces; ++face )
    {
      normal_[ face ] = cube().normal( face );
      const ctype hn = std::abs( normal_[ face ] * h_ );
      normal_[ face ] *= volume / hn;
    }
  }



  // SPGridLevel::GeometryCache
  // --------------------------

  template< class ct, int dim >
  template< int codim >
  class SPGridLevel< ct, dim >::GeometryCache
  {
    typedef GeometryCache< codim > This;

    friend class SPGridLevel< ct, dim >;

  public:
    static const int codimension = codim;
    static const int mydimension = dimension - codimension;

    typedef FieldVector< ctype, mydimension > LocalVector;
    typedef FieldMatrix< ctype, dimension, mydimension > Jacobian;
    typedef FieldMatrix< ctype, mydimension, dimension > JacobianTransposed;

  private:
    static void
    apply ( const GlobalVector &h, void *(&geometryCache)[ 1 << dimension ] )
    {
      for( unsigned int dir = 0; dir < (1 << dimension); ++dir )
      {
        const int mydim = bitcount( dir );
        if( mydim == dimension - codim )
          geometryCache[ dir ] = new This( h, dir );
      }
    }

    GeometryCache ( const GlobalVector &h, const unsigned int dir )
    : volume_( 1 ),
      jacobianTransposed_( 0 ),
      jacobianInverseTransposed_( 0 )
    {
      int k = 0;
      for( int j = 0; j < dimension; ++j )
      {
        if( ((dir >> j) & 1) == 0 )
          continue;
        volume_ *= h[ j ];
        jacobianTransposed_[ j ][ k ] = h[ j ];
        jacobianInverseTransposed_[ k ][ j ] = ctype( 1 ) / h[ j ];
        ++k;
      }
    }

  public:
    const ctype &volume () const
    {
      return volume_;
    }

    const JacobianTransposed &jacobianTransposed () const
    {
      return jacobianTransposed_;
    }

    const Jacobian &jacobianInverseTransposed () const
    {
      return jacobianInverseTransposed_;
    }

  private:
    ctype volume_;
    JacobianTransposed jacobianTransposed_;
    Jacobian jacobianInverseTransposed_;
  };

}

#endif // #ifndef DUNE_SPGRID_GRIDLEVEL_HH
