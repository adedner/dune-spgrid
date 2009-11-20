#ifndef DUNE_SPGRID_DECOMPOSITION_HH
#define DUNE_SPGRID_DECOMPOSITION_HH

#include <dune/grid/spgrid/multiindex.hh>
#include <dune/grid/spgrid/refinement.hh>

namespace Dune
{

  // SPPartition
  // -----------

  template< int dim >
  class SPPartition
  {
    typedef SPPartition< dim > This;

  public:
    static const int dimension = dim;

    typedef SPMultiIndex< dimension > MultiIndex;

    SPPartition ( const MultiIndex &origin, const MultiIndex &width )
    : origin_( origin ),
      width_( width )
    {}

    template< class ctype, SPRefinementStrategy strategy >
    SPDomain ( const This &other, const SPRefinement< ctype, dimension, strategy > &refinement )
    : origin_( other.origin_ )
    {
      for( int i = 0; i < dimension; ++i )
        width_[ i ] = refinement.factor( i ) * other.width_[ i ];
    }

    const MultiIndex &origin () const
    {
      return origin_;
    }

    const MultiIndex &width () const
    {
      return width_;
    }

  private:
    MultiIndex origin_, width_;
  };



  // SPDecomposition
  // ---------------

  template< int dim >
  class SPDecomposition
  {
    typedef SPDecomposition< dim > This;

    struct Node;

  public:
    static const int dimension = dim;

    typedef SPMultiIndex< dimension > MultiIndex;
    typedef SPPartition< dimension > Partition;

  public:
    SPDecomposition ( const MultiIndex &width, const unsigned int size )
    : node_( new Node( Partition( MultiIndex::zero(), width ) ), size )
    {}

    Partition partition ( const unsigned int rank, const PartitionIteratorType pitype ) const
    {
      switch( pitype )
      {
      case Interior_Partition:
        return root_->interiorPartition( rank );
      
      default:
        DUNE_THROW( GridError, "Partition type " << pitype << " not supported." );
      }
    }

  private:
    Node *root_;
  };


  template< int dim >
  struct SPDecomposition< dim >::Node
  {

    Node ( const Partition &partition, const unsigned int size )

    ~Node ();

    const Partition &interiorPartition ( const unsigned int rank ) const;

  private:
    Partition partition_;
    unsigned int size_;
    Node *left_, *right_;
  };


  template< int dim >
  SPDecomposition< dim >::Node::Node ( const Partition &partition, const unsigned int size )
  : partition_( partition ),
    size_( size ),
    left_( 0 ),
    right_( 0 )
  {
    if( size_ > 1 )
    {
      const MultiIndex &width = partition_.width();
      const int dir = argmax( width );

      MultiIndex leftWidth = width;
      leftWidth[ dir ] = ((size/2) * width[ dir ]) / size;
      MultiIndex rightWidth = width;
      rightWidth[ dir ] -= leftWidth[ dir ];

      MultiIndex leftOrigin = partition_.origin();
      MultiIndex rightOrigin = partition_.origin();
      rightOrigin[ dir ] += leftWidth[ dir ];

      left_ = new Node( Partition( leftOrigin, leftWidth ), size/2 );
      right_ = new Node( Partition( rightOrigin, rightWidth ), size - size/2 );
    }
  }


  template< int dim >
  SPDecomposition< dim >::Node::~Node ()
  {
    delete left_;
    delete right_;
  }


  template< int dim >
  const typename SPDecomposition< dim >::Partition &
  SPDecomposition< dim >::Node::interiorPartition ( const unsigned int rank ) const
  {
    assert( rank < size_ );
    if( size_ > 1 )
    {
      assert( (left_ != 0) && (right_ != 0) );
      if( rank_ < size_/2 )
        return left_->interiorPartition_( rank );
      else
        return right_->interiorPartition_( rank );
    }
    else
      return partition_;
  }

}

#endif // #ifndef DUNE_SPGRID_DECOMPOSITION_HH
