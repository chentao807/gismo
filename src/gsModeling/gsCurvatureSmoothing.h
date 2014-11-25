/// @file gsCurvatureSmoothing.h
/// Author: Mario Kapl
/// Computes a closed B-spline curve with a smaller number of curvature extrema compared to a given closed B-spline curve  i.e. some
/// kind of smoothing the curvature of the curve. This smoothting can be done with the help of two methods - total variation and
/// Hadenfeld's algorithm (see Jan Hadenfeld, Iteratives Glätten von B-Spline Kurven und B-Spline Flächen, Shaker Verlag, PhD Thesis)

#pragma once

#include <iostream>
#include <stdexcept>

#include <gsCore/gsGeometry.h>
#include <gsNurbs/gsBSpline.h>
#include <gsCore/gsLinearAlgebra.h>

namespace gismo
{
/**
    Class for computing a closed B-spline curve with a smaller number of curvature extrema compared to a given closed B-spline curve  i.e. some
    kind of smoothing the curvature of the curve with the help of two different methods
*/

template<class T>
class gsCurvatureSmoothing
{
public:
  /// default constructor
  gsCurvatureSmoothing()
  {
      m_curve_original=NULL;
      m_curve_smooth = NULL;
  }

  /// constructor
  gsCurvatureSmoothing(const gsBSpline<T> & init_curve, const gsMatrix<T>& param_values, const gsMatrix<T>& points)
  {
      m_curve_original= &init_curve;
      m_curve_smooth = init_curve.clone();
      m_param_values=param_values;
      m_points=points;
  }

  /// Destructor
  ~gsCurvatureSmoothing()
  {
      delete m_curve_smooth; // deleting the B-spline curve
  }


  /// gives back the original B-spline curve
  const gsBSpline<T> & curveOriginal() const { return *m_curve_original; }
  /// gives back the smoother B-spline curve
  const gsBSpline<T> & curveSmooth() const { return *m_curve_smooth; }


  /// smooth the curve by total variation -- computes the stepsize by itsself (with the help of a backtracking line search method)
  /// this method should be used instead of the two methods void smoothTotalVariationSelectLamda
  void smoothTotalVariation(const T omega1, const T omega2, const T lamda, const T tau,const unsigned iter=50);

  /// smooth the curve by total variation -- uses different stepsizes (in listlamdas) in the gradient descent method (look for the best from the list!)
  /// if possible use the method void smoothTotalVariation
  void smoothTotalVariationSelectLamda(const T omega1, const T omega2, const gsMatrix<T> listlamdas, const unsigned iter=50);

  /// smooth the curve by total variation -- uses always the same stepsize lamda - but which has to be chosen!!
  /// if possible use the method void smoothTotalVariation
  void smoothTotalVariationSelectLamda(const T omega1, const T omega2, const T lamda, const unsigned iter=50);

  /// smooth the curve by smoothing only one cofficient in each step using the Hadenfeld algorithm --- the usual Hadenfeld algorithm --this method should be used
  void smoothHadenfeld(const unsigned smooth_degree, const T delta, const int iter_step, const int iter_total, gsVector<int> &iterated, const bool original=true);


  /// smooth the curve in one step for all coefficients using the Hadenfeld algorithm. Be aware of the fact
  /// that it is not ensured that we get a nice result --- can work but do not have to work (not sure that it will converge!!)
  /// if possible use method void smoothHadenfeld
  void smoothAllHadenfeld(const unsigned smooth_degree=4, const unsigned iter=500);

  /// writes the smooth curve to a file, which can be visualized in Mathematica (Name of Mathematica File VisualizationCurvatureSmoothing)
  void write(std::ostream &os);

  /// computes approximation error of the smoother curve to the original point cloud
  void computeApproxError(T & error);

  /// computes the L^{2}-norm approximation error of the smoother curve
  void computeApproxErrorL2(T & error);

  /// computes the L-max-norm approximation error of the smoother curve
  void computeApproxErrorLMax(T & error);

  /// computes the max approximation error between the coeffeicientsof the original and the smoother curve
  void computeApproxErrorCoef(T & error);

  /// computes the curvature error of the smoother curve
  void computeCurvatureError(T & error);

private:

  /// the original B-spline curve
  const gsBSpline<T> * m_curve_original;
  /// the smoother B-spline curve
  gsBSpline<T> * m_curve_smooth;
  /// the parameter values of the original point cloud
  gsMatrix<T> m_param_values;
  /// the points of the original point cloud
  gsMatrix<T> m_points;

  /// computes all values and derivatives (up to three) at the parameter values u for the given coefs
  void compute_AllValues(gsBSplineBasis<T,gsKnotVector<T> >  * basis, gsMatrix<T> u, gsMatrix<T> *coefs, gsMatrix<T> & values0, gsMatrix<T> & values1, gsMatrix<T> & values2, gsMatrix<T> & values3);

  /// computes the objective function for given coefs and omega1 and omega2 -- objective function = omega1*ApproximationFunction + omega2*CurvatureFunction
  void compute_ObjectiveFunction(gsBSplineBasis<T,gsKnotVector<T> >  * basis, gsMatrix<T> *coefs, const T omega1, const T omega2, T &value);

  /// set the smooth curve to the the original curve
  void reset(gsBSpline<T> * newCurve)
  {
      if (m_curve_smooth)
          delete m_curve_smooth;
      m_curve_smooth = newCurve;
  }

}; // class gsCurvatureSmoothing

template<class T>
void gsCurvatureSmoothing<T>::smoothTotalVariation(const T omega1, const T omega2, const T lamda, const T tau,const unsigned iter)
{
    gsKnotVector<T> m_knots=m_curve_smooth->knots(); // take the knots of the current smooth curve
    int m_degree=m_curve_smooth->degree(); // degree of the smooth curve

    gsMatrix<T> current_coefs=m_curve_smooth->coefs(); // the coefficients of the current smooth curve

    gsMatrix<T> different_coefs;  //later needed for current selection of the coefficients for the decreasing lamdas

    int num_rows=current_coefs.rows()-m_degree; //number of rows of the coefficients
    int num_cols=current_coefs.cols();  // number of columns of the coefficients

    gsMatrix<T> deriv_coefs1; //needed later for numerical differentiation to construct the gradient (upper value)
    gsMatrix<T> deriv_coefs2; //needed later for numerical differentiation to construct the gradient (lower value)

    gsMatrix<T> m_gradient(num_rows,num_cols); // the gradient in each step be aware that we have a closed curve that means the first coefficients are equal to the last one


    gsMatrix<T> deriv_coefs3; //needed later for numerical differentiation to construct the gradient (upper value) in the lamda*gradient direction (for the backtracking line search method)
    gsMatrix<T> deriv_coefs4; //needed later for numerical differentiation to construct the gradient (lower value) in the lamda*gradient direction (for the backtracking line search method)

    gsMatrix<T> m_gradient2(num_rows,num_cols); // the gradient in the lamda*gradient direction (for line search method)

    T delta=0.0000001; // for numerical differentiation

    // the needed basis -- needed for constructing the derivatives
    gsBSplineBasis<T> * basis = new gsBSplineBasis<T,gsKnotVector<T> >(m_knots);

    //needed for computing the objective value and the derivatives
    T m_value0=0;
    T m_value1=0;
    T m_value2=0;

    // for the iteration step for the different lamdas later
    T lamda_value;
    T m_lamda=lamda;

    //the constants of the Armijio-Goldstein (Wolfe) condition
    T c1=0.0001;
    T c2=0.9;
    // the different sides of the Armijio-Goldstein (Wolfe) condition
    T cond11,cond12,cond21,cond22,helpcond;

    // the objective function for the current coefficients
    // here computed for the first iteration step - for the next steps it will be computed already in the end of the loop
    compute_ObjectiveFunction(basis,&current_coefs,omega1,omega2,m_value0);



    for(unsigned i=0;i<iter;i++){
        // gradient descent method
        //computes the gradient with the help of numerical differentiation (2 point formula)
        for(index_t j=0;j<num_rows;j++){
            for(index_t k=0;k<num_cols;k++){
                deriv_coefs1 = current_coefs;
                deriv_coefs1(j,k)+=delta;
                deriv_coefs2 = current_coefs;
                deriv_coefs2(j,k)-=delta;

                //because of closed curve -- some first and last are equal
                if(j<m_degree){
                    deriv_coefs1(j+num_rows,k)+=delta;
                    deriv_coefs2(j+num_rows,k)-=delta;
                }

                // compute the objective function for numerical differentiation by means of the 2 point formula
                compute_ObjectiveFunction(basis,&deriv_coefs1,omega1,omega2,m_value1);
                compute_ObjectiveFunction(basis,&deriv_coefs2,omega1,omega2,m_value2);

                //initialize the gradient
                m_gradient(j,k)=(m_value1-m_value2)/(2*delta);
            }
        }


        // we have to find the right lamda --> with line searching method!!
        m_lamda=lamda;
        cond11=1;
        cond12=0;
        cond21=1;
        cond22=0;

        while( (cond11 > cond12) || (cond21 > cond22) ) {
            /* first step computing the objective value in the lamda*gradient direction  */
            different_coefs=current_coefs;
            different_coefs.conservativeResize(num_rows,num_cols);
            different_coefs=different_coefs-m_lamda*m_gradient;
            different_coefs.conservativeResize(num_rows+m_degree,num_cols);
            for(index_t k=0;k<m_degree;k++){
                different_coefs.row(num_rows+k)=different_coefs.row(k);
            }
            //here computing the value for the lamda*gradient direction
            compute_ObjectiveFunction(basis,&different_coefs,omega1,omega2,lamda_value);

            /* second step computing the gradient in the lamda*gradient direction */
            //computes the gradient in the lamda*gradient direction with the help of numerical differentiation (2 point formula)
            for(index_t j=0;j<num_rows;j++){
                for(index_t k=0;k<num_cols;k++){
                    deriv_coefs3 = different_coefs;
                    deriv_coefs3(j,k)+=delta;
                    deriv_coefs4 = different_coefs;
                    deriv_coefs4(j,k)-=delta;

                    //because of closed curve -- some first and last are equal
                    if(j<m_degree){
                        deriv_coefs3(j+num_rows,k)+=delta;
                        deriv_coefs4(j+num_rows,k)-=delta;
                    }

                    // compute the objective function for numerical differentiation by means of the 2 point formula
                    compute_ObjectiveFunction(basis,&deriv_coefs3,omega1,omega2,m_value1);
                    compute_ObjectiveFunction(basis,&deriv_coefs4,omega1,omega2,m_value2);

                    //initialize the gradient
                    m_gradient2(j,k)=(m_value1-m_value2)/(2*delta);
                }
            }

            /* third step initialising the different sides of the Armijio-Goldstein (Wolfe) conditions */
            cond11=lamda_value;


            helpcond=0;
            for(index_t j=0;j<m_gradient.rows();j++){
                for(index_t k=0;k<m_gradient.cols();k++){
                    if(j<m_degree){  // do not forget we have multiple coefficients
                        helpcond+=2*m_gradient(j,k)*m_gradient(j,k);
                    }
                    else{
                        helpcond+=m_gradient(j,k)*m_gradient(j,k);
                    }
                }
            }
            cond12=m_lamda*c1*helpcond+m_value0;


            cond21=0;
            for(index_t j=0;j<m_gradient.rows();j++){
                for(index_t k=0;k<m_gradient.cols();k++){
                    if(j<m_degree){  // do not forget we have multiple coefficients
                        cond21+=2*m_gradient(j,k)*m_gradient2(j,k);
                    }
                    else{
                        cond21+=m_gradient(j,k)*m_gradient2(j,k);
                    }
                }
            }
            cond21=std::abs(cond21);

            cond22=std::abs(c2*helpcond);

            //for next testing if condition is satisfied
            m_lamda=m_lamda*tau;
        }

        m_lamda=m_lamda/tau; // to have the right m_lamda in the output

        //computing the coefficient after knowing stepsize and descent direction
        current_coefs.conservativeResize(num_rows,num_cols);
        current_coefs=current_coefs-m_lamda*m_gradient;
        current_coefs.conservativeResize(num_rows+m_degree,num_cols);
        for(index_t k=0;k<m_degree;k++){
            current_coefs.row(num_rows+k)=current_coefs.row(k);
        }

        // the objective function for the current coefficients for the next iteration step and for the output of the error
        compute_ObjectiveFunction(basis,&current_coefs,omega1,omega2,m_value0);

        gsInfo << "Step: " << i+1 << " lamda: " << m_lamda  <<" objective value: " <<m_value0 << "\n";
    }
    // construct the new smoother B-spline curve
    delete basis;
    reset( new gsBSpline<T>(m_knots, give(current_coefs)) );
}

template<class T>
void gsCurvatureSmoothing<T>::smoothTotalVariationSelectLamda(const T omega1, const T omega2, const gsMatrix<T> listlamdas, const unsigned iter)
{
    gsKnotVector<T> m_knots=m_curve_smooth->knots(); // take the knots of the current smooth curve
    int m_degree=m_curve_smooth->degree(); // degree of the smooth curve

    gsMatrix<T> current_coefs=m_curve_smooth->coefs(); // the coefficients of the current smooth curve

    gsMatrix<T> different_coefs;

    int num_rows=current_coefs.rows()-m_degree; //number of rows of the coefficients
    int num_cols=current_coefs.cols();  // number of columns of the coefficients

    gsMatrix<T> deriv_coefs1; //needed later for numerical differentiation to construct the gradient (upper value)
    gsMatrix<T> deriv_coefs2; //needed later for numerical differentiation to construct the gradient (lower value)

    gsMatrix<T> m_gradient(num_rows,num_cols); // the gradient in each step be aware that we have a closed curve that means the first coefficients are equal to the last one

    T delta=0.0000001; // for numerical differentiation

    // the needed basis -- needed for constructing the derivatives
    gsBSplineBasis<T> * basis = new gsBSplineBasis<T,gsKnotVector<T> >(m_knots);


    T m_value0=0;
    T m_value1=0;
    T m_value2=0;

    // for the iteration step for the different lamdas later
    T lamda_value;
    T max_value=1000000;
    T m_lamda=1;

    // the objective function for the current coefficients
    // here computed for the first iteration step - for the next steps it will be computed already in the end of the loop
    compute_ObjectiveFunction(basis,&current_coefs,omega1,omega2,m_value0);

    for(unsigned i=0;i<iter;i++){
        // gradient descent method
        //computes the gradient with the help of numerical differentiation (2 point formula)
        for(index_t j=0;j<num_rows;j++){
            for(index_t k=0;k<num_cols;k++){
                deriv_coefs1 = current_coefs;
                deriv_coefs1(j,k)+=delta;
                deriv_coefs2 = current_coefs;
                deriv_coefs2(j,k)-=delta;

                //because of closed curve -- some first and last are equal
                if(j<m_degree){
                    deriv_coefs1(j+num_rows,k)+=delta;
                    deriv_coefs2(j+num_rows,k)-=delta;
                }

                // compute the objective function for numerical differentiation by means of the 2 point formula
                compute_ObjectiveFunction(basis,&deriv_coefs1,omega1,omega2,m_value1);
                compute_ObjectiveFunction(basis,&deriv_coefs2,omega1,omega2,m_value2);

                //initialize the gradient
                m_gradient(j,k)=(m_value1-m_value2)/(2*delta);
            }
        }

        //iteration steps for different lamdas
        //we have first to break down to non-multiple coefficients and then the iteration step and then again generate the multiple coefficients

        max_value=1000000;

        // for different lemmas
        for(index_t j=0;j<listlamdas.cols();j++){

            different_coefs=current_coefs;

            different_coefs.conservativeResize(num_rows,num_cols);
            different_coefs=different_coefs-listlamdas(0,j)*m_gradient;
            different_coefs.conservativeResize(num_rows+m_degree,num_cols);
            for(index_t k=0;k<m_degree;k++){
                different_coefs.row(num_rows+k)=different_coefs.row(k);
            }

            //here computing the values for the different lamdas
            compute_ObjectiveFunction(basis,&different_coefs,omega1,omega2,lamda_value);

            //if objective function value smaller than change the value
            if(lamda_value<max_value){
                max_value=lamda_value;
                m_lamda=listlamdas(0,j);
            }
        }


        current_coefs.conservativeResize(num_rows,num_cols);
        current_coefs=current_coefs-m_lamda*m_gradient;
        current_coefs.conservativeResize(num_rows+m_degree,num_cols);
        for(index_t k=0;k<m_degree;k++){
            current_coefs.row(num_rows+k)=current_coefs.row(k);
        }

        // the objective function for the current coefficients for the next iteration step and for the output of the error
        compute_ObjectiveFunction(basis,&current_coefs,omega1,omega2,m_value0);

        gsInfo << "Step: " << i+1 << " lamda: " << m_lamda  <<" objective value: " <<m_value0 << "\n";

    }
    delete basis;

    // construct the new smoother B-spline curve
    reset( new gsBSpline<T>(m_knots, give(current_coefs)) );
}


template<class T>
void gsCurvatureSmoothing<T>::smoothTotalVariationSelectLamda(const T omega1, const T omega2, const T lamda, const unsigned iter)
{
    gsKnotVector<T> m_knots=m_curve_smooth->knots(); // take the knots of the current smooth curve
    int m_degree=m_curve_smooth->degree(); // degree of the smooth curve

    gsMatrix<T> current_coefs=m_curve_smooth->coefs(); // the coefficients of the current smooth curve

    int num_rows=current_coefs.rows()-m_degree; //number of rows of the coefficients
    int num_cols=current_coefs.cols();  // number of columns of the coefficients

    gsMatrix<T> deriv_coefs1; //needed later for numerical differentiation to construct the gradient (upper value)
    gsMatrix<T> deriv_coefs2; //needed later for numerical differentiation to construct the gradient (lower value)

    gsMatrix<T> m_gradient(num_rows,num_cols); // the gradient in each step be aware that we have a closed curve that means the first coefficients are equal to the last one

    T delta=0.0000001; // for numerical differentiation

    // the needed basis -- needed for constructing the derivatives
    gsBSplineBasis<T> * basis = new gsBSplineBasis<T,gsKnotVector<T> >(m_knots);


    T m_value0=0;
    T m_value1=0;
    T m_value2=0;

    // the objective function for the current coefficients
    // here computed for the first iteration step - for the next steps it will be computed already in the end of the loop
    compute_ObjectiveFunction(basis,&current_coefs,omega1,omega2,m_value0);

    for(index_t i=0;i<iter;i++){
        // gradient descent method
        //computes the gradient with the help of numerical differentiation (2 point formula)
        for(index_t j=0;j<num_rows;j++){
            for(index_t k=0;k<num_cols;k++){
                deriv_coefs1 = current_coefs;
                deriv_coefs1(j,k)+=delta;
                deriv_coefs2 = current_coefs;
                deriv_coefs2(j,k)-=delta;

                //because of closed curve -- some first and last are equal
                if(j<m_degree){
                    deriv_coefs1(j+num_rows,k)+=delta;
                    deriv_coefs2(j+num_rows,k)-=delta;
                }

                // compute the objective function for numerical differentiation by means of the 2 point formula
                compute_ObjectiveFunction(basis,&deriv_coefs1,omega1,omega2,m_value1);
                compute_ObjectiveFunction(basis,&deriv_coefs2,omega1,omega2,m_value2);

                //initialize the gradient
                m_gradient(j,k)=(m_value1-m_value2)/(2*delta);
            }
        }
        //iteration step for a given lamda
        //we have first to break down to non-multiple coefficients and then the iteration step and then again generate the multiple coefficients
        current_coefs.conservativeResize(num_rows,num_cols);
        current_coefs=current_coefs-lamda*m_gradient;
        current_coefs.conservativeResize(num_rows+m_degree,num_cols);
        for(index_t k=0;k<m_degree;k++){
            current_coefs.row(num_rows+k)=current_coefs.row(k);
        }

        // the objective function for the current coefficients for the next iteration step and for the output of the error
        compute_ObjectiveFunction(basis,&current_coefs,omega1,omega2,m_value0);

        gsInfo << "Step: " << i+1 << " lamda: " << lamda  <<" objective value: " <<m_value0 << "\n";
    }
    delete basis;

    // construct the new smoother B-spline curve
    reset( new gsBSpline<T>(m_knots, give(current_coefs)) );
}


template<class T>
void gsCurvatureSmoothing<T>::smoothHadenfeld(const unsigned smooth_degree, const T delta, const int iter_step, const int iter_total, gsVector<int> &iterated, const bool original)
{
    using std::min;
  
    int m_degree=m_curve_smooth->degree(); // degree of the curve
    gsMatrix<T> m_coefs=m_curve_smooth->coefs(); // get the coefficients they will be changed
    int num_rows=m_coefs.rows()-m_degree; // the last for coefficients are equal
    m_coefs.conservativeResize(num_rows,2); // the coefficients without the last equal points

    //iter_total could be too high chosen compared with the iter_step -- maximal allowed iter_step*num_roes <= iter_total - otherwise there could be unwanted effects for the resulting curve!!
    int m_iter_total= min(iter_total,iter_step*num_rows);

    //construct coefficients which will be not changed - from which type the should be compared original or first smoothed one
    gsMatrix<T> m_coefs_original;
    if(original==true){
        m_coefs_original=m_curve_original->coefs(); // get the original coefficients which will not be changed
        m_coefs_original.conservativeResize(num_rows,2);
    }

    else{
       m_coefs_original=m_coefs; // these coefficients will not be changed => take them from already pre-smoothed one
    }

    // describes later how often a coefficient has been already changed!! changing iterator
    gsVector<int> m_iterated(num_rows);
    m_iterated.setZero();

    T max_value,coef0,coef1,current0,current1,dist_value,m_smooth1,m_smooth2,m_smooth3,m_smooth4;
    int index=0;

    // the degree of smoothing inplies the smoother (i.e. the mask for smoothing)
    if(smooth_degree==2){
        m_smooth1=(43.0/95.0);
        m_smooth2=(16.0/95.0);
        m_smooth3=-(11.0/95.0);
        m_smooth4=-(1.0/190.0);
    }
    else if (smooth_degree==4){
        m_smooth1=(4.0/5.0);
        m_smooth2=-(2.0/5.0);
        m_smooth3=(4.0/35.0);
        m_smooth4=-(1.0/70.0);
    }
    else { // smooth degree 3 -- default smooth degree
        m_smooth1=(17.0/25.0);
        m_smooth2=-(4.0/25.0);
        m_smooth3=-(1.0/25.0);
        m_smooth4=(1.0/50.0);
    }


    // Hadenfelds algorithm (for more detail see his PhD thesis)
    for(int j=0;j<m_iter_total;j++){
        max_value=-100;
        coef0=0;
        coef1=0;
        index=0;

       for(index_t i=0;i<num_rows;i++){
            if(m_iterated(i)<iter_step){ // aks if the iter_step for one coefficient is already reached
                // computation of the new coefficients
                current0=m_smooth1*m_coefs((num_rows+i-1)%num_rows,0)+m_smooth1*m_coefs((i+1)%num_rows,0)+m_smooth2*m_coefs((num_rows+i-2)%num_rows,0)
                        +m_smooth2*m_coefs((i+2)%num_rows,0)+m_smooth3*m_coefs((num_rows+i-3)%num_rows,0)+m_smooth3*m_coefs((i+3)%num_rows,0)
                        +m_smooth4*m_coefs((num_rows+i-4)%num_rows,0)+m_smooth4*m_coefs((i+4)%num_rows,0);
                current1=m_smooth1*m_coefs((num_rows+i-1)%num_rows,1)+m_smooth1*m_coefs((i+1)%num_rows,1)+m_smooth2*m_coefs((num_rows+i-2)%num_rows,1)
                        +m_smooth2*m_coefs((i+2)%num_rows,1)+m_smooth3*m_coefs((num_rows+i-3)%num_rows,1)+m_smooth3*m_coefs((i+3)%num_rows,1)
                        +m_smooth4*m_coefs((num_rows+i-4)%num_rows,1)+m_smooth4*m_coefs((i+4)%num_rows,1);

                // the distance of the new cofficient compared to the older one from last step
                dist_value=sqrt((current0-m_coefs(i,0))*(current0-m_coefs(i,0))+(current1-m_coefs(i,1))*(current1-m_coefs(i,1)));
                // for this setting the distance is the largest until now => change values
                if(dist_value>max_value){
                    coef0=current0;
                    coef1=current1;
                    index=i;
                    max_value=dist_value;
                }

            }
        }
       // for the coefficient index the iterator will be increased by one
       m_iterated(index)++;
       // computes locally the new coefficient depending on how large the distance has been -< checks if the new values are not too far away from the original curve
       max_value=sqrt((coef0-m_coefs_original(index,0))*(coef0-m_coefs_original(index,0))+(coef1-m_coefs_original(index,1))*(coef1-m_coefs_original(index,1)));
       if(max_value>delta){
           m_coefs(index,0)= m_coefs_original(index,0)+(delta/max_value)*(coef0-m_coefs_original(index,0));
           m_coefs(index,1)= m_coefs_original(index,1)+(delta/max_value)*(coef1-m_coefs_original(index,1));

       }
       else{
           m_coefs(index,0)=coef0;
           m_coefs(index,1)=coef1;
       }
    }

   // coefficient for the closed curve again
    m_coefs.conservativeResize(num_rows+m_degree,2);
    for(index_t k=0;k<m_degree;k++){
        m_coefs.row(num_rows+k)=m_coefs.row(k);
    }

    // the knot vector of the curve
    gsKnotVector<T> m_knots;
    m_knots=m_curve_smooth->knots();

    // construct the new smoother B-spline curve
    reset( new gsBSpline<T>(m_knots, give(m_coefs)) );
    iterated=m_iterated;

}

template<class T>
void gsCurvatureSmoothing<T>::smoothAllHadenfeld(const unsigned smooth_degree,const unsigned iter)
{
    int m_degree=m_curve_smooth->degree(); // degree of the curve
    gsMatrix<T> m_coefs=m_curve_smooth->coefs(); // get the coefficients
    int num_rows=m_coefs.rows()-m_degree; // the last for coefficients are equal
    m_coefs.conservativeResize(num_rows,2); // the coefficients without the last equal points
    gsMatrix<T> m_A(num_rows,num_rows);
    m_A.setZero(); // ensure that all entries are zero in the beginning


    T m_smooth1, m_smooth2, m_smooth3, m_smooth4;
    // the degree of smoothing inplies the smoother (i.e. the matrix for smoothing)
    if(smooth_degree==2){
        m_smooth1=(43.0/95.0);
        m_smooth2=(16.0/95.0);
        m_smooth3=-(11.0/95.0);
        m_smooth4=-(1.0/190.0);
    }
    else if(smooth_degree==4){
        m_smooth1=(4.0/5.0);
        m_smooth2=-(2.0/5.0);
        m_smooth3=(4.0/35.0);
        m_smooth4=-(1.0/70.0);
    }
    else {  // smooth degree 3 -- also default smooth degree
        m_smooth1=(17.0/25.0);
        m_smooth2=-(4.0/25.0);
        m_smooth3=-(1.0/25.0);
        m_smooth4=(1.0/50.0);
    }

    // set the smoothing matrix
    for (index_t i=0;i<num_rows;i++){
        m_A(i,(num_rows+i-1)%num_rows)=m_smooth1;
        m_A(i,(i+1)%num_rows)=m_smooth1;
        m_A(i,(num_rows+i-2)%num_rows)=m_smooth2;
        m_A(i,(i+2)%num_rows)=m_smooth2;
        m_A(i,(num_rows+i-3)%num_rows)=m_smooth3;
        m_A(i,(i+3)%num_rows)=m_smooth3;
        m_A(i,(num_rows+i-4)%num_rows)=m_smooth4;
        m_A(i,(i+4)%num_rows)=m_smooth4;
    }

   // smoothing
    for(index_t k=0;k<iter;k++){
        m_coefs=m_A*m_coefs;
    }


    // coefficient for the closed curve again
    m_coefs.conservativeResize(num_rows+m_degree,2);
    for(index_t k=0;k<m_degree;k++){
        m_coefs.row(num_rows+k)=m_coefs.row(k);
    }

    // the knot vector of the curve
    gsKnotVector<T> m_knots;
    m_knots=m_curve_smooth->knots();

    // construct the new smoother B-spline curve
    reset( new gsBSpline<T>(m_knots, give(m_coefs)) );

};

template< class T>
void gsCurvatureSmoothing<T>::write(std::ostream &os){
   gsMatrix<T> m_coefs=m_curve_smooth->coefs(); // get the coefficients
   int num_rows=m_coefs.rows();
   os << "{";
   for(index_t k=0;k<num_rows-1;k++){
       os << "{" << m_coefs(k,0) << "," << m_coefs(k,1) << "},";
   }
   os << "{" << m_coefs(num_rows-1,0) << "," << m_coefs(num_rows-1,1) << "}}\n";

};

template<class T>
void gsCurvatureSmoothing<T>::computeApproxError(T & error){
    gsMatrix<T> results;
    m_curve_smooth->eval_into(m_param_values.transpose(),results); // the points of the curve for the corresponding parameter values
    results.transposeInPlace();
    error=0;
    //computing the approximation error = sum_i ||x(u_i)-p_i||^2
    for(index_t k=0;k<m_points.rows();k++){
        error+=std::pow(m_points(k,0)-results(k,0),2)+std::pow(m_points(k,1)-results(k,1),2);
    }
}

template<class T>
void gsCurvatureSmoothing<T>::computeApproxErrorL2(T & error){
    error=0;
    // uses the approximation error. Since the the parameter domain is [0,1] of the function the L^2 error = (approx.error/points)^{1/2}
    computeApproxError(error);
    error= std::sqrt(error/(m_points.rows()+0.0));
}


template<class T>
void gsCurvatureSmoothing<T>::computeApproxErrorLMax(T & error){
    gsMatrix<T> results;
    m_curve_smooth->eval_into(m_param_values.transpose(),results); // the points of the curve for the corresponding parameter values
    results.transposeInPlace();
    error=0;
    //computing the Lmax approximation error
    for(index_t k=0;k<m_points.rows();k++){
        error= std::max(error,std::sqrt(std::pow(m_points(k,0)-results(k,0),2)+std::pow(m_points(k,1)-results(k,1),2)));
    }
}


template<class T>
void gsCurvatureSmoothing<T>::computeApproxErrorCoef(T & error){
    gsMatrix<T> coefs_original=m_curve_original->coefs(); // get the coefficients of the original curve
    gsMatrix<T> coefs_smooth=m_curve_smooth->coefs(); // get the coefficients of the smoothed curve

    error=0;

    //computing the maximal coefficient approximation error
    for(index_t k=0;k<coefs_original.rows();k++){
        error= std::max(error,std::sqrt(std::pow(coefs_original(k,0)-coefs_smooth(k,0),2)+std::pow(coefs_original(k,1)-coefs_smooth(k,1),2)));
    }
}


template<class T>
void gsCurvatureSmoothing<T>::computeCurvatureError(T & error){

    gsKnotVector<T> m_knots=m_curve_smooth->knots(); // take the knots of the current smooth curve

    gsMatrix<T> current_coefs=m_curve_smooth->coefs(); // the coefficients of the current smooth curve

    // the needed basis
    gsBSplineBasis<T> * basis = new gsBSplineBasis<T,gsKnotVector<T> >(m_knots);

    error=0;

    //computation of the curvature error
    compute_ObjectiveFunction(basis,&current_coefs,0,1,error);
    delete basis;
}

template<class T>
void gsCurvatureSmoothing<T>::compute_AllValues(gsBSplineBasis<T, gsKnotVector<T> > * basis, gsMatrix<T> u, gsMatrix<T> *coefs, gsMatrix<T> & values0, gsMatrix<T> & values1, gsMatrix<T> & values2, gsMatrix<T> & values3){

    gsMatrix<T> m_results;
    gsMatrix<T> m_results1;
    gsMatrix<unsigned> actives;
    basis->evalAllDers_into(u,3,m_results);
    basis->active_into(u,actives);

    // have to resize the input matrices to ensure to have the right size
    values0.resize(coefs->cols(),u.cols());
    values1.resize(coefs->cols(),u.cols());
    values2.resize(coefs->cols(),u.cols());
    values3.resize(coefs->cols(),u.cols());
    values0.setZero();
    values1.setZero();
    values2.setZero();
    values3.setZero();

    // how many actives for one parameter value (i.e degree + 1)
    int num=actives.rows();

    //computes the values and the derivatives at the parameter values for the coefs
    for(index_t i=0;i<u.cols();i++){
        for(index_t k=0;k<num;k++){
            values0.col(i)+=coefs->row(actives(k,i))*m_results(k,i);
            values1.col(i)+=coefs->row(actives(k,i))*m_results(k+num,i);
            values2.col(i)+=coefs->row(actives(k,i))*m_results(k+2*num,i);
            values3.col(i)+=coefs->row(actives(k,i))*m_results(k+3*num,i);
        }
    }

};


template<class T>
void gsCurvatureSmoothing<T>::compute_ObjectiveFunction(gsBSplineBasis<T, gsKnotVector<T> > *basis, gsMatrix<T> *coefs, const T omega1, const T omega2, T & value){

    gsMatrix<T> m_values0;
    gsMatrix<T> m_values1;
    gsMatrix<T> m_values2;
    gsMatrix<T> m_values3;

    T objective1=0;
    T objective2=0;

    //computes all derivatives (0-th to 3rd)
    compute_AllValues(basis,m_param_values.transpose(),coefs,m_values0,m_values1,m_values2,m_values3);

    // using numerical integration for computing the values - since we have a closed curve rectangle method = trapezoidal rule
    // numerical integration by rectangle method == trapezoidal rule (because of closed curve!!!!)
    for(index_t i=0;i<m_param_values.rows();i++){
        objective1+=std::pow(m_values0(0,i)-m_points(i,0),2)+std::pow(m_values0(1,i)-m_points(i,1),2);

        objective2+= std::abs( 6.0*(m_values1(1,i)*m_values2(0,i) - m_values1(0,i)*m_values2(1,i))*(m_values1(0,i)*m_values2(0,i) + m_values1(1,i)*m_values2(1,i)) +
                          2*( (std::pow(m_values1(0,i),2)+std::pow(m_values1(1,i),2)) * ((-1.0)*m_values1(1,i)*m_values3(0,i)+ m_values1(0,i)*m_values3(1,i))  )    )/
                (2*std::pow( std::pow(m_values1(0,i),2)+std::pow(m_values1(1,i),2) ,2.5)   );
    }
    objective2=objective2/(0.0+m_param_values.rows());

    // the objective function
    value=omega1*objective1+omega2*objective2;
}



}; // namespace gismo