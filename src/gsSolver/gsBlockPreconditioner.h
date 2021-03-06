/** @file gsBlockOp.h

    @brief Simple class create a block preconditioner structure.

    This file is part of the G+Smo library.

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.

    Author(s): J. Sogn
*/
#pragma once

#include <gsCore/gsExport.h>
#include <gsCore/gsLinearAlgebra.h>
#include <gsSolver/gsLinearOperator.h>

namespace gismo
{

/** \brief Simple class create a block preconditioner structure.
 *
 * Let \f$C\f$ be a preconditioner for the system of equations \f$ A\mathbf{x} =  \mathbf{f}\f$.
 * We instead wish to solve the preconditioned system \f$ CA\mathbf{x} =  C\mathbf{f}\f$.
 *
 * This class allows \f$C\f$ to be a block structure of preconditioners i.e \f$C\f$:
 * \f[
     \left( \begin{array}{cccc}
     C_{00} & C_{01} & \ldots & C_{0n}  \\
     C_{10} & C_{11} & \ldots & C_{1n}  \\
     \vdots & \vdots & \ddots & \vdots  \\
     C_{n0} & C_{n1} & \ldots & C_{nn}
     \end{array}
     \right)
     \f]
 * Where \f$C_{ij}\f$ are pre-defined preconditioners which all have the apply(input, dest) function defined.
 *
 * \ingroup Solver
 */
class GISMO_EXPORT gsBlockOp : public gsLinearOperator
{
public:

    /// Shared pointer for gsBlockOp
    typedef memory::shared_ptr< gsBlockOp > Ptr;

    /// Unique pointer for gsBlockOp
    typedef memory::unique< gsBlockOp >::ptr uPtr;
    
    /// Base class
    typedef memory::shared_ptr< gsLinearOperator > BasePtr;    
    
    gsBlockOp(index_t nRows, index_t nCols);
    
    static Ptr make(index_t nRows, index_t nCols) { return shared( new gsBlockOp(nRows,nCols) ); }

    /**
     * @brief Add a preconditioner \f$C_{ij}\f$ to the block structure
     * @param prec Pointer the preconditioner
     * @param row row position in the block preconditioner
     * @param col column position in the block preconditioner
     */
    void addOperator(index_t row, index_t col, const BasePtr& prec);

    /**
     * @brief Apply the correct segment of the input vector on the preconditioners in the block structure and store the result.
     * @param input  Input vector
     * @param result Result vector
     */
    void apply(const gsMatrix<real_t> & input, gsMatrix<real_t> & result) const;

    index_t rows() const {return blockTargetPositions.sum();}
    index_t cols() const {return blockInputPositions.sum() ;}

private:

    Eigen::Array<BasePtr, Dynamic, Dynamic> blockPrec;

    //Contains the size of the target vector for each block
    gsVector<index_t> blockTargetPositions;
    //Contains the size of the input vector for each block
    gsVector<index_t> blockInputPositions;

};

} // namespace gismo

