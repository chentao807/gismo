/** @file gsIterativeSolver.h

    @brief Abstract class for iterative solvers.

    This file is part of the G+Smo library.

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.

    Author(s): J. Sogn
*/
#pragma once

#include <gsCore/gsExport.h>
#include <gsCore/gsLinearAlgebra.h>
#include <gsSolver/gsMatrixOperator.h>

namespace gismo
{
/// @brief Abstract class for iterative solvers.
///
/// \ingroup Solver
class gsIterativeSolver
{
public:
    typedef gsMatrix<real_t>                VectorType;

    /// Constructor for general linear operator
    gsIterativeSolver(const gsLinearOperator& _mat, index_t _maxIt=1000, real_t _tol=1e-10)
        : m_mat_ptr(), m_mat(_mat), m_maxIters(_maxIt), m_tol(_tol), m_numIter(0)
    {
        GISMO_ASSERT(m_mat.rows() == m_mat.cols(), "Matrix is not square, current implementation requires this!");
    }

    /// Constructor for general linear operator, takes ownership of the passed operator
    gsIterativeSolver(const gsLinearOperator::Ptr _mat_ptr, index_t _maxIt=1000, real_t _tol=1e-10)
        : m_mat_ptr(_mat_ptr), m_mat(*m_mat_ptr), m_maxIters(_maxIt), m_tol(_tol), m_numIter(0)
    {
        GISMO_ASSERT(m_mat.rows() == m_mat.cols(), "Matrix is not square, current implementation requires this!");
    }

    /// Constructor for sparse matrix
    template<class T, int _Options, typename _Index>
    gsIterativeSolver(const gsSparseMatrix<T, _Options, _Index > & _mat, index_t _maxIt=1000, real_t _tol=1e-10)
        : m_mat_ptr(makeMatrixOp(_mat)),
          m_mat(*m_mat_ptr), m_maxIters(_maxIt), m_tol(_tol), m_numIter(0)
    {
        GISMO_ASSERT(m_mat.rows() == m_mat.cols(), "Matrix is not square, current implementation requires this!");
    }

    /// Constructor for dense matrix
    template<class T, int _Rows, int _Cols, int _Options>
    gsIterativeSolver(const gsMatrix<T, _Rows, _Cols, _Options> & _mat, index_t _maxIt=1000, real_t _tol=1e-10)
        : m_mat_ptr(makeMatrixOp(_mat)),
          m_mat(*m_mat_ptr), m_maxIters(_maxIt), m_tol(_tol), m_numIter(0)
    {
        GISMO_ASSERT(m_mat.rows() == m_mat.cols(), "Matrix is not square, current implementation requires this!");
    }

    virtual ~gsIterativeSolver()    {}

    /// @brief Solves the linear system and stores the solution in \a x
    ///
    /// Solves the linear system of equations
    /// \param[in] rhs      the right hand side of the linear system
    /// \param[in,out] x    starting value; the solution is stored in here
    /// \param[in] precond  the preconditioner used (default: identity preconditioner)
    ///
    /// \ingroup Solver
    virtual void solve(const VectorType& rhs, VectorType& x, const gsLinearOperator & precond) = 0;

    //gsIdentityOp preConMat(N);

    virtual bool step( VectorType& x, const gsLinearOperator& precond ) = 0;

    /// Returns the size of the linear system
    index_t size() const {return m_mat.rows();}

    /// Set the maximum number of iterations (default: 1000)
    void setMaxIterations(index_t maxIt) {m_maxIters = maxIt;}

    ///Set the tolerance for the error criteria (default: 1e-10)
    void setTolerance(real_t tol) {m_tol = tol;}

    ///The number of iterations needed to reach the error criteria
    int iterations() const { return m_numIter; }

    ///The error of the iterative method
    real_t error() const { return m_error; }

    ///The tolerance used in the iterative method
    real_t tolerance() const { return m_tol; }


protected:
    const gsLinearOperator::Ptr m_mat_ptr;
    const gsLinearOperator &m_mat;

    index_t  m_maxIters;
    real_t   m_tol;
    index_t  m_numIter;
    real_t   m_error;

};

} // namespace gismo
