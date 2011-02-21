#ifndef DUNE_SPGRID_GRIDLEVEL_HH
#define DUNE_SPGRID_GRIDLEVEL_HH

#include <cassert>
#include <vector>

#include <dune/common/forloop.hh>

#include <dune/grid/spgrid/referencecube.hh>
#include <dune/grid/spgrid/misc.hh>
#include <dune/grid/spgrid/refinement.hh>
#include <dune/grid/spgrid/domain.hh>
#include <dune/grid/spgrid/mesh.hh>
#include <dune/grid/spgrid/partitionpool.hh>
#include <dune/grid/spgrid/linkage.hh>
#include <dune/grid/spgrid/decomposition.hh>
#include <dune/grid/spgrid/geometrycache.hh>

namespace Dune
{

  // External Forward Declarations
  // -----------------------------

  template< int mydim, int cdim, class Grid >
  class SPLocalGeometry;



  // SPGeometricGridLevel
  // ----------------

  template< class Grid >
  class SPGeometricGridLevel
  {
    typedef SPGeometricGridLevel< Grid > This;

  public:
    typedef typename remove_const< Grid >::type::Traits Traits;

    typedef typename Traits::ReferenceCube ReferenceCube;

    typedef typename ReferenceCube::ctype ctype;
    static const int dimension = ReferenceCube::dimension;

    typedef typename ReferenceCube::GlobalVector GlobalVector;

    static const unsigned int numDirections = ReferenceCube::numCorners;

    template< int codim >
    struct Codim
    {
      typedef SPReferenceCube< ctype, dimension-codim > ReferenceCube;
      typedef SPGeometryCache< ctype, dimension, codim > GeometryCache;
    };

  private:
    template< int codim >
    struct BuildGeometryCache;
    template< int codim >
    struct DestroyGeometryCache;

  public:
    SPGeometricGridLevel ( const Grid &grid, const GlobalVector &h );
    SPGeometricGridLevel ( const This &other );

    ~SPGeometricGridLevel ();

    const Grid &grid () const { return *grid_; }

    const ReferenceCube &referenceCube () const { return grid().referenceCube(); }

    template< int codim >
    const typename Codim< codim >::ReferenceCube &
    referenceCube () const { return grid().template referenceCube< codim >(); }

    const GlobalVector &h () const { return h_; }

    template< int codim >
    const typename Codim< codim >::GeometryCache &
    geometryCache ( const unsigned int dir ) const;

    ctype faceVolume ( const int i ) const;
    const GlobalVector &volumeNormal ( const int i ) const;

  private:
    void buildGeometry ();

    const Grid *grid_;

    GlobalVector h_;
    void *geometryCache_[ numDirections ];
    ctype faceVolume_[ ReferenceCube::numFaces ];
    GlobalVector normal_[ ReferenceCube::numFaces ];
  };



  // SPGeometricGridLevel::BuildGeometryCache
  // ------------------------------------

  template< class Grid >
  template< int codim >
  struct SPGeometricGridLevel< Grid >::BuildGeometryCache
  {
    static void
    apply ( const GlobalVector &h, void *(&geometryCache)[ 1 << dimension ] )
    {
      typedef typename Codim< codim >::GeometryCache GeometryCache;
      for( unsigned int dir = 0; dir < (1 << dimension); ++dir )
      {
        const int mydim = bitCount( dir );
        if( mydim == dimension - codim )
          geometryCache[ dir ] = new GeometryCache( h, dir );
      }
    }
  };



  // SPGeometricGridLevel::DestroyGeometryCache
  // --------------------------------------

  template< class Grid >
  template< int codim >
  struct SPGeometricGridLevel< Grid >::DestroyGeometryCache
  {
    static void
    apply ( void *(&geometryCache)[ 1 << dimension ] )
    {
      typedef typename Codim< codim >::GeometryCache GeometryCache;
      for( unsigned int dir = 0; dir < (1 << dimension); ++dir )
      {
        const int mydim = bitCount( dir );
        if( mydim == dimension - codim )
        {
          delete (GeometryCache *)geometryCache[ dir ];
          geometryCache[ dir ] = 0;
        }
      }
    }
  };



  // SPGridLevel
  // -----------

  template< class Grid >
  class SPGridLevel
  : public SPGeometricGridLevel< Grid >
  {
    typedef SPGridLevel< Grid > This;
    typedef SPGeometricGridLevel< Grid > Base;

  public:
    typedef SPGridLevel< Grid > GridLevel;

    typedef typename Base::Traits Traits;
    typedef typename Base::ReferenceCube ReferenceCube;
    typedef typename Base::ctype ctype;

    static const int dimension = Base::dimension;
    static const unsigned int numDirections = Base::numDirections;

    typedef typename Traits::Domain Domain;
    typedef typename Traits::Refinement Refinement;
    typedef typename Traits::RefinementPolicy RefinementPolicy;

    typedef typename ReferenceCube::GlobalVector GlobalVector;
    typedef typename ReferenceCube::MultiIndex MultiIndex;

    typedef SPDecomposition< dimension > Decomposition;
    typedef SPPartitionPool< dimension > PartitionPool;
    typedef SPLinkage< dimension > Linkage;

    typedef typename Decomposition::Mesh Mesh;

    typedef typename PartitionPool::PartitionList PartitionList;

    typedef typename Linkage::Interface CommInterface;

    typedef typename Traits::template Codim< 0 >::LocalGeometry LocalGeometry;

  private:
    typedef SPLocalGeometry< dimension, dimension, Grid > LocalGeometryImpl;

  public:
    SPGridLevel ( const Grid &grid, const Decomposition &decomposition );
    SPGridLevel ( const GridLevel &father, const RefinementPolicy &policy );
    SPGridLevel ( const This &other );

    ~SPGridLevel ();

    using Base::grid;
    using Base::referenceCube;

    int level () const { return level_; }
    const Domain &domain () const { return domain_; }
    const Refinement &refinement () const { return refinement_; }

    const Mesh &globalMesh () const;
    const Mesh &localMesh () const;

    template< PartitionIteratorType pitype >
    const PartitionList &partition () const;

    template< int codim >
    PartitionType
    partitionType ( const MultiIndex &id, const unsigned int partitionNumber ) const;

    const CommInterface &commInterface ( const InterfaceType iftype ) const;

    MultiIndex macroId ( const MultiIndex &id ) const;

    size_t boundaryIndex ( const MultiIndex &id,
                           const unsigned int partitionNumber,
                           const int face ) const;

    const LocalGeometry &geometryInFather ( const MultiIndex &id ) const;

    int size () const;

  private:
    void buildLocalGeometry ();

    static MultiIndex coarseMacroFactor ();
    static GlobalVector meshWidth ( const Domain &domain, const Mesh &mesh );

    MultiIndex overlap () const;

    int level_;

    const Refinement refinement_;
    MultiIndex macroFactor_;
    Domain domain_;
    std::vector< Mesh > decomposition_;
    Mesh localMesh_;
    PartitionPool partitionPool_;
    Linkage linkage_;

    LocalGeometry **geometryInFather_;
  };



  // Implementation of SPGeometricGridLevel
  // ----------------------------------

  template< class Grid >
  inline SPGeometricGridLevel< Grid >
    ::SPGeometricGridLevel ( const Grid &grid, const GlobalVector &h )
  : grid_( &grid ),
    h_( h )
  {
    buildGeometry();
  }


  template< class Grid >
  inline SPGeometricGridLevel< Grid >::SPGeometricGridLevel ( const This &other )
  : grid_( other.grid_ ),
    h_( other.h_ )
  {
    buildGeometry();
  }


  template< class Grid >
  inline SPGeometricGridLevel< Grid >::~SPGeometricGridLevel ()
  {
    ForLoop< DestroyGeometryCache, 0, dimension >::apply( geometryCache_ );
  }


  template< class Grid >
  template< int codim >
  inline const typename SPGeometricGridLevel< Grid >::template Codim< codim >::GeometryCache &
  SPGeometricGridLevel< Grid >::geometryCache ( const unsigned int dir ) const
  {
    typedef typename Codim< codim >::GeometryCache GeometryCache;
    assert( bitCount( dir ) == dimension - codim );
    return *((const GeometryCache *)geometryCache_[ dir ]);
  }


  template< class Grid >
  inline typename SPGeometricGridLevel< Grid >::ctype
  SPGeometricGridLevel< Grid >::faceVolume ( const int i ) const
  {
    assert( (i >= 0) && (i < ReferenceCube::numFaces) );
    return faceVolume_[ i ];
  }


  template< class Grid >
  inline const typename SPGeometricGridLevel< Grid >::GlobalVector &
  SPGeometricGridLevel< Grid >::volumeNormal ( const int i ) const
  {
    assert( (i >= 0) && (i < ReferenceCube::numFaces) );
    return normal_[ i ];
  }


  template< class Grid >
  inline void SPGeometricGridLevel< Grid >::buildGeometry ()
  {
    ForLoop< BuildGeometryCache, 0, dimension >::apply( h_, geometryCache_ );
    
    const ctype volume = geometryCache< 0 >( numDirections-1 ).volume();
    for( int face = 0; face < ReferenceCube::numFaces; ++face )
    {
      normal_[ face ] = referenceCube().normal( face );
      faceVolume_[ face ] = std::abs( volume / (normal_[ face ] * h_) );
      normal_[ face ] *= faceVolume_[ face ];
    }
  }



  // Implementation of SPGridLevel
  // -----------------------------

  template< class Grid >
  inline SPGridLevel< Grid >
    ::SPGridLevel ( const Grid &grid, const Decomposition &decomposition )
  : Base( grid, meshWidth( grid.domain(), decomposition.mesh() ) ),
    level_( 0 ),
    refinement_(),
    macroFactor_( coarseMacroFactor() ),
    domain_( grid.domain() ),
    decomposition_( decomposition.subMeshes() ),
    localMesh_( decomposition_[ grid.comm().rank() ] ),
    partitionPool_( localMesh_, decomposition.mesh(), overlap(), domain_.topology() ),
    linkage_( grid.comm().rank(), partitionPool_, decomposition_ )
  {
    buildLocalGeometry();
  }


  template< class Grid >
  inline SPGridLevel< Grid >
    ::SPGridLevel ( const GridLevel &father, const RefinementPolicy &policy )
  : Base( father.grid(), meshWidth( father.domain(), Refinement( father.refinement(), policy )( father.globalMesh() ) ) ),
    level_( father.level() + 1 ),
    refinement_( father.refinement(), policy ),
    macroFactor_( father.macroFactor_ * refinement_ ),
    domain_( father.domain() ),
    decomposition_( refinement_( father.decomposition_ ) ),
    localMesh_( refinement_( father.localMesh() ) ),
    partitionPool_( localMesh_, refinement_( father.globalMesh() ), overlap(), domain_.topology() ),
    linkage_( father.grid().comm().rank(), partitionPool_, decomposition_ )
  {
    buildLocalGeometry();
  }


  template< class Grid >
  inline SPGridLevel< Grid >::SPGridLevel ( const This &other )
  : Base( other ),
    refinement_( other.refinement_ ),
    macroFactor_( other.macroFactor_ ),
    domain_( other.domain_ ),
    decomposition_( other.decomposition_ ),
    localMesh_( other.localMesh_ ),
    partitionPool_( other.partitionPool_ ),
    linkage_( other.linkage_ )
  {
    buildLocalGeometry();
  }


  template< class Grid >
  inline SPGridLevel< Grid >::~SPGridLevel ()
  {
    if( geometryInFather_ )
    {
      unsigned int numChildren = refinement().numChildren();
      for( unsigned int index = 0; index < numChildren; ++index )
        delete geometryInFather_[ index ];
      delete geometryInFather_;
    }
  }

  template< class Grid >
  inline const typename SPGridLevel< Grid >::Mesh &
  SPGridLevel< Grid >::globalMesh () const
  {
    return partitionPool_.globalMesh();
  }


  template< class Grid >
  inline const typename SPGridLevel< Grid >::Mesh &
  SPGridLevel< Grid >::localMesh () const
  {
    return localMesh_;
  }


  template< class Grid >
  template< PartitionIteratorType pitype >
  inline const typename SPGridLevel< Grid >::PartitionList &
  SPGridLevel< Grid >::partition () const
  {
    return partitionPool_.template get< pitype >();
  }


  template< class Grid >
  template< int codim >
  inline PartitionType SPGridLevel< Grid >
    ::partitionType ( const MultiIndex &id, const unsigned int partitionNumber ) const
  {
    return partitionPool_.template partitionType< codim >( id, partitionNumber );
  }


  template< class Grid >
  inline const typename SPGridLevel< Grid >::CommInterface &
  SPGridLevel< Grid >::commInterface ( const InterfaceType iftype ) const
  {
    return linkage_.interface( iftype );
  }


  template< class Grid >
  inline typename SPGridLevel< Grid >::MultiIndex
  SPGridLevel< Grid >::macroId ( const MultiIndex &id ) const
  {
    MultiIndex macroId;
    for( int i = 0; i < dimension; ++i )
      macroId[ i ] = (((id[ i ] >> 1) / macroFactor_[ i ]) << 1) | (id[ i ] & 1);
    return macroId;
  }


  template< class Grid >
  inline size_t SPGridLevel< Grid >
    ::boundaryIndex ( const MultiIndex &id,
                      const unsigned int partitionNumber,
                      const int face ) const
  {
    // note: boundaryIndex ignores the last bit of macroId,
    //       hence we can use this fast computation
    MultiIndex macroId;
    for( int i = 0; i < dimension; ++i )
      macroId[ i ] = id[ i ] / macroFactor_[ i ];
    return grid().boundaryIndex( macroId, partitionNumber, face );
  }


  template< class Grid >
  inline const typename SPGridLevel< Grid >::LocalGeometry &
  SPGridLevel< Grid >::geometryInFather ( const MultiIndex &id ) const
  {
    assert( (level() > 0) && (geometryInFather_ != 0) );
    return *(geometryInFather_[ refinement().childIndex( id ) ]);
  }


  template< class Grid >
  inline int SPGridLevel< Grid >::size () const
  {
    return globalMesh().volume();
  }


  template< class Grid >
  inline void SPGridLevel< Grid >::buildLocalGeometry ()
  {
    geometryInFather_ = 0;
    if( level() > 0 )
    {
      const unsigned int numChildren = refinement().numChildren();
      geometryInFather_ = new LocalGeometry *[ numChildren ];
      const GlobalVector hInFather = refinement().template hInFather< ctype >();
      const typename Base::template Codim< 0 >::GeometryCache cacheInFather( hInFather, numDirections-1 );
      for( unsigned int index = 0; index < numChildren; ++index )
      {
        const GlobalVector origin = refinement().template originInFather< ctype >( index );
        geometryInFather_[ index ] = new LocalGeometry( LocalGeometryImpl( referenceCube(), cacheInFather, origin ) );
      }
    }
  }


  template< class Grid >
  inline typename SPGridLevel< Grid >::MultiIndex
  SPGridLevel< Grid >::coarseMacroFactor ()
  {
    MultiIndex macroFactor;
    for( int i = 0; i < dimension; ++i )
      macroFactor[ i ] = 1;
    return macroFactor;
  }


  template< class Grid >
  inline typename SPGridLevel< Grid >::GlobalVector
  SPGridLevel< Grid >::meshWidth ( const Domain &domain, const Mesh &mesh )
  {
    GlobalVector h = domain.cube().width();
    const MultiIndex meshWidth = mesh.width();
    for( int i = 0; i < dimension; ++i )
      h[ i ] /= ctype( meshWidth[ i ] );
    return h;
  }


  template< class Grid >
  inline typename SPGridLevel< Grid >::MultiIndex
  SPGridLevel< Grid >::overlap () const
  {
    MultiIndex overlap;
    for( int i = 0; i < dimension; ++i )
      overlap[ i ] = macroFactor_[ i ] * grid().overlap()[ i ];
    return overlap;
  }

} // namespace Dune

#endif // #ifndef DUNE_SPGRID_GRIDLEVEL_HH
