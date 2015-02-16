#ifndef DUNE_SPGRID_CHECKSEITERATOR_HH
#define DUNE_SPGRID_CHECKSEITERATOR_HH

#include <dune/common/forloop.hh>
#include <dune/common/typetraits.hh>

#include <dune/grid/common/gridview.hh>

#include <dune/grid/extensions/superentityiterator.hh>

namespace Dune
{

  template< class VT >
  struct CheckSuperEntityIterator;



  template< class VT >
  void checkSuperEntityIterator ( const Dune::GridView< VT > &gridView )
  {
    CheckSuperEntityIterator< VT >::apply( gridView );
  }



  template< class VT >
  struct CheckSuperEntityIterator
  {
    typedef Dune::GridView< VT > GridView;
    typedef typename GridView::Grid Grid;

    template< int codim >
    struct Codim
    {
      template< bool b >
      struct Check
      {
        static void apply ( const GridView &gridView );
      };

      template< bool b >
      struct NoCheck
      {
        static void apply ( const GridView &gridView )
        {}
      };

      static void apply ( const GridView &gridView )
      {
        const bool check = Extensions::SuperEntityIterator< Grid, codim >::v;
        Dune::conditional< check, Check< true >, NoCheck< false > >::type::apply( gridView );
      }
    };

    static void apply ( const GridView &gridView )
    {
      Dune::ForLoop< Codim, 1, GridView::dimension >::apply( gridView );
    }
  };


  template< class VT >
  template< int codim >
  template< bool b >
  inline void CheckSuperEntityIterator< VT >::Codim< codim >::Check< b >
    ::apply ( const GridView &gridView )
  {
#if DUNE_GRID_EXPERIMENTAL_GRID_EXTENSIONS
    typedef typename GridView::IndexSet IndexSet;
    std::cout << ">>> Checking SuperEntityIterator for codimension "
              << codim << "..." << std::endl;

    const IndexSet &indexSet = gridView.indexSet();
    std::vector< int > count( indexSet.size( codim ), 0 );

    typedef typename GridView::template Codim< 0 >::Iterator ElementIterator;

    const ElementIterator elEnd = gridView.template end< 0 >();
    for( ElementIterator elIt = gridView.template begin< 0 >(); elIt != elEnd; ++elIt )
    {
      const typename ElementIterator::Entity &entity =*elIt;
      for( unsigned int i = 0; i < entity.subEntities( codim ); ++i )
        ++count[ indexSet.subIndex( entity, i, codim ) ];
    }

    typedef typename GridView::template Codim< codim >::Iterator CodimIterator;
    typedef typename GridView::Implementation::template Codim< codim >::SuperEntityIterator SEIterator;

    const CodimIterator codimEnd = gridView.template end< codim >();
    for( CodimIterator codimIt = gridView.template begin< codim >(); codimIt != codimEnd; ++codimIt )
    {
      const typename CodimIterator::Entity &entity = *codimIt;

      int cnt = 0;
      const SEIterator seEnd = gridView.impl().superEntityEnd( entity );
      for( SEIterator seIt = gridView.impl().superEntityBegin( entity ); seIt != seEnd; ++seIt )
      {
        const typename SEIterator::Entity &element = *seIt;

        ++cnt;

        int k = 0;
        const int numSubs = element.subEntities( codim );
        while( (k < numSubs) && (element.template subEntity< codim >( k ) != *codimIt) )
          ++k;
        if( k == numSubs )
        {
          std::cout << "Entity " << indexSet.index( entity )
                    << "of codimension " << codim
                    << "is not a subentity of its super entity iterator."
                    << std::endl;
        }
        else if( k != seIt.index() )
        {
          std::cout << "Entity " << indexSet.index( entity )
                    << "of codimension " << codim
                    << "is subEntity " << k << " instead of " << seIt.index() << "."
                    << std::endl;
        }

        SEIterator copy = seIt;
        ++copy;
        if( copy == seIt )
          std::cout << "Two subsequent super entity iterators should differ." << std::endl;
      }

      if( cnt != count[ indexSet.index( entity ) ] )
      {
        std::cout << "Number of super entities differs for entity "
                  << indexSet.index( entity )
                  << " of codimension " << codim
                  << " (" << cnt << " != " << count[ indexSet.index( entity ) ] << ")."
                  << std::endl;
      }
    }
#else // #if DUNE_GRID_EXPERIMENTAL_GRID_EXTENSIONS
    std::cerr << ">>> Skipping check for SuperEntityIterator for codimension "
              << codim << ", because experimental grid extensions are disabled."
              << std::endl;
#endif // #else // #if DUNE_GRID_EXPERIMENTAL_GRID_EXTENSIONS
  }

} // namespace Dune

#endif // #ifndef DUNE_SPGRID_CHECKSEITERATOR_HH
