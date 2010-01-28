#ifndef DUNE_SPGRID_ITERATOR_HH
#define DUNE_SPGRID_ITERATOR_HH

#include <dune/grid/spgrid/misc.hh>
#include <dune/grid/spgrid/entitypointer.hh>

namespace Dune
{

  // SPPartitionIterator
  // -------------------

  template< int codim, class Grid >
  class SPPartitionIterator
  : public SPEntityPointer< codim, Grid >
  {
    typedef SPPartitionIterator< codim, Grid > This;
    typedef SPEntityPointer< codim, Grid > Base;

  public:
    typedef typename Base::EntityInfo EntityInfo;
    typedef typename Base::GridLevel GridLevel;

    typedef typename GridLevel::PartitionList PartitionList;

    static const int dimension = Base::dimension;
    static const int codimension = Base::codimension;
    static const int mydimension = Base::mydimension;

    static const unsigned int numDirections = GridLevel::numDirections;

    struct Begin {};
    struct End {};

  protected:
    typedef typename EntityInfo::MultiIndex MultiIndex;

  public:
    using Base::gridLevel;

    SPPartitionIterator ( const GridLevel &gridLevel, const PartitionList &partitionList,
                          const Begin &b, const unsigned int sweepDir = 0 );
    SPPartitionIterator ( const GridLevel &gridLevel, const PartitionList &partitionList,
                          const End &e, const unsigned int sweepDir = 0 );

    void increment ();

  private:
    int begin ( const int i, const unsigned int dir ) const;
    int end ( const int i, const unsigned int dir ) const;

    void init ();

  protected:
    using Base::entity_;

  private:
    typename PartitionList::Iterator partition_;
    unsigned int sweepDirection_;
  };



  // Implementation of SPPartitionIterator
  // -------------------------------------

  template< int codim, class Grid >
  inline SPPartitionIterator< codim, Grid >
    ::SPPartitionIterator ( const GridLevel &gridLevel, const PartitionList &partitionList,
                            const Begin &b, const unsigned int sweepDir )
  : Base( gridLevel ),
    partition_( partitionList.begin() ),
    sweepDirection_( sweepDir )
  {
    assert( sweepDir < numDirections );
    init();
  }


  template< int codim, class Grid >
  inline SPPartitionIterator< codim, Grid >
    ::SPPartitionIterator ( const GridLevel &gridLevel, const PartitionList &partitionList,
                            const End &e, const unsigned int sweepDir )
  : Base( gridLevel ),
    partition_( partitionList.end() ),
    sweepDirection_( sweepDir )
  {
    assert( sweepDir < numDirections );
    init();
  }


  template< int codim, class Grid >
  inline void SPPartitionIterator< codim, Grid >::increment ()
  {
    EntityInfo &entityInfo = Grid::getRealImplementation( entity_ ).entityInfo();
    MultiIndex &id = entityInfo.id();
    for( int i = 0; i < dimension; ++i )
    {
      const unsigned int sweep = (sweepDirection_ >> i) & 1;
      id[ i ] += (2 - 4*sweep);
      if( id[ i ] != end( i, entityInfo.direction() ) )
        return entityInfo.update();
      id[ i ] = begin( i, entityInfo.direction() );
    }

    unsigned int dir = entityInfo.direction()+1;
    const unsigned int mydim = mydimension;
    for( ; (dir < numDirections) && (bitCount( dir ) != mydim); ++dir );
    if( dir < numDirections )
    {
      for( int i = 0; i < dimension; ++i )
        id[ i ] = begin( i, dir );
      entityInfo.update();
    }
    else
    {
      ++partition_;
      init();
    }
  }


  template< int codim, class Grid >
  inline int
  SPPartitionIterator< codim, Grid >::begin ( const int i, const unsigned int dir ) const
  {
    const unsigned int b = partition_->begin()[ i ];
    const unsigned int e = partition_->end()[ i ];
    const unsigned int s = (sweepDirection_ >> i) & 1;
    const unsigned int d = (dir >> i);
    return (1-s)*int( b + ((b^d)&1) ) + s*int( e - ((e^d)&1) );
  }


  template< int codim, class Grid >
  inline int
  SPPartitionIterator< codim, Grid >::end ( const int i, const unsigned int dir ) const
  {
    const unsigned int b = partition_->begin()[ i ];
    const unsigned int e = partition_->end()[ i ];
    const unsigned int s = (sweepDirection_ >> i) & 1;
    const unsigned int d = (dir >> i);
    return (1-s)*int( e+2 - ((b^d)&1) ) + s*int( b-2 + ((e^d)&1) );
  }


  template< int codim, class Grid >
  inline void
  SPPartitionIterator< codim, Grid >::init ()
  {
    EntityInfo &entityInfo = Grid::getRealImplementation( entity_ ).entityInfo();
    MultiIndex &id = entityInfo.id();
    if( partition_ )
    {
      unsigned int dir = 0;
      const unsigned int mydim = mydimension;
      for( ; (dir < numDirections) && (bitCount( dir ) != mydim); ++dir );
      assert( dir < numDirections );

      for( int i = 0; i < dimension; ++i )
        id[ i ] = begin( i, dir );
    }
    else
      id = std::numeric_limits< MultiIndex >::max();

    entityInfo.update();
  }



  // SPIterator
  // ----------

  template< int codim, PartitionIteratorType pitype, class Grid >
  class SPIterator
  : public SPPartitionIterator< codim, Grid >
  {
    typedef SPIterator< codim, pitype, Grid > This;
    typedef SPPartitionIterator< codim, Grid > Base;

  public:
    typedef typename Base::GridLevel GridLevel;

    template< class BeginEnd >
    SPIterator ( const GridLevel &gridLevel, const BeginEnd &be, const unsigned int sweepDir = 0 )
    : Base( gridLevel, gridLevel.allPartition(), be, sweepDir )
    {}
  };

}

#endif // #ifndef DUNE_SPGRID_ITERATOR_HH
