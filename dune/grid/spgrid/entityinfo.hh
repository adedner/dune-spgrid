#ifndef DUNE_SPGRID_ENTITYINFO_HH
#define DUNE_SPGRID_ENTITYINFO_HH

namespace Dune
{

  template< class ct, int dim, int codim >
  class SPEntityInfo
  {
    typedef SPEntityInfo< ct, dim, codim > This;

  public:
    typedef SPGridLevel< ct, dim > GridLevel;

    typedef typename GridLevel::MultiIndex MultiIndex;

    typedef typename GridLevel::template Codim< codim >::GeometryCache GeometryCache;

    unsigned int direction () const
    {
      return direction_;
    }

    const MultiIndex &multiIndex () const
    {
      return multiIndex_;
    }

    GlobalVector origin () const
    {
      const GlobalVector &h = gridLevel().h();
      GlobalVector origin = gridLevel().domain().origin();
      for( int i = 0; i < dimension; ++i )
        origin[ i ] += multiIndex_[ i ] * h[ i ];
      return origin;
    }

    const ctype volume () const
    {
      return geometryCache().volume( direction_ );
    }

    const JacobianTransposed &jacobianTransposed () const
    {
      return geometryCache().jacobianTransposed( direction_ );
    }

    const Jacobian &jacobianInverseTransposed () const
    {
      return geometryCache().jacobianInverseTransposed( direction_ );
    }

    const GridLevel &gridLevel () const
    {
      return *gridLevel_;
    }

    const GeometryCache &geometryCache () const
    {
      return gridLevel().template geometryCache< codim >();
    }

  private:
    const GridLevel *gridLevel_;
    unsigned int direction_;
    MultiIndex multiIndex_;
  };

}

#endif
