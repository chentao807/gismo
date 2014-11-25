/** @file gsNorm.h

    @brief Provides generic routines for computing function and error norms

    This file is part of the G+Smo library.

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.

    Author(s): A. Mantzaflaris
*/

#pragma once


namespace gismo
{

/** @brief The gsNorm class provides to generic routines for
 * computing function norms and distances as well as error estimates
 * element-wise or globally
 *
*/
template <class T>
class gsNorm
{
public:
//typedef typename NormVisitor::Scalar_t T;

    /// Constructor using a multipatch domain
    gsNorm(const gsField<T> & _field1,
           const gsFunction<T> & _func2) 
    : patchesPtr( &_field1.patches() ), 
      field1(&_field1), func2(&_func2)
    { }


    void setField(const gsField<T> & _field1)
    {
        field1 = &_field1;
    }

    template <class NormVisitor>
    void apply(NormVisitor & visitor, bool storeElWise = false)
    {
        if ( storeElWise )
            m_elWise.clear();

        gsMatrix<T> quNodes  ; // Temp variable for mapped nodes
        gsVector<T> quWeights; // Temp variable for mapped weights
        gsQuadRule<T> QuRule; // Reference Quadrature rule

        // Evaluation flags for the Geometry map
        unsigned evFlags(0);

        m_value = T(0.0);
        for (unsigned pn=0; pn < patchesPtr->nPatches(); ++pn )// for all patches
        {
            const gsGeometry<T> & func1 = field1->igaFunction(pn);

            // Initialize visitor
            visitor.initialize(func1.basis(), QuRule, evFlags);

            // Initialize geometry evaluator
            typename gsGeometry<T>::Evaluator geoEval(
                patchesPtr->patch(pn).evaluator(evFlags));
            
            typename gsBasis<T>::domainIter domIt = func1.basis().makeDomainIterator();
            for (; domIt->good(); domIt->next())
            {
                // Map the Quadrature rule to the element
                QuRule.mapTo( domIt->lowerCorner(), domIt->upperCorner(), quNodes, quWeights );

                // Evaluate on quadrature points
                visitor.evaluate(*geoEval, func1, *func2, quNodes);
                
                // ** Evaluate function v
                //gsMatrix<T> f2val = func2Param ? func2.eval(quNodes)
                //: func2.eval( geoEval->values() );

                // Accumulate value from the current element (squared)
                const T result = visitor.compute(*domIt, *geoEval, quWeights);
                m_value += result;
                if ( storeElWise )
                    m_elWise.push_back( math::sqrt(result) );
            }
        }

        m_value = math::sqrt(m_value);
    }
    
public:

    /// Return the multipatch
    const gsMultiPatch<T> & patches() const { return *patchesPtr; }

    /// @brief Returns the computed norm values element-wise
    const std::vector<T> & elementNorms() const { return m_elWise; }

    /// @brief Returns the computed norm value
    T value() const { return m_value; }

protected:

    const gsMultiPatch<T> * patchesPtr;

    const gsField<T>    * field1;

    const gsFunction<T> * func2;

protected:

    std::vector<T> m_elWise;
    T              m_value;
};


} // namespace gismo
