/** @file gsMath.h

    @brief Mathematical functions for use in G+Smo.

    This file is part of the G+Smo library. 

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.
    
    Author(s): A. Bressan, A. Mantzaflaris
*/

#pragma once

#include <gsCore/gsForwardDeclarations.h>

#ifdef _MSC_VER
#define _USE_MATH_DEFINES
#endif

#include <cmath>

#ifdef GISMO_WITH_MPQ
template <class U, class V>
inline mpq_class (max)(const __gmp_expr<mpq_t, U> & a,
                       const __gmp_expr<mpq_t, V> & b)
{return mpq_class(a < b ? b : a);}

template <class U, class V>
inline mpq_class (min)(const __gmp_expr<mpq_t, U> & a,
                       const __gmp_expr<mpq_t, V> & b)
{return mpq_class(a < b ? a : b);}

template <class U, class Z> inline
mpq_class pow(const __gmp_expr<mpq_t, U> & a, const Z & b)
{return std::pow(mpq_class(a).get_d(), b);}

/*
template <class U, class V>
inline mpq_class pow(const __gmp_expr<mpq_t, U> & a,
                     const __gmp_expr<mpq_t, V> & b)
{
    mpq_class r;
    const mpq_class aa(a);
    const mpq_class bb(b);
    mpz_pow_ui(r.get_num().__get_mp(), aa.get_num().__get_mp(), bb.get_num().get_si());
    mpz_pow_ui(r.get_den().__get_mp(), aa.get_den().__get_mp(), bb.get_num().get_si());
    if ( 1 == bb.get_den() )
        return r;
    else
        return std::pow(r.get_d(), 1/bb.get_den().get_d());
}
*/

#define GMP_EXTRA_STD_UNARY_FUNCTION(std_fun) template <class U> \
inline mpq_class std_fun(const __gmp_expr<mpq_t, U> & expr)      \
{return std::std_fun(mpq_class(expr).get_d());}
#define GMP_EXTRA_STD_BINARY_FUNCTION(std_fun) template <class U, class V>              \
inline mpq_class std_fun(const __gmp_expr<mpq_t, U> & a,const __gmp_expr<mpq_t, V> & b) \
{return std::std_fun(mpq_class(a).get_d(), mpq_class(b).get_d());}
GMP_EXTRA_STD_UNARY_FUNCTION(sqrt)
GMP_EXTRA_STD_BINARY_FUNCTION(pow)
GMP_EXTRA_STD_UNARY_FUNCTION(sin)
GMP_EXTRA_STD_UNARY_FUNCTION(asin)
GMP_EXTRA_STD_UNARY_FUNCTION(sinh)
GMP_EXTRA_STD_UNARY_FUNCTION(cos)
GMP_EXTRA_STD_UNARY_FUNCTION(cosh)
GMP_EXTRA_STD_UNARY_FUNCTION(acos)
GMP_EXTRA_STD_UNARY_FUNCTION(tan)
GMP_EXTRA_STD_UNARY_FUNCTION(tanh)
GMP_EXTRA_STD_BINARY_FUNCTION(atan2)
GMP_EXTRA_STD_UNARY_FUNCTION(atan)
GMP_EXTRA_STD_UNARY_FUNCTION(exp)
GMP_EXTRA_STD_UNARY_FUNCTION(log)
GMP_EXTRA_STD_UNARY_FUNCTION(floor)
GMP_EXTRA_STD_UNARY_FUNCTION(ceil)
#undef GMP_EXTRA_STD_UNARY_FUNCTION
#undef GMP_EXTRA_STD_BINARY_FUNCTION

#endif

namespace gismo {

/** @namespace gismo::math

    @brief
    This namespace contains common mathematical functions.
    
    \ingroup Core
*/
namespace math {

typedef std::numeric_limits<real_t> limits;

// Math functions
using std::abs;
using std::sqrt;
using std::ceil;
using std::floor;
using std::cos;
using std::cosh;
using std::sin;
using std::sinh;
using std::tan;
using std::tanh;
using std::acos;
using std::asin;
using std::log;
using std::log10;
using std::atan;
using std::atan2;
using std::exp;
using std::min;
using std::max;
using std::frexp;
using std::ldexp;
using std::pow;

// template <typename T> T min(T a, T b) {return  (a < b ? a : b); }
// template <typename T> T max(T a, T b) {return  (a < b ? b : a); }

template <typename T> inline T exp2(const T a) { return 1U << a;}

template <typename T>
T round(T a) {return math::floor(a<0.0 ? a-0.5 : a+0.5); }


/// For numeric types, this function returns the next representable
/// value after \a x in the direction of \a y
inline real_t nextafter(real_t x, real_t y)
{
#   if defined(GISMO_WITH_MPFR) || defined(GISMO_WITH_MPQ)
    return x + ( y < x ? -1e-16 : 1e-16 );
#   elif defined(_MSC_VER) && _MSC_VER < 1800
    return _nextafter(x,y);
    #else
    return ::nextafter(x,y);
    #endif
}


/** Numeric precision (number of exact decimal digits expected) for
    real_t
*/
#ifdef GISMO_WITH_MPFR
#  define REAL_DIG std::numeric_limits<real_t>::digits10()
#else
#  define REAL_DIG std::numeric_limits<real_t>::digits10
#endif

// functions to check for floating point errors
// Get isnan/isinf working on different compilers
#ifdef _MSC_VER
#include <float.h>
template <typename T>
int isnan (T a)
{return _isnan(a); }
//bool isnan (T a) {return a == a;} //equiv.
template <typename T>
int isfinite(T a)
{return _finite(a);}
//bool isfinite(T a)  {(a - a) == (a - a);} //equiv.
//template <typename T>
//bool isinf(T a) {return (_FPCLASS_PINF|_FPCLASS_NINF) & _fpclass(a);}

#ifndef NAN
// MSVC doesn't have the NAN constant in cmath, so we use the C++
// standard definition
#define NAN (std::numeric_limits<real_t>::quiet_NaN())
#endif

#else
#ifdef _INTEL_COMPILER
#include <mathimf.h>
#else
using std::isnan;
using std::isfinite;
using std::isinf;
#endif
#endif

#ifdef GISMO_WITH_MPFR
// Math functions for MPFR
using mpfr::abs;
using mpfr::pow;
using mpfr::sqrt;
using mpfr::ceil;
using mpfr::floor;
using mpfr::cos;
using mpfr::cosh;
using mpfr::asin;
using mpfr::sin;
using mpfr::sinh;
using mpfr::tan;
using mpfr::tanh;
using mpfr::acos;
using mpfr::log;
using mpfr::atan;
using mpfr::atan2;
using mpfr::isnan;
using mpfr::isfinite;
using mpfr::isinf;


//dummies
inline real_t frexp(const real_t & a, int* b) {return  a;}
inline real_t ldexp(const real_t & a, int b ) {return  a;}

#endif

#ifdef GISMO_WITH_MPQ
// Math functions for GMP/mpq_class
using ::abs;
using ::sqrt;
using ::pow;
using ::ceil;
using ::floor;
using ::cos;
using ::cosh;
using ::acos;
using ::asin;
using ::sin;
using ::sinh;
using ::tan;
using ::tanh;
using ::log;
using ::log10;
using ::atan;
using ::atan2;
using ::exp;

//fixme: min/max duplication with global
inline mpq_class (max)(const mpq_class & a, const mpq_class & b)
{return mpq_class(a < b ? b : a);}
inline mpq_class (min)(const mpq_class & a, const mpq_class & b)
{return mpq_class(a < b ? a : b);}
inline bool isfinite(mpq_class a) {return true; }
inline bool isnan(mpq_class a)    {return false;}
inline mpq_class frexp(const mpq_class & a, int* b) {return  a;}
inline mpq_class ldexp(const mpq_class & a, int b ) {return  a;}
#endif

/// Returns the sign of \a val
template <typename T> int getSign(T val)
{
    return (T(0) < val) - (val < T(0));
}

/// integer power
inline int ipow(int x, unsigned exp)
{
    int result = 1;
    while (exp)
    {
        if (exp & 1)
            result *= x;
        exp >>= 1;
        x *= x;
    }
    return result;
}

/// integer square root
inline unsigned isqrt(unsigned value) 
{
    const unsigned sr = static_cast<unsigned>(std::sqrt(static_cast<double>(value)));
    //do { ++sr; } while(sr * sr  <= value); // pick closest integer
    //do { --sr; } while(sr * sr   > value);  
    return sr; 
}

/// Returns convex combination of \a a and \a b with weight \a t
template<class T>
inline T mix(T const & a, T const & b, T const & t)
{
    return (1 - t) * a + t * b;
}

// numerical comparison
template<class T>
inline bool lessthan (T const  a, T const  b)
{ return ( b-a > math::limits::epsilon); }

// numerical equality with \a prec decimal digits
template<int prec, class T>
bool almostEqual (T const a, T const b)
{  return ( math::log10(math::abs(b-a)) < -prec); }

// numerical equality adjusted to the floating point number type
template<class T>
bool almostEqualUlp(const T a, const T b, const unsigned ulps)
{
    typedef std::numeric_limits<T> Tlimits;

    // Handle NaN.
    if (isnan(a) || isnan(b))
        return false;

    // Handle very small and exactly equal values.
    if (math::abs(a-b) <= ulps * Tlimits::denorm_min())
        return true;

    // If we get this far and either number is zero, then the other is
    // too big, so just handle that now.
    if (a == 0 || b == 0)
        return false;

    // Break the numbers into significand and exponent, sorting them
    // by exponent. (note that infinity might not be correctly handled)
    int min_exp(0), max_exp(0);
    T min_frac = frexp(a, &min_exp);
    T max_frac = frexp(b, &max_exp);
    if (min_exp > max_exp)
    {
        std::swap(min_frac, max_frac);
        std::swap(min_exp, max_exp);
    }

    // Convert the smaller to the scale of the larger by adjusting its
    // significand.
    const T scaled_min_frac = math::ldexp(min_frac, min_exp-max_exp);

    // Since the significands are now in the same scale, and the
    // larger is in the range [0.5, 1), 1 ulp is just epsilon/2.
    return math::abs(max_frac-scaled_min_frac) <= ulps * Tlimits::epsilon() / 2.0;
}

// Default value for ULPs in the above
template<class T>
bool almostEqual(const T a, const T b)
{return almostEqualUlp<T>(a,b,4); }

// static const double pi      =  3.141592653589793238462;
// static const double e       =  2.718281828459045235360;
// static const double pi_2    =  1.570796326794896619231;
// static const double pi_4    =  0.785398163397448309616;
// static const double pi_180  =  0.017453292519943295769;
// static const double _1_pi   =  0.318309886183790671538;
// static const double _2_pi   =  0.636619772367581343076;
// static const double _180_pi = 57.295779513082320876798;

} //end namespace math

/**
    \brief tests if the difference between two numbers is below
    tolerance
*/
template <typename T>
inline bool gsClose (const T &a, const T &b, const T &tol)
{
    return math::abs(a-b) <= tol;
}

/**
    \brief tests if the difference between two matrices is bounded by
    tol in \f$ L^\infty \f$ norm

    The tolerance is relative to maximum absolute values of the
    entries of the matrices.
*/
template <typename matrix_t1, typename matrix_t2>
inline bool gsAllCloseRelativeToMax (const matrix_t1 &a, const matrix_t2 &b, const typename matrix_t1::Scalar &tol )
{
    GISMO_ASSERT( a.cols()==b.cols() && a.rows()==b.rows(), "Only matrices of the same size can be compared" );
    return (a-b).array().abs().maxCoeff() <= tol*math::max(a.array().abs().maxCoeff(), b.array().abs().maxCoeff());
}

/**
    \brief tests if the difference between two matrices is bounded by
    tol in \f$ L^\infty \f$ norm

    The tolerance is absolute, therefore independent of the matrix entries.
*/
template <typename matrix_t1, typename matrix_t2>
inline bool gsAllCloseAbsolute (const matrix_t1 &a, const matrix_t2 &b, const typename matrix_t1::Scalar &tol )
{
    GISMO_ASSERT( a.cols()==b.cols() && a.rows()==b.rows(), "Only matrices of the same size can be compared" );
    return (a-b).array().abs().maxCoeff() <= tol;
}

/**
    \brief Tests whether the difference between two matrices is bounded by
    tol in \f$ L^\infty \f$ norm

    The tolerance is absolute below the reference, but relative for
    bigger numbers.
*/
template <typename matrix_t1, typename matrix_t2>
inline bool gsAllCloseRelAndAbsWithRef (const matrix_t1 &a, const matrix_t2 &b, const typename matrix_t1::Scalar &tol, const typename matrix_t1::Scalar &ref )
{
    GISMO_ASSERT( a.cols()==b.cols() && a.rows()==b.rows(), "Only matrices of the same size can be compared" );
    return (a-b).array().abs().maxCoeff() <= tol*math::max(ref,a.array().abs().maxCoeff(), b.array().abs().maxCoeff());
}


/*
  CompileTimeLog2 computes the logarithm base 2 of the argument using
  template recursion.

template <unsigned arg>
struct CompileTimeLog2
{
    enum { result = CompileTimeLog2< arg / 2 >::result + 1 };
};
template <>
struct CompileTimeLog2<1>
{
    enum { result = 0 };
};
template <>
struct CompileTimeLog2<0>
{
    // empty because the logarithm of 0 is -infinity
};
template <unsigned arg>
unsigned CTLog2 (void) {return CompileTimeLog2<arg>::result;}
*/

} //end namespace gismo
