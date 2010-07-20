#ifndef DUNE_SPGRID_DOMAIN_HH
#define DUNE_SPGRID_DOMAIN_HH

#include <dune/grid/spgrid/cube.hh>
#include <dune/grid/spgrid/topology.hh>
#include <dune/grid/spgrid/refinement.hh>

namespace Dune
{

  // SPDomain
  // --------

  /** \class SPDomain
   *  \brief description of the computational domain
   *
   *  \tparam  ct   coordinate type (e.g., double)
   *  \tparam  dim  dimension of the domain
   */
  template< class ct, int dim >
  class SPDomain
  {
    typedef SPDomain< ct, dim > This;

  public:
    typedef SPCube< ct, dim > Cube;
    typedef SPTopology< dim > Topology;

    /** \brief coordinate type */
    typedef typename Cube::ctype ctype;

    /** \brief dimension of the domain */
    static const int dimension = Cube::dimension;

    /** \brief type of global vectors, i.e., vectors within the domain */
    typedef typename Cube::GlobalVector GlobalVector;

    /** \brief constructor
     *
     *  \param[in]  a         one corner of the domain
     *  \param[in]  b         the opposite corner of the domain
     *  \param[in]  periodic  bit field specifying which directions should be
     *                        periodic (defaults to 0)
     *
     *  \note The only restriction on the given corners is that they are
     *        opposite to each other.
     *        It is not guaranteed, that one of the corners will be returned
     *        by the method origin.
     */
    SPDomain ( const GlobalVector &a, const GlobalVector &b,
               const unsigned int periodic = 0 );

    /** \todo please doc me */
    SPDomain ( const Topology &topology, const std::vector< Cube > &cubes );

    /** \todo please doc me */
    const Cube &cube () const { return cube_; }

    /** \todo please doc me */
    const Topology &topology () const { return topology_; }

    /** \brief determine whether the domain contains a point x
     *
     *  \param[in]  x  point to consider
     *
     *  \returns true, if x is contained in the domain
     */
    bool contains ( const GlobalVector &x ) const { return cube().contains( x ); }

    /** \brief determine whether a direction is periodic
     *
     *  \param[in]  i  direction (0 <= i < dimension)
     *
     *  \returns true, if direction i is periodic
     */
    bool periodic ( const int i ) const { return topology_.periodic( i ); }

    /** \brief obtain the periodicity bit field
     *
     *  \returns the bitfield specifying which directions are periodic
     */
    unsigned int periodic () const { return topology_.periodic(); }

    /** \brief obtain a domain modelling the unit cube
     *
     *  \returns a domain modelling \f$[0,1]^{dim}\f$
     */
    static This unitCube ();

  private:
    Cube cube_;
    Topology topology_;
  };



  // Implementation of SPDomain
  // --------------------------

  template< class ct, int dim >
  inline SPDomain< ct, dim >
    ::SPDomain ( const GlobalVector &a, const GlobalVector &b,
                 const unsigned int periodic )
  : cube_( a, b ),
    topology_( periodic )
  {}


  template< class ct, int dim >
  inline SPDomain< ct, dim >
    ::SPDomain ( const Topology &topology, const std::vector< Cube > &cubes )
  : cube_( cubes[ 0 ] ),
    topology_( topology )
  {}


  template< class ct, int dim >
  inline typename SPDomain< ct, dim >::This
  SPDomain< ct, dim >::unitCube ()
  {
    GlobalVector a, b;
    for( int i = 0; i < dimension; ++i )
    {
      a = ctype( 0 );
      b = ctype( 1 );
    }
    return This( a, b );
  }

}

#endif // #ifndef DUNE_SPGRID_DOMAIN_HH
