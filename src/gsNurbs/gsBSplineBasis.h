
#pragma once

#include <gsCore/gsForwardDeclarations.h>
#include <gsCore/gsBasis.h>
#include <gsTensor/gsTensorDomainIterator.h>
#include <gsTensor/gsTensorDomainBoundaryIterator.h>

#include <gsNurbs/gsBSplineAlgorithms.h>

namespace gismo
{

/// Traits for BSplineBasis in more dimensions
template<unsigned d, class T, class KnotVectorType>
struct gsTraits<gsBSplineBasis<T, KnotVectorType>,d>
{
    typedef gsTensorBSplineBasis<d,T,KnotVectorType>  TensorBasisType;
    typedef gsTensorBSpline<d, T, KnotVectorType>     TensorGeometryType;
    typedef typename 
    gsTraits<gsBSplineBasis<T,KnotVectorType>,d-1>::TensorBasisType
        TensorBoundaryType;

    typedef gsTensorNurbsBasis<d,T,KnotVectorType>     RationalBasisType;
    typedef gsTensorNurbs<d, T, KnotVectorType>        RationalGeometryType;
    typedef typename 
    gsTraits<gsBSplineBasis<T,KnotVectorType>,d-1>::RationalBasisType
        RationalBoundaryType;
};

/// Traits for BSplineBasis in 1 dimension: specialization for d=1
template<class T, class KnotVectorType>
struct gsTraits<gsBSplineBasis<T, KnotVectorType>,1>
{
    typedef gsBSplineBasis<T,KnotVectorType>  TensorBasisType;
    typedef gsBSpline<T, KnotVectorType>      TensorGeometryType;
    typedef gsBSpline<T, KnotVectorType>      TensorBoundaryType;

    typedef gsNurbsBasis<T,KnotVectorType>    RationalBasisType;
    typedef gsNurbsBasis<T, KnotVectorType>   RationalBoundaryType;
    typedef gsNurbs<T, KnotVectorType>        RationalGeometryType;
};


/** \brief
    A univariate B-spline basis.

    \tparam T coefficient type
    \tparam KnotVectorType the type of knot vector to use

    \ingroup basis
*/

template<class T, class KnotVectorType >
class gsBSplineBasis : public gsBasis<T>
{
public:
    typedef gsBasis<T> Base;

    /// Coefficient type
    typedef T Scalar_t;

    /// Associated geometry type
    typedef gsBSpline<T,KnotVectorType> GeometryType;

    /// Associated Boundary basis type
    typedef gsBSplineBasis<T> BoundaryBasisType;

    /// Dimension of the parameter domain
    static const int Dim = 1;

    /// Shared pointer for gsBSplineBasis
    typedef memory::shared_ptr< gsBSplineBasis > Ptr;

    static Ptr makeShared ( const KnotVectorType & KV )
    { return Ptr( new gsBSplineBasis(KV) ); }
  
public:

    /// Default empty constructor
    explicit gsBSplineBasis( bool periodic = false ) :
    Base(), m_p(0), m_periodic(0)
    {
        m_knots.initClamped(0);
        if( periodic )
        {
            gsWarn << "Converting your basis to periodic.\n";
            _convertToPeriodic();
        }

        if( ! check()  )
            gsWarn << "Warning: Insconsistent "<< *this<< "\n";
    }


    /// Construct BSpline basis of a knot vector
    gsBSplineBasis( const KnotVectorType & KV, bool periodic = false) :
    m_p(KV.degree()), m_knots(KV), m_periodic(0)
    { 
        if( periodic )
        {
            gsWarn << "Converting your basis to periodic.\n";
            _convertToPeriodic();
        }

        if( ! check()  )
            gsWarn << "Warning: Insconsistent "<< *this<< "\n";

    }

    /// Construct a BSpline basis
    /// \param u0 starting parameter
    /// \param u1 end parameter parameter
    /// \param interior number of interior knots
    /// \param degree degree of the spline space
    /// \param mult_interior multiplicity at the interior knots
    gsBSplineBasis(T u0, T u1, unsigned interior, 
                   int degree, unsigned mult_interior=1,
                   bool periodic = false ) :
    m_p(degree), 
    m_knots(u0, u1, interior, m_p+1, mult_interior, m_p), 
    m_periodic(0)
    {
        if( periodic )
        {
            _convertToPeriodic();
            gsWarn << "Converting your basis to periodic.\n";
        }

        if( ! check()  )
            gsWarn << "Warning: Insconsistent "<< *this<< "\n";
    }

    
    /// Copy Constructor
    gsBSplineBasis( const gsBSplineBasis & o)
    : m_p(o.m_p), m_knots( o.knots() ), m_periodic(o.m_periodic)
    {

    }
    
    // gsBSplineBasis( const Base & o)
    // { 
    //     const gsBSplineBasis * a;
    //     if ( ( a = dynamic_cast<const gsBSplineBasis *>( &o )) )
    //     {
    //         m_p        = a->degree() ; 
    //         m_knots    = KnotVectorType( a->knots() );
    //         m_periodic = a->m_periodic;
    //     }
    //     else
    //         GISMO_ERROR("Cannot convert "<<o<<" to gsBSplineBasis\n");
    // }
    
public:

    void swap(gsBSplineBasis& other)
    {
        std::swap(m_p, other.m_p);
        std::swap(m_periodic, other.m_periodic);
        m_knots.swap(other.m_knots);
    }

//////////////////////////////////////////////////
// Virtual member functions required by the base class
//////////////////////////////////////////////////

    // Look at gsBasis class for a description
    int dim() const { return Dim; }

    // Look at gsBasis class for a description
    int size() const { return m_knots.size() - m_p - 1 - m_periodic; }
    
    // Look at gsBasis class for a description
    int numElements() const { return m_knots.numKnotSpans(); }

    // Look at gsBasis class for a description
    int elementIndex(const gsVector<T> & u ) const;

    // Same as gsBasis::elementIndex but argument is a value instead of a vector
    int elementIndex(T u ) const;

    // Look at gsBasis class for a description
    gsBSplineBasis & component(unsigned i) const;

    /// Returns the anchors (greville points) of the basis
    void anchors_into(gsMatrix<T> & result) const 
    { 
        m_knots.greville_into(result); 
    }

    // Look at gsBasis class for a description
    void connectivity(const gsMatrix<T> & nodes, 
                      gsMesh<T> & mesh) const;

    // Look at gsBasis class for a description
    void active_into(const gsMatrix<T> & u, gsMatrix<unsigned>& result) const;

    // Look at gsBasis class for a description
    gsMatrix<unsigned> * boundary( ) const ;

    // Look at gsBasis class for a description
    gsMatrix<unsigned> * boundary(boundary::side const & s ) const;

    // Look at gsBasis class for a description
    gsBSplineBasis * boundaryBasis(boundary::side const & s ) const 
    { 
        return new gsBSplineBasis(KnotVectorType(0,1,0,1));
    }

    // Look at gsBasis class for a description
    gsMatrix<T> support() const ;

    // Look at gsBasis class for a description
    gsMatrix<T> support( const unsigned & i ) const ;

    /// Only meaningfull for periodic basis: For basis members that have
    /// a twin, this function returns the other twin index, otherwise it
    /// returns the same index as the argument
    unsigned twin(unsigned i) const ;

    // Look at gsBasis class for a description
    virtual void eval_into(const gsMatrix<T> & u, gsMatrix<T>& result) const;

    // Look at gsBasis class for a description
    virtual void evalSingle_into(unsigned i, const gsMatrix<T> & u, gsMatrix<T>& result) const;

    // Look at gsBasis class for a description
    virtual void eval_into(const gsMatrix<T> & u, const gsMatrix<T> & coefs, gsMatrix<T>& result) const;

    // Look at gsBasis class for a description
    void deriv_into(const gsMatrix<T> & u, gsMatrix<T>& result ) const ;

    // Look at gsBasis class for a description
    void derivSingle_into(unsigned i, const gsMatrix<T> & u, gsMatrix<T>& result ) const ;

    // Look at gsBasis class for a description
    void deriv_into(const gsMatrix<T> & u, const gsMatrix<T> & coefs, gsMatrix<T>& result ) const ;

    // Look at gsBasis class for a description
    void deriv2_into(const gsMatrix<T> & u, gsMatrix<T>& result ) const ;

    // Look at gsBasis class for a description
    void deriv2Single_into(unsigned i, const gsMatrix<T> & u, gsMatrix<T>& result ) const ;

    // Look at gsBasis class for a description
    void deriv2_into(const gsMatrix<T> & u, const gsMatrix<T> & coefs, gsMatrix<T>& result ) const ;

    // Look at gsBasis class for a description
    gsMatrix<T> * laplacian(const gsMatrix<T> & u ) const ;

    // Look at gsBasis class for a description
    gsBSplineBasis * clone() const;

    // Look at gsBasis class for a description
    gsBasis<T> * tensorize(const gsBasis<T> & other) const;
    
    // Look at gsBasis class for a description
    virtual gsGeometry<T> * makeGeometry( const gsMatrix<T> & coefs ) const;

    // Look at gsBasis class for a description
    virtual gsGeometry<T> * makeGeometry( gsMovable< gsMatrix<T> > coefs ) const;

    /// Check the BSplineBasis for consistency
    bool check() const
    { 
        if ( m_periodic > 0 )
        {
            // Periodicity check wrt knot values
            return ( 
                m_knots.degree()      == m_p &&
                (int)m_knots.size()   >  2*m_p+1
                );           
        }
        else
        {
            return ( 
                m_knots.degree()      == m_p &&
                (int)m_knots.size()   >  2*m_p+1
                ); 
        }
    }
  
    /// Prints the object as a string.
    std::ostream &print(std::ostream &os) const;

    /// Return a string with detailed information on the basis.
    std::string detail() const;

    // Look at gsBasis class for a description
    virtual void evalDerSingle_into(unsigned i, const gsMatrix<T> & u, 
                                    int n, gsMatrix<T>& result) const;

    // Look at gsBasis class for a description
    virtual void evalAllDers_into(const gsMatrix<T> & u, int n, gsMatrix<T>& result) const;

    // Look at gsBasis class for a description
    virtual void evalAllDersSingle_into(unsigned i, const gsMatrix<T> & u, 
                                        int n, gsMatrix<T>& result) const;

    // Look at gsBasis class for a description
    int degree(int i = 0) const 
    { 
        GISMO_ASSERT(i==0,"Asked for degree(i) in 1D basis.");
        return m_p; 
    }

    // Look at gsBasis class for a description
    int maxDegree()   const { return m_p; }

    // Look at gsBasis class for a description
    int minDegree()   const { return m_p; }

    // Look at gsBasis class for a description
    int totalDegree() const { return m_p; }
 
    /// Returns the order of the B-spline  basis
    inline unsigned order() const { return m_p+1; }

    /// True iff the point \a pp is in the domain of the basis
    inline bool inDomain(T const & pp) const 
    { return ( (pp >= m_knots[m_p]) &&  (pp <= *(m_knots.end()-m_p-1) ) ); }

    /// Returns the starting value of the domain of the basis
    T domainStart() const { return m_knots[m_p]; }

    /// Returns the ending value of the domain of the basis
    T domainEnd() const { return m_knots[m_knots.size()-m_p-1]; }

    /// Returns length of the ``active" part of the knot vector.
    T _activeLength() const { return domainEnd() - domainStart(); }

    /// Returns the index of the first active (ie. non-zero) basis function at point u
    /// Takes into account non-clamped knots.
    inline unsigned firstActive(T u) const { 
        return ( inDomain(u) ? m_knots.findspan(u)-m_p : 0 );
    }

    // Look at gsBasis class for a description
    inline unsigned numActive() const { return m_p + 1; }

    /// Returns the index of the first active (ie. non-zero) basis
    /// function at all columns (points) of u
    inline gsMatrix<unsigned,1> * firstActive(const gsMatrix<T,1> & u) const { 
        gsMatrix<unsigned,1> * fa = new gsMatrix<unsigned,1>(1, u.cols() );
        for ( index_t i = 0; i < u.cols(); i++ )
            fa->at(0,i) = ( inDomain(u(0,i)) ? m_knots.findspan(u(0,i))-m_p : 0 );
        return fa;
    }

    // Look at gsBasis class for a description
    gsDomain<T> * domain() const { return const_cast<KnotVectorType *>(&m_knots); }

    /// Returns the knot vector of the basis
    const KnotVectorType & knots () const { return m_knots;}
    KnotVectorType & knots ()       { return m_knots;}

    T knot(index_t const & i) const { return m_knots[i];}

    /// Inserts the knot \em knot in the underlying knot vector.
    void insertKnot(T knot, int mult=1)
    { m_knots.insert( knot, mult); }

    // Look at gsBasis class for a description
    void refineElements(std::vector<unsigned> const & elements)
    { m_knots.refineSpans(elements); }

    // Look at gsBasis class for a description
    void uniformRefine(int numKnots = 1)
    { m_knots.uniformRefine(numKnots); }

    // Look at gsBasis class for a description
    void uniformRefine_withCoefs(gsMatrix<T>& coefs, int numKnots = 1);

    // Look at gsBasis class for a description
    void uniformRefine_withTransfer(gsSparseMatrix<T,RowMajor> & transfer, int numKnots = 1);

    /// Refine the basis by inserting the given knots and perform knot refinement for the given coefficient matrix.
    void refine_withCoefs(gsMatrix<T>& coefs, const std::vector<T>& knots);

    /// Refine the basis by inserting the given knots and produce a sparse matrix which maps coarse coefficient vectors to refined ones.
    void refine_withTransfer(gsSparseMatrix<T,RowMajor> & transfer, const std::vector<T>& knots);

    /// Apply k-refinement to the basis i times
    void uniform_k_refine(int const & i = 1) 
    { 
        m_p += i; 
        m_knots.degreeElevate(i);
        m_knots.uniformRefine();
    }
  
    void degreeElevate(int const & i = 1) { m_p+=i; m_knots.degreeElevate(i); }

    void setDegree(int const & i);

    void degreeReduce (int const & i = 1) 
    { 
        GISMO_ASSERT( i<=m_p, "Cannot reduce degree to negative");
        m_p-=i; m_knots.degreeReduce(i);
        //m_periodic =
    }

    /// Reduces spline continuity at interior knots by \a i
    void reduceContinuity(int const & i = 1) 
    { 
        GISMO_ASSERT( i>=0 && ( m_knots.size()>2*(m_p+1) || i<=m_p ), 
                      "Cannot achieve continuity less than C^{-1} at interior knots.");
        // TODO check: max interior mult + i <= m_p+1
        m_knots.increaseMultiplicity(i);
    }

    /// Tells, whether the basis is periodic.
    bool isPeriodic() const
    {
        return (m_periodic > 0);
    }

    /// Returns number of functions crossing the boundary of the knot vector.
    int numCrossingFunctions () const
    {
        return m_periodic;
    }

    /// Checks, if both endknots have multiplicity m_p + 1.
    bool isClamped() const
    {
        if( m_knots[0] != m_knots[m_p])
            return false;

        else if( m_knots[m_knots.size() - m_p -1] != m_knots[m_knots.size()-1])
            return false;

        else
            return true;
    }

    /// If flag is true, tries to convert the basis to periodic (succeeds only if the knot vector is suitable).
    void setPeriodic(bool flag = true)
    {
        if ( flag )
            _convertToPeriodic();
        else
            m_periodic = 0;
    }

    /// Returns the multiplicity of the first ``significant" knot
    /// (i.e., the m_p+1st). If it is different from the multiplicity
    /// of the corresponding knot at the end, returns zero.
    int borderKnotMult() const;

    typename gsBasis<T>::domainIter makeDomainIterator() const
    {
        return typename gsBasis<T>::domainIter(new gsTensorDomainIterator<T,1>(*this));
    }

    typename gsBasis<T>::domainIter makeDomainIterator(const boundary::side & s) const
    {
        return ( s == boundary::none ? 
                 typename gsBasis<T>::domainIter(new gsTensorDomainIterator<T,1>(*this)) :
                 typename gsBasis<T>::domainIter(new gsTensorDomainBoundaryIterator<T,1>(*this, s))
                );
    }

    /// Moves the knot vectors to enforce periodicity.
    void enforceOuterKnotsPeriodic();

    // Look at gsBasis class for a description
    void reverse() { m_knots.reverse(); }


    /// Returns the size of the basis ignoring the bureaucratic way of
    /// turning the basis into periodic.
    int trueSize() const
    { return size() + m_periodic; }

private:

    /// Tries to convert the basis into periodic
    void _convertToPeriodic();

    /// Adjusts endknots so that the knot vector can be made periodic.
    void _stretchEndKnots();

public:

    /// \brief Helper function for evaluation with periodic basis.
    ///
    /// \param coefs coefficients (control points, one per row) before
    /// converting the basis into periodic.  
    ///
    ///\return copy of coefs with the first m_periodic rows copied to
    /// the last m_periodic rows.
    gsMatrix<T> perCoefs( const gsMatrix<T>& coefs ) const
    {
        gsMatrix<T> per_coefs = coefs;
        per_coefs.bottomRows( m_periodic ) = coefs.topRows( m_periodic );
        return per_coefs;
    }

    /// \brief Helper function for transforming periodic coefficients
    /// to full coefficients
    void expandCoefs(gsMatrix<T> & coefs) const
    {
        const index_t sz = coefs.rows();
        coefs.conservativeResize(sz+m_periodic, Eigen::NoChange);
        coefs.bottomRows( m_periodic ) = coefs.topRows( m_periodic );
    }

    /// \brief Helper function for transforming full coefficients to
    /// periodic coefficients
    void trimCoefs(gsMatrix<T> & coefs) const
    {
        const index_t sz = coefs.rows();
        coefs.conservativeResize(sz-m_periodic, Eigen::NoChange);
    }
 
// Data members
private:

    /// Degree
    int m_p;

    /// Knot vector
    KnotVectorType m_knots;
    
    /// Denotes whether the basis is periodic, ( 0 -- non-periodic, >0 -- number of ``crossing" functions)
    int m_periodic;

    /*/// Multiplicity of the p+1st knot from the beginning and from the end.
      int m_bordKnotMulti;*/

}; // class gsBSplineBasis


} // namespace gismo


//////////////////////////////////////////////////
//////////////////////////////////////////////////


#ifndef GISMO_HEADERS_ONLY
#include GISMO_HPP_HEADER(gsBSplineBasis.hpp)
#endif