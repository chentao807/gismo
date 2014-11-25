
# pragma once

// Assumes that Eigen library has been already included

namespace gismo
{


/**
   Class that provides a container for triplets (i,j,value) to be
   filled in a sparse matrix.  Constructing a sparse matrix from
   triplets is much faster than inserting directly.  Use
   gsSparseMatrix().setFrom(gsSparseEntries) to pass the triplets to
   the matrix.
*/
template<class T>
class gsSparseEntries : public std::vector<Eigen::Triplet<T,index_t> >
{
public:
    typedef Eigen::Triplet<T,index_t> Triplet;
    typedef std::vector<Eigen::Triplet<T,index_t> > Base;

public:
    gsSparseEntries() ;

    ~gsSparseEntries() ;

    inline void add( int i, int j, T value )
    { this->push_back( Triplet(i,j,value) ); }

};



/** @brief
 *    Sparse matrix class, based on Eigen::SparseMatrix.
 *
 *   \tparam T coefficient type
 *   \tparam _Option zero is ColMajor order.
 *   \tparam _Index index type
 *
 * See http://eigen.tuxfamily.org/dox/classEigen_1_1SparseMatrix.html
 * for documentation of the Eigen::SparseMatrix class.
 *
 * Remarks:
 *
 * An entry of the gsSparseMatrix can be accessed by <b>coeff( index
 * row, index col )</b> or just with the operator <b>( index row, index col )</b>.\n
 * An entry can be changed with either
 * <b>coeffRef( index row, index col)</b> or operator <b>( index row, index col )</b>.\n
 *
 */

// Export the result to a file: saveAsBitmap(...);

template<typename T, int _Options, typename _Index>
class gsSparseMatrix : public Eigen::SparseMatrix<T,_Options,_Index>
{
public:
    typedef Eigen::SparseMatrix<T,_Options,_Index> Base;

    // Type pointing to a block of the sparse matrix
    typedef typename Eigen::Block<Base> Block;

    // Type pointing to a const block of the sparse matrix
    typedef typename Eigen::Block<const Base> constBlock;

    // Type pointing to a block view of the sparse matrix
    typedef gsMatrixBlockView<Base> BlockView;

    /// Shared pointer for gsSparseMatrix
    typedef memory::shared_ptr< gsSparseMatrix > Ptr;

public:
    gsSparseMatrix() ;

    gsSparseMatrix(_Index rows, _Index cols) ;

    /// This constructor allows constructing a gsSparseMatrix from Eigen expressions
    template<typename OtherDerived>
    gsSparseMatrix(const Eigen::EigenBase<OtherDerived>& other)  : Base(other) { }

    /// This constructor allows constructing a gsSparseMatrix from a selfadjoint view
    template<typename OtherDerived, unsigned int UpLo>
    gsSparseMatrix(const Eigen::SparseSelfAdjointView<OtherDerived, UpLo>& other)  : Base(other) { }

    /// This constructor allows constructing a gsSparseMatrix from Eigen expressions
    template<typename OtherDerived>
    gsSparseMatrix(const Eigen::MatrixBase<OtherDerived>& other)  : Base(other) { }

    /// This constructor allows constructing a gsSparseMatrix from another sparse expression
    template<typename OtherDerived> 
    gsSparseMatrix(const Eigen::SparseMatrixBase<OtherDerived>& other)  : Base(other) { } 

    /// This constructor allows constructing a gsSparseMatrix from Eigen expressions
    template<typename OtherDerived>
    gsSparseMatrix(const Eigen::ReturnByValue<OtherDerived>& other)  : Base(other) { }

    gsSparseMatrix(gsMovable< gsSparseMatrix > other)
    {
        this->swap( other.ref() );
    }

    // Using the assignment operators of Eigen
    // Note: using Base::operator=; is ambiguous in MSVC
#ifdef _MSC_VER
    template <class EigenExpr>
    gsSparseMatrix& operator= (const EigenExpr & other) 
    {
        this->Base::operator=(other);
        return *this;
    }
#else
    using Base::operator=;
#endif

    /// This method allows to swap with another vector
    gsSparseMatrix& operator=(gsMovable< gsSparseMatrix > other)
    {
        this->resize(0,0);
        this->swap( other.ref() );
        return *this;
    }


    ~gsSparseMatrix() ;

    void setFrom( gsSparseEntries<T> const & entries) ;

    inline T   at (_Index i, _Index j = 0) const { return this->coeff(i,j); }
    inline T & at (_Index i, _Index j = 0) { return this->coeffRef(i,j); }

    inline T    operator () (_Index i, _Index j = 0) const { return this->coeff(i,j); }
    inline T  & operator () (_Index i, _Index j = 0) { return this->coeffRef(i,j); }

    /// Clone function. Used to make a copy of the matrix
    gsSparseMatrix * clone() const ;

    /// Return a block view of the matrix with \a rowSizes and \a colSizes
    BlockView blockView(const gsVector<index_t> & rowSizes, 
                        const gsVector<index_t> & colSizes)
    {
        return BlockView(*this, rowSizes, colSizes);
    }

}; // class gsSparseMatrix



////////////////////////////////////////////////
////////////////////////////////////////////////




template<class T> inline
gsSparseEntries<T>::gsSparseEntries() : Base() { }

template<class T> inline
gsSparseEntries<T>::~gsSparseEntries() {  }

//template<class T>
//inline void gsSparseEntries<T>::add( int const& i, int const& j, const T & value)
//        { this->push_back( Triplet(i,j,value) ); }

//
// gsSparseMatrix //////////////////////////////////////////////////////////////
//
 
template<typename T, int _Options, typename _Index> inline
gsSparseMatrix<T, _Options, _Index>::gsSparseMatrix() : Base() { }

template<typename T, int _Options, typename _Index> inline
gsSparseMatrix<T, _Options, _Index>::gsSparseMatrix(_Index rows, _Index cols) : Base(rows,cols) { }

template<typename T, int _Options, typename _Index> inline
gsSparseMatrix<T, _Options, _Index>::~gsSparseMatrix() { }

template<typename T, int _Options, typename _Index> inline
void gsSparseMatrix<T, _Options, _Index>::setFrom( gsSparseEntries<T> const & entries) 
        { this->setFromTriplets(entries.begin(),entries.end() ); }

template<typename T, int _Options, typename _Index> inline
gsSparseMatrix<T, _Options,_Index> * gsSparseMatrix<T, _Options, _Index>::clone() const
        { return new gsSparseMatrix(*this); }



}; // namespace gismo