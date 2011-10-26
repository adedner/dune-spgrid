#ifndef DUNE_CARTESIANGRID_HOSTGRIDINFO_HH
#define DUNE_CARTESIANGRID_HOSTGRIDINFO_HH

#include <dune/common/fvector.hh>

#if ENABLE_ALUGRID
#include <dune/grid/alugrid/3d/grid.hh>
#endif // #if ENABLE_ALUGRID

namespace Dune
{

  template <class HostGrid> 
  struct CartesianGridHostGridInfo
  {
    //////////////////////////////////////////////////
    //  direction methods 
    //////////////////////////////////////////////////
    
    //! default direction for given dimension 
    static unsigned int defaultDirection (const int mydimension)
    {
      return (1 << mydimension)-1;
    }

    //! direction for face i 
    static unsigned int direction ( const int i, const int dimension )
    {
      assert( (i >= 0) && (i < 2*dimension) );
      return (1 << (i / 2)) ^ ((1 << dimension) - 1);
    }

    //! direction for given host entity
    template< class HostEntity >
    static unsigned int direction ( const HostEntity &hostEntity )
    {
      typedef typename HostEntity::Geometry HostGeometry;
      unsigned int direction = 0;
      const HostGeometry &geo = hostEntity.geometry();
      const typename HostGeometry::GlobalCoordinate origin = geo.corner( 0 );
      for( int d = 0; d <HostGeometry::mydimension; ++d )
      {
        const typename HostGeometry::GlobalCoordinate point = geo.corner( 1 << d );
        for( int i = 0; i < HostGeometry::coorddimension; ++i )
        {
          if( std::abs( point[ i ] - origin[ i ] ) > 1e-8 )
            direction |= (1 << i);
        }
      }
      return direction;
    }

    //////////////////////////////////////////////////////
    //  origin methods 
    //////////////////////////////////////////////////////
    template< int codim >
    struct Codim
    {
      // maybe we have to change this 
      typedef typename HostGrid :: template Codim < codim > :: Geometry :: GlobalCoordinate  OriginReturnType ; 
    };

    //! default origin 
    static typename Codim< 0 > :: OriginReturnType  
    defaultOrigin ()
    {
      return typename Codim< 0 > :: OriginReturnType( 0 );
    }

    //! origin for given entity or intersection 
    template <class HostItem>
    static typename Codim< HostItem::codimension > :: OriginReturnType  
    origin ( const HostItem &hostItem )
    {
      return hostItem.geometry().corner( 0 );
    }

    //! origin for given intersection 
    template <class HostIntersection>
    static typename Codim< HostIntersection::codimension > :: OriginReturnType  
    originIntersection ( const HostIntersection& hostIntersection )
    {
      typedef typename HostIntersection::Geometry Geometry;
      const Geometry &geo = hostIntersection.geometry();
      assert( geo.corners() == (1 << Geometry::mydimension) );
      const typename Geometry::GlobalCoordinate a = geo.corner( 0 );
      const typename Geometry::GlobalCoordinate b = geo.corner( (1 << Geometry::mydimension)-1 );
      typename Geometry::GlobalCoordinate origin;
      for( int i = 0; i < Geometry::coorddimension; ++i )
        origin[ i ] = std::min( a[ i ], b[ i ] );
      return origin;
    }

    //////////////////////////////////////////////////////////
    //  child index methods 
    //////////////////////////////////////////////////////////

    //! return child index for entity 
    template <class HostEntity>
    static int childIndex ( const HostEntity &hostEntity )
    {
      int childIndex = 0;
      typedef typename HostGrid :: template Codim < HostEntity :: codimension > :: LocalGeometry  LocalGeometry; 
      typedef  typename LocalGeometry :: GlobalCoordinate GlobalCoordinate;
      const GlobalCoordinate centerInFather
        = hostEntity.geometryInFather().center();
      for( int i = 0; i <GlobalCoordinate::dimension; ++i )
        childIndex |= (centerInFather[ i ] < 0.5 ? 0 : (1 << i));
      return childIndex;
    }


    //////////////////////////////////////////////////////////
    //  level methods 
    //////////////////////////////////////////////////////////

    //! return inside level for intersection 
    template <class HostIntersection>
    static int insideLevel ( const HostIntersection &hostIntersection )
    {
      return hostIntersection.inside().level();
    }

    //! return outside level for intersection 
    template <class HostIntersection>
    static int outsideLevel ( const HostIntersection &hostIntersection, 
                              const int insideLevel )
    {
      return hostIntersection.neighbor() ? hostIntersection.outside().level() : insideLevel; 
    }

    template <class HostIntersection>
    static int childIndexInInside ( const HostIntersection &hostIntersection,
                                    const int insideLevel,
                                    const int outsideLevel ) 
    {
      return getIntersectionChildLevel( hostIntersection.geometryInInside(),
                                        hostIntersection.indexInInside(),
                                        insideLevel,
                                        outsideLevel );
    }

    template <class HostIntersection>
    static int childIndexInOutside ( const HostIntersection &hostIntersection,
                                     const int insideLevel,
                                     const int outsideLevel ) 
    {
      return getIntersectionChildLevel( hostIntersection.geometryInOutside(),
                                        hostIntersection.indexInOutside(),
                                        outsideLevel, 
                                        insideLevel );
    }

  protected:  
    template <class LocalGeometry>
    static int getIntersectionChildLevel( const LocalGeometry& localGeo,
                                          const int index,
                                          const int myLevel, 
                                          const int otherLevel )
    {
      typedef typename LocalGeometry :: GlobalCoordinate GlobalCoordinate;
      // make sure non-confoming level difference is at most 1 
      assert( std::abs( myLevel - otherLevel ) <= 1 );
      if( myLevel < otherLevel )
      {
        int childIndex = 0;
        const GlobalCoordinate center = localGeo.center();
        for( int i = 0; i < LocalGeometry::mydimension; ++i )
        {
          const int j = (i < index/2 ? i : i+1 );
          childIndex |= (center[ j ] < 0.5 ? 0 : (1 << i));
        }
        return childIndex;
      }
      else
        return -1;
    }
  };


  ////////////////////////////////////////////////////////////////
  //
  //  Specilization for ALUCubeGrid 
  //
  ////////////////////////////////////////////////////////////////
  
#if ENABLE_ALUGRID
  template< int dimension, int codimension >
  struct ALU3dCubeOrigin
  {
    typedef ALUCubeGrid< dimension, dimension > HostGrid;

    typedef typename HostGrid::template Codim< codimension >::Geometry::GlobalCoordinate OriginReturnType;

    template< class Item >
    static OriginReturnType get ( const Item &item )
    {
      return item.geometry().corner( 0 );
    }
  };

  // for vertices 
  template< int dimension >
  struct ALU3dCubeOrigin< dimension, dimension >
  {
    typedef ALUCubeGrid< dimension, dimension > HostGrid;

    typedef double coord_t[ dimension ];
    typedef const coord_t &OriginReturnType;

    template< class Item >
    static OriginReturnType get ( const Item &item )
    {
      return HostGrid::getRealImplementation( item ).getItem().Point();
    }
  };

  // for elements 
  template< int dimension >
  struct ALU3dCubeOrigin< dimension, 0 >
  {
    typedef ALUCubeGrid< dimension, dimension > HostGrid;

    typedef double coord_t[ dimension ];
    typedef const coord_t &OriginReturnType;

    template< class Item >
    static OriginReturnType get ( const Item &item )
    {
      return HostGrid::getRealImplementation( item ).getItem().myvertex( 0 )->Point();
    }
  };

  // specialization for intersections 
  template< int dimension >
  struct ALU3dCubeOrigin< dimension, -1 >
  {
    typedef ALUCubeGrid< dimension, dimension > HostGrid;

    typedef double coord_t[ dimension ];
    typedef const coord_t &OriginReturnType;

    template< class Intersection >
    static OriginReturnType get( const Intersection &intersection )
    {
#ifndef NDEBUG 
      typedef FaceTopologyMapping< hexa > CubeFaceMapping;
      const int duneTwist = HostGrid::getRealImplementation( intersection ).twistInInside();
      const int twistedIndex = CubeFaceMapping::twistedDuneIndex( 0, duneTwist );
      assert( twistedIndex == 0 );
#endif
      return HostGrid::getRealImplementation( intersection ).it().getItem().myvertex( 0 )->Point();
    }
  };



  template<> 
  struct CartesianGridHostGridInfo< ALUCubeGrid< 3, 3 > >
  {
    static const int dimension = 3;

    typedef ALUCubeGrid< 3, 3 > HostGrid; 

    //////////////////////////////////////////////////
    //  direction methods 
    //////////////////////////////////////////////////
    
    //! default direction for given dimension 
    static unsigned int defaultDirection (const int mydimension)
    {
      return (1 << mydimension)-1;
    }

    //! direction for face i 
    static unsigned int direction ( const int i, const int dimension )
    {
      assert( (i >= 0) && (i < 2*dimension) );
      return (1 << (i / 2)) ^ ((1 << dimension) - 1);
    }

    //! direction for given host entity
    template< class HostEntity >
    static unsigned int direction ( const HostEntity& hostEntity )
    {
      typedef typename HostEntity::Geometry HostGeometry;
      unsigned int direction = 0;
      const HostGeometry &geo = hostEntity.geometry();
      const typename HostGeometry::GlobalCoordinate origin = geo.corner( 0 );
      for( int d = 0; d <HostGeometry::mydimension; ++d )
      {
        const typename HostGeometry::GlobalCoordinate point = geo.corner( 1 << d );
        for( int i = 0; i < HostGeometry::coorddimension; ++i )
        {
          if( std::abs( point[ i ] - origin[ i ] ) > 1e-8 )
            direction |= (1 << i);
        }
      }
      return direction;
    }

    //! default origin
    static FieldVector< double, dimension > defaultOrigin ()
    {
      return FieldVector< double, dimension >( 0 );
    }

    //! origin for given entity or intersection 
    template< class HostItem >
    static typename ALU3dCubeOrigin< HostItem::dimension, HostItem::codimension >::OriginReturnType
    origin ( const HostItem &hostItem )
    {
      return ALU3dCubeOrigin< HostItem::dimension, HostItem::codimension >::get( hostItem );
    }

    //! origin for given entity or intersection 
    template< class HostItem >
    static typename ALU3dCubeOrigin< HostItem::dimension, -HostItem::codimension >::OriginReturnType
    originIntersection ( const HostItem &hostItem )
    {
      return ALU3dCubeOrigin< HostItem::dimension, -HostItem::codimension >::get( hostItem );
    }

    //////////////////////////////////////////////////////////
    //  child index methods 
    //////////////////////////////////////////////////////////

    //! return child index for entity 
    template< class HostEntity >
    static int childIndex ( const HostEntity &hostEntity )
    {
      // apply the same change as for the vertices of the hexa 
      typedef ElementTopologyMapping< hexa > ElemTopo;
      return ElemTopo::alu2duneVertex( HostGrid::getRealImplementation( hostEntity ).getItem().nChild() );
    }


    //////////////////////////////////////////////////////////
    //  level methods 
    //////////////////////////////////////////////////////////

    //! return inside level for intersection 
    template <class HostIntersection>
    static int insideLevel ( const HostIntersection &hostIntersection )
    {
      assert( HostGrid::getRealIntersection( hostIntersection ).level() == hostIntersection.inside().level() );
      return HostGrid::getRealIntersection( hostIntersection ).level();
    }

    //! return outside level for intersection 
    template< class HostIntersection >
    static int outsideLevel ( const HostIntersection &hostIntersection, 
                              const int insideLevel )
    {
      // outsideLevel might be less than insideLevel if there is no level neighbor
      const int outsideLevel = HostGrid::getRealIntersection( hostIntersection ).it().outsideLevel();
      assert( !hostIntersection.neighbor() || (outsideLevel == hostIntersection.outside().level()) );
      return outsideLevel;
    }

    template< class HostIntersection >
    static int childIndexInInside ( const HostIntersection &hostIntersection,
                                    const int insideLevel,
                                    const int outsideLevel ) 
    {
      return getIntersectionChildLevel( HostGrid :: getRealIntersection( hostIntersection ).twistInInside(),
                                        HostGrid :: getRealIntersection( hostIntersection ).it().getItem().nChild(),  
                                        insideLevel,
                                        outsideLevel );
      /*
      const int calcN = getIntersectionChildLevelOld( HostGrid :: getRealIntersection( hostIntersection ).geometryInInside(),
                                                  hostIntersection.indexInInside(),
                                                  insideLevel,
                                                  outsideLevel );
                                                  */
    }

    template< class HostIntersection >
    static int childIndexInOutside ( const HostIntersection &hostIntersection,
                                     const int insideLevel,
                                     const int outsideLevel ) 
    {
      return getIntersectionChildLevel( HostGrid :: getRealIntersection( hostIntersection ).twistInOutside(),
                                        HostGrid :: getRealIntersection( hostIntersection ).it().getItem().nChild(),  
                                        outsideLevel, 
                                        insideLevel );
      /*
      const int calcN = getIntersectionChildLevelOld( HostGrid :: getRealIntersection( hostIntersection ).geometryInOutside(),
                                                  hostIntersection.indexInOutside(),
                                                  outsideLevel,
                                                  insideLevel );

      return childNr ;
      */
    }

  protected:  
    /*
    template <class LocalGeometry>
    static int getIntersectionChildLevelOld( const LocalGeometry& localGeo,
                                             const int index,
                                             const int myLevel, 
                                             const int otherLevel )
    {
      typedef typename LocalGeometry :: GlobalCoordinate GlobalCoordinate;
      // make sure non-confoming level difference is at most 1 
      assert( std::abs( myLevel - otherLevel ) <= 1 );
      if( myLevel < otherLevel )
      {
        int childIndex = 0;
        const GlobalCoordinate center = localGeo.center();
        for( int i = 0; i < LocalGeometry::mydimension; ++i )
        {
          const int j = (i < index/2 ? i : i+1 );
          childIndex |= (center[ j ] < 0.5 ? 0 : (1 << i));
        }
        return childIndex;
      }
      else
        return -1;
    }
    */

    static int getIntersectionChildLevel(
                                          const int duneTwist,
                                          const int child,
                                          const int myLevel, 
                                          const int otherLevel )
    {
      // make sure non-confoming level difference is at most 1 
      assert( std::abs( myLevel - otherLevel ) <= 1 );
      if( myLevel < otherLevel )
      {
        // swap children 2 and 3 
        static const int map[4] = { 0, 1, 3, 2 }; 
        return map[ child ];
        /*
        typedef FaceTopologyMapping<hexa>  CubeFaceMapping;
        const int twistedChild = CubeFaceMapping :: twistedDuneIndex( child, duneTwist );
        return CubeFaceMapping::dune2aluVertex( twistedChild );
        */
      }
      else
        return -1;
    }
  };
#endif // #if ENABLE_ALUGRID

} // namespace Dune 

#endif // #ifndef DUNE_CARTESIANGRID_HOSTGRIDINFO_HH
