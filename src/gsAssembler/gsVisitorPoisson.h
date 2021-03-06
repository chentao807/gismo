/** @file gsVisitorPoisson.h

    @brief Poisson equation element visitor.

    This file is part of the G+Smo library. 

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.
    
    Author(s): A. Mantzaflaris
*/

#pragma once

#include <gsAssembler/gsGaussRule.h>

namespace gismo
{

/** \brief Visitor for the Poisson equation.
 *
 * Assembles the bilinear terms
 * \f[ (\nabla u,\nabla v)_\Omega \text{ and } (f,v)_\Omega \f]
 * For \f[ u = g \quad on \quad \partial \Omega \f],
 *
 */

template <class T, bool paramCoef = false>
class gsVisitorPoisson
{
public:

    gsVisitorPoisson(const gsPde<T> & pde)
    { 
        rhs_ptr = static_cast<const gsPoissonPde<T>*>(&pde)->rhs() ;
    }
    
    /** \brief Constructor for gsVisitorPoisson.
     *
     * \param[in] rhs Given right-hand-side function/source term that, for
     */
    /// Constructor with the right hand side function of the Poisson equation
    gsVisitorPoisson(const gsFunction<T> & rhs) : 
    rhs_ptr(&rhs)
    { }

    gsVisitorPoisson() :     rhs_ptr(NULL) { }


    void initialize(const gsBasis<T> & basis, 
                    gsQuadRule<T>    & rule, 
                    unsigned         & evFlags )
    {
        gsVector<index_t> numQuadNodes( basis.dim() );
        for (int i = 0; i < basis.dim(); ++i) // to do: improve
            numQuadNodes[i] = basis.degree(i) + 1;
        
        // Setup Quadrature
        rule = gsGaussRule<T>(numQuadNodes);// harmless slicing occurs here

        // Set Geometry evaluation flags
        evFlags = NEED_VALUE | NEED_MEASURE | NEED_GRAD_TRANSFORM;
    }

    void initialize(const gsBasis<T> & basis,
                    const index_t patchIndex,
                    const gsAssemblerOptions & options,
                    gsQuadRule<T>    & rule,
                    unsigned         & evFlags )
    {
        // Setup Quadrature
        rule = gsGaussRule<T>(basis, options.quA, options.quB);// harmless slicing occurs here

        // Set Geometry evaluation flags
        evFlags = NEED_VALUE | NEED_MEASURE | NEED_GRAD_TRANSFORM;
    }

    // Evaluate on element.
    inline void evaluate(gsBasis<T> const       & basis,
                         gsGeometryEvaluator<T> & geoEval,
                         gsMatrix<T> const      & quNodes)
    {
        // Compute the active basis functions
        // Assumes actives are the same for all quadrature points on the elements
        basis.active_into(quNodes.col(0), actives);
        numActive = actives.rows();
        
        // Evaluate basis functions on element
        basis.evalAllDers_into( quNodes, 1, basisData);
        
        // Compute image of Gauss nodes under geometry mapping as well as Jacobians
        geoEval.evaluateAt(quNodes);// is this generic ??
        
        // Evaluate right-hand side at the geometry points paramCoef
        // specifies whether the right hand side function should be
        // evaluated in parametric(true) or physical (false)
        rhs_ptr->eval_into( (paramCoef ?  quNodes :  geoEval.values() ), rhsVals );
        
        // Initialize local matrix/rhs
        localMat.setZero(numActive, numActive      );
        localRhs.setZero(numActive, rhsVals.rows() );//multiple right-hand sides
    }
    
    inline void assemble(gsDomainIterator<T>    & element, 
                         gsGeometryEvaluator<T> & geoEval,
                         gsVector<T> const      & quWeights)
    {
        gsMatrix<T> & bVals  = basisData[0];
        gsMatrix<T> & bGrads = basisData[1];

        for (index_t k = 0; k < quWeights.rows(); ++k) // loop over quadrature nodes
        {
            // Multiply weight by the geometry measure
            const T weight = quWeights[k] * geoEval.measure(k);
            
            // Compute physical gradients at k as a Dim x NumActive matrix
            geoEval.transformGradients(k, bGrads, physGrad);
            
            localRhs.noalias() += weight * ( bVals.col(k) * rhsVals.col(k).transpose() ) ;
            localMat.noalias() += weight * (physGrad.transpose() * physGrad);
        }
    }

    inline void localToGlobal(const int patchIndex,
                              const std::vector<gsMatrix<T> > & eliminatedDofs,
                              gsSparseSystem<T>     & system)
    {
        // Map patch-local DoFs to global DoFs
        system.mapColIndices(actives, patchIndex, actives);

        // Add contributions to the system matrix and right-hand side
        system.push(localMat, localRhs, actives, eliminatedDofs.front(), 0, 0);
    }
    
    inline void localToGlobal(const gsDofMapper     & mapper,
                              const gsMatrix<T>     & eliminatedDofs,
                              const int patchIndex,
                              gsSparseMatrix<T>     & sysMatrix,
                              gsMatrix<T>           & rhsMatrix )
    {
        //Assert eliminatedDofs.rows() == mapper.boundarySize()

        // Local DoFs to global DoFs
        mapper.localToGlobal(actives, patchIndex, actives);
        //const int numActive = actives.rows();
        
        for (index_t i=0; i < numActive; ++i)
        {
            const int ii = actives(i);
            if ( mapper.is_free_index(ii) )
            {
                rhsMatrix.row(ii) += localRhs.row(i);
                
                for (index_t j=0; j < numActive; ++j)
                {
                    const int jj = actives(j);
                    if ( mapper.is_free_index(jj) )
                    {
                        // Matrix is symmetric, so we could store only
                        // lower triangular part
                        //if ( jj <= ii ) 
                        sysMatrix.coeffRef(ii, jj) += localMat(i, j);
                    }
                    else // if ( mapper.is_boundary_index(jj) ) // Fixed DoF?
                    {
                        rhsMatrix.row(ii).noalias() -= localMat(i, j) * 
                            eliminatedDofs.row( mapper.global_to_bindex(jj) );
                    }
                }
            }
        }
    }

protected:
    // Right hand side
    const gsFunction<T> * rhs_ptr;

protected:
    // Basis values
    std::vector<gsMatrix<T> > basisData;
    gsMatrix<T>        physGrad;
    gsMatrix<unsigned> actives;
    index_t numActive;

protected:
    // Local values of the right hand side
    gsMatrix<T> rhsVals;

protected:
    // Local matrices
    gsMatrix<T> localMat;
    gsMatrix<T> localRhs;
};


} // namespace gismo

