#ifndef DUNE_SPGRID_MULTIINDEX_HH
#define DUNE_SPGRID_MULTIINDEX_HH

#include <algorithm>
#include <array>

#include <dune/common/iostream.hh>

#include <dune/grid/spgrid/misc.hh>

namespace Dune
{

  // SPMultiIndex
  // ------------

  /**
   * \class SPMultiIndex
   * \brief multiindex
   *
   * \tparam  dim  dimension of the multiindex
   */
  template< int dim >
  class SPMultiIndex
  {
    typedef SPMultiIndex< dim > This;

  public:
    /** \brief dimension of the multiindex */
    static const int dimension = dim;

    typedef typename std::array< int, dimension >::const_iterator ConstIterator;
    typedef typename std::array< int, dimension >::iterator Iterator;

    /** \brief default constructor */
    SPMultiIndex () : index_( {} ) {}

    /**
     * \brief constructor from int array
     *
     * \note This constructor defines an implicit conversion.
     *
     * \param[in]  index  int array to copy
     */
    template <int d, std::enable_if_t<(d == dimension && d > 0), int> = 0>
    SPMultiIndex ( const int (&index)[ d ] ) { *this = index; }

    /**
     * \brief constructor from int array
     *
     * \note This constructor defines an implicit conversion.
     *
     * \param[in]  index  int array to copy
     */
    SPMultiIndex ( const std::array< int, dimension > &index ) { *this = index; }

    /** \brief copy constructor */
    // Note: The default copy constructor generated by gcc will call memmove,
    //       which is less efficient than copying each value by hand.
    //       Interestingly, calling memcpy would yield even faster code than
    //       this, but gcc does not understand its meaning and fails to optimize
    //       out unnecessary copies.
    SPMultiIndex ( const This &other ) { *this = other; }

    /** \brief copy assignment */
    // Note: The default copy assignment operator generated by gcc will call
    //       memmove, which is less efficient than copying each value by hand.
    //       Interestingly, calling memcpy would yield even faster code than
    //       this, but gcc does not understand its meaning and fails to optimize
    //       out unnecessary copies.
    This &operator= ( const This &other )
    {
      for( int i = 0; i < dimension; ++i )
        index_[ i ] = other.index_[ i ];
      return *this;
    }

    /** \brief assignment operator from int array */
    template <int d, std::enable_if_t<(d == dimension && d > 0), int> = 0>
    This &operator= ( const int (&index)[ d ] )
    {
      for( int i = 0; i < dimension; ++i )
        index_[ i ] = index[ i ];
      return *this;
    }

    This &operator= ( const std::array< int, dimension > &index )
    {
      for( int i = 0; i < dimension; ++i )
        index_[ i ] = index[ i ];
      return *this;
    }

    /** \brief add another multiindex to this one (vector operation) */
    This &operator+= ( const This &other )
    {
      for( int i = 0; i < dimension; ++i )
        index_[ i ] += other.index_[ i ];
      return *this;
    }

    /** \brief subtract another multiindex from this one (vector operation) */
    This &operator-= ( const This &other )
    {
      for( int i = 0; i < dimension; ++i )
        index_[ i ] -= other.index_[ i ];
      return *this;
    }

    /** \brief scale this multiindex (vector operation) */
    This &operator*= ( int a )
    {
      for( int i = 0; i < dimension; ++i )
        index_[ i ] *= a;
      return *this;
    }

    /** \brief scale this multiindex (vector operation) */
    This &operator/= ( int a )
    {
      for( int i = 0; i < dimension; ++i )
        index_[ i ] /= a;
      return *this;
    }

    /** \brief access i-th component */
    const int &operator[] ( int i ) const { return index_[ i ]; }

    /** \brief access i-th component */
    int &operator[] ( int i ) { return index_[ i ]; }

    /** \brief compare two multiindices for equality */
    bool operator== ( const This &other ) const
    {
      bool equals = true;
      for( int i = 0; i < dimension; ++i )
        equals &= (index_[ i ] == other.index_[ i ]);
      return equals;
    }

    /** \brief compare two multiindices for inequality */
    bool operator!= ( const This &other ) const
    {
      bool equals = false;
      for( int i = 0; i < dimension; ++i )
        equals |= (index_[ i ] != other.index_[ i ]);
      return equals;
    }

    /** \brief add multiple of a multiindex to this one (vector operation) */
    void axpy( const int a, const This &other )
    {
      for( int i = 0; i < dimension; ++i )
        index_[ i ] += a*other.index_[ i ];
    }

    ConstIterator begin () const { return index_.begin(); }
    Iterator begin () { return index_.begin(); }

    ConstIterator cbegin () const { return index_.begin(); }

    ConstIterator end () const { return index_.end(); }
    Iterator end () { return index_.end(); }

    ConstIterator cend () const { return index_.end(); }

    /** \brief initialize to zero */
    void clear ()
    {
      for( int i = 0; i < dimension; ++i )
        index_[ i ] = 0;
    }

    /** \todo please doc me */
    void increment ( const This &bound, const int k = 1 )
    {
      for( int i = 0; i < dimension; ++i )
      {
        index_[ i ] += k;
        if( index_[ i ] < bound[ i ] )
          return;
        index_[ i ] = 0;
      }
    }

    /** \todo please doc me */
    int codimension () const
    {
      int codim = dimension;
      for( int i = 0; i < dimension; ++i )
        codim -= (index_[ i ] & 1);
      return codim;
    }

    /** \brief obtain the zero multiindex */
    static This zero ()
    {
      This zero;
      zero.clear();
      return zero;
    }

  private:
    std::array< int, dimension > index_;
  };



  // Auxilliary Functions for SPMultiIndex
  // -------------------------------------

  template< class char_type, class traits, int dim >
  inline std::basic_ostream< char_type, traits > &
  operator<< ( std::basic_ostream< char_type, traits > &out, const SPMultiIndex< dim > &multiIndex )
  {
    out << "( " << multiIndex[ 0 ];
    for( int i = 1; i < dim; ++i )
      out << ", " << multiIndex[ i ];
    return out << " )";
  }


  template< class char_type, class traits, int dim >
  inline std::basic_istream< char_type, traits > &
  operator>> ( std::basic_istream< char_type, traits > &in, SPMultiIndex< dim > &multiIndex )
  {
    SPMultiIndex< dim > m;
    in >> match( '(' ) >> m[ 0 ];
    for( int i = 1; i < dim; ++i )
      in >> match( ',' ) >> m[ i ];
    in >> match( ')' );
    if( !in.fail() )
      multiIndex = m;
    return in;
  }


  template< int dim >
  inline SPMultiIndex< dim >
  operator+ ( const SPMultiIndex< dim > &a, const SPMultiIndex< dim > &b )
  {
    SPMultiIndex< dim > c = a;
    c += b;
    return c;
  }


  template< int dim >
  inline SPMultiIndex< dim >
  operator- ( const SPMultiIndex< dim > &a, const SPMultiIndex< dim > &b )
  {
    SPMultiIndex< dim > c = a;
    c -= b;
    return c;
  }


  template< int dim >
  inline SPMultiIndex< dim >
  operator* ( const SPMultiIndex< dim > &a, const int &b )
  {
    SPMultiIndex< dim > c = a;
    c *= b;
    return c;
  }


  template< int dim >
  inline SPMultiIndex< dim >
  operator* ( const int &a, const SPMultiIndex< dim > &b )
  {
    return (b*a);
  }


  template< int dim >
  inline SPMultiIndex< dim >
  operator/ ( const SPMultiIndex< dim > &a, const int &b )
  {
    SPMultiIndex< dim > c = a;
    c /= b;
    return c;
  }

} // namespace Dune


namespace std
{

  // Auxilliary functions for SPMultiIndex
  // -------------------------------------

  template< int dim >
  inline Dune::SPMultiIndex< dim >
  min ( const Dune::SPMultiIndex< dim > &a, const Dune::SPMultiIndex< dim > &b )
  {
    Dune::SPMultiIndex< dim > c;
    for( int i = 0; i < dim; ++i )
      c[ i ] = min( a[ i ], b[ i ] );
    return c;
  }


  template< int dim >
  inline Dune::SPMultiIndex< dim >
  max ( const Dune::SPMultiIndex< dim > &a, const Dune::SPMultiIndex< dim > &b )
  {
    Dune::SPMultiIndex< dim > c;
    for( int i = 0; i < dim; ++i )
      c[ i ] = max( a[ i ], b[ i ] );
    return c;
  }

} // namespace std

#endif // #ifndef DUNE_SPGRID_MULTIINDEX_HH
