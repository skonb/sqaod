#include "DeviceMath.h"
#include "Assertion.h"

using namespace sqaod_cuda;
using sqaod::Dim;
using sqaod::SizeType;

template<class real>
void DeviceMathType<real>::setToDiagonals(DeviceMatrix *A, real v) {
    assertValidMatrix(*A, __func__);
    SizeType size = std::min(A->rows, A->cols);
    devCopy_(A, v, size, A->rows + 1, 0);
}

template<class real>
void DeviceMathType<real>::scale(DeviceScalar *y, real alpha, const DeviceScalar &x,
                                 real addAssignFactor) {
    devAlloc_->allocateIfNull(y);
    devKernels_.scale(y->d_data, alpha, x.d_data, 1, addAssignFactor);
}

template<class real>
void DeviceMathType<real>::scale(DeviceVector *y, real alpha, const DeviceVector &x,
                                 real addAssignFactor) {
    devAlloc_->allocateIfNull(y, x.size);
    assertSameShape(*y, x, __func__);
    devKernels_.scale(y->d_data, alpha, x.d_data, x.size, addAssignFactor);
}

template<class real>
void DeviceMathType<real>::scale(DeviceMatrix *B, real alpha, const DeviceMatrix &A) {
    devAlloc_->allocateIfNull(B, A.dim());
    assertSameShape(*B, A, __func__);
    devKernels_.scale(B->d_data, alpha, A.d_data, A.rows * A.cols, real(0.));
}

template<class real>
void DeviceMathType<real>::scaleBroadcast(DeviceVector *y, real alpha, const DeviceScalar &x,
                                          real addAssignFactor) {
    assertValidScalar(x, __func__);
    assertValidVector(*y, __func__);
    devKernels_.scaleBroadcast(y->d_data, alpha, x.d_data, y->size, addAssignFactor);
}

template<class real>
void DeviceMathType<real>::scaleBroadcast(DeviceMatrix *A, real alpha, const DeviceVector &x,
                                          BatchOp op, real addAssignFactor) {
    assertValidMatrix(*A, __func__);
    if (op == opRowwise) {
        abortIf(A->cols != x.size, "Cols of matrix does not match vector length.");
        devKernels_.scaleBroadcastVector(A->d_data, alpha, x.d_data, x.size, A->cols,
                                         addAssignFactor);
    }
    else if (op == opColwise) {
        throwErrorIf(A->rows != x.size, "Rows of matrix does not match vector length.");
        devKernels_.scaleBroadcastScalars(A->d_data, alpha, x.d_data, x.size, A->cols,
                                          addAssignFactor);
    }
    else {
        abort_("Unknown matrix op.");
    }
}
    
template<class real>
void DeviceMathType<real>::sum(DeviceScalar *s, real alpha, const DeviceVector &x,
                               real addAssignFactor) {
    devAlloc_->allocateIfNull(s);
    devKernels_.sum(s->d_data, alpha, x.d_data, x.size, addAssignFactor);
}

template<class real>
void DeviceMathType<real>::sum(DeviceScalar *s, real alpha, const DeviceMatrix &dmat,
                               real addAssignFactor) {
    devAlloc_->allocateIfNull(s);
    devKernels_.sum(s->d_data, alpha, dmat.d_data, dmat.rows * dmat.cols, addAssignFactor);
}

template<class real>
void DeviceMathType<real>::sumDiagonals(DeviceScalar *s, const DeviceMatrix &dmat) {
    devAlloc_->allocateIfNull(s);
    int nElms = std::min(dmat.rows, dmat.cols);
    devKernels_.sumGather(s->d_data, 1., dmat.d_data, nElms, dmat.cols + 1, 0);
}

template<class real>
void DeviceMathType<real>::sumBatched(DeviceVector *vec,
                                      real alpha, const DeviceMatrix &A, BatchOp op) {
    const DeviceMatrix *dmat;
    if (op == opColwise) {
        DeviceMatrix *transposed = tempDeviceMatrix(A.dim());
        transpose(transposed, A);
        dmat = transposed;
    }
    else {
        assert(op == opRowwise);
        dmat = &A;
    }
    devAlloc_->allocateIfNull(vec, dmat->rows);
    devKernels_.sumBatched(vec->d_data, 1., dmat->d_data, dmat->cols, dmat->rows);
}

template<class real>
void DeviceMathType<real>::dot(DeviceScalar *z,
                               real alpha, const DeviceVector &x, const DeviceVector &y,
                               real addAssignFactor) {
    devAlloc_->allocateIfNull(z);
    assertSameShape(x, y, __func__);
    devKernels_.dot(z->d_data, alpha, x.d_data, y.d_data, x.size, addAssignFactor);
}

template<class real>
void DeviceMathType<real>::dotBatched(DeviceVector *z, real alpha,
                                      const DeviceMatrix &A, MatrixOp opA,
                                      const DeviceMatrix &B, MatrixOp opB) {
    const DeviceMatrix *dMat0, *dMat1;
    if (opA == opTranspose) {
        DeviceMatrix *dAt;
        dAt = tempDeviceMatrix(A.rows, A.cols);
        transpose(dAt, A);
        dMat0 = dAt;
    }
    else {
        assert(opA == opNone);
        dMat0 = &A;
    }
    if (opB == opTranspose) {
        DeviceMatrix *dBt;
        dBt = tempDeviceMatrix(B.rows, B.cols);
        transpose(dBt, A);
        dMat0 = dBt;
    }
    else {
        assert(opB == opNone);
        dMat1 = &B;
    }
    assertSameShape(*dMat0, *dMat1, __func__);
    devAlloc_->allocateIfNull(z, dMat0->rows);
    assertValidVector(*z, dMat0->rows, __func__);
    devKernels_.dotBatched(z->d_data, alpha, dMat0->d_data, dMat1->d_data, dMat0->cols, dMat0->rows);
}

template<class real>
void DeviceMathType<real>::mvProduct(DeviceVector *y, real alpha,
                                     const DeviceMatrix &A, MatrixOp opA, const DeviceVector &x) {
    SizeType size = getProductShape(A, opA, x);
    devAlloc_->allocateIfNull(y, size);
    assertValidVector(*y, size, __func__);
    
    const DeviceScalar &d_alpha = d_const(alpha);
    gemv(opA, d_alpha, A, x, d_zero(), *y);
}

template<class real>
void DeviceMathType<real>::vmProduct(DeviceVector *y, real alpha,
                                     const DeviceVector &x, const DeviceMatrix &A, MatrixOp opA,
                                     real addAssignFactor) {
    const DeviceScalar &d_alpha = d_const(alpha);
    const DeviceScalar &d_factor = d_const(addAssignFactor);
    opA = (opA == opNone) ? opTranspose : opNone;
    SizeType size = getProductShape(A, opA, x);
    devAlloc_->allocateIfNull(y, size);
    assertValidVector(*y, size, __func__);
    gemv(opA, d_alpha, A, x, d_factor, *y);
}

template<class real>
void DeviceMathType<real>::mmProduct(DeviceMatrix *C, real alpha,
                                     const DeviceMatrix &A, MatrixOp opA,
                                     const DeviceMatrix &B, MatrixOp opB) {
    Dim dim = getProductShape(A, opA, B, opB);
    abortIf(dim == Dim(), "shape mismatch on matrix-matrix multiplication.");
    devAlloc_->allocateIfNull(C, dim);
    assertValidMatrix(*C, dim, __func__);

    const DeviceScalar &d_alpha = d_const(alpha);
    gemm(opA, opB, d_alpha, A, B, d_zero(), *C);
}
    
template<class real>
void DeviceMathType<real>::vmvProduct(DeviceScalar *z, real alpha,
                                      const DeviceVector &y, const DeviceMatrix &A,
                                      const DeviceVector &x) {
    DeviceVector *Ax = tempDeviceVector(A.rows);
    gemv(opNone, d_one(), A, x, d_zero(), *Ax);

    devAlloc_->allocateIfNull(z);
    dot(z, 1., y, *Ax);
}

template<class real>
void DeviceMathType<real>::batchedVmvProduct(DeviceVector *z, real alpha,
                                             const DeviceMatrix &y,
                                             const DeviceMatrix &A,
                                             const DeviceMatrix &x) {
    Dim dim = getProductShape(x, opNone, A, opTranspose);
    abortIf(dim == Dim(), "shape mismatch on batched VxMxV product.");
        
    DeviceMatrix *Ax = tempDeviceMatrix(dim);
    dim = getProductShape(*Ax, opNone, y, opNone);
    abortIf(dim == Dim(), "shape mismatch on batched VxMxV product.");
    devAlloc_->allocateIfNull(z, A.rows);
    assertValidVector(*z, A.rows, __func__);
    
    gemm(opTranspose, opNone, d_one(), x, A, d_zero(), *Ax);
    dotBatched(z, alpha, *Ax, opNone, y, opNone);
}

template<class real>
void DeviceMathType<real>::mmmProduct(DeviceMatrix *z, real alpha,
                                      const DeviceMatrix &y, MatrixOp opy,
                                      const DeviceMatrix &A, MatrixOp opA,
                                      const DeviceMatrix &x, MatrixOp opx) {
    Dim dimAx = getProductShape(A, opA, x, opx);
    DeviceMatrix *Ax =  tempDeviceMatrix(dimAx);
    mmProduct(Ax, 1., A, opA, x, opx);
    mmProduct(z, 1., y, opy, *Ax, opNone);
}

template<class real>
void DeviceMathType<real>::min(DeviceScalar *s, const DeviceMatrix &A) {
    devAlloc_->allocateIfNull(s);
    devKernels_.min(s->d_data, A.d_data, A.rows * A.cols);
}

template<class real>
void DeviceMathType<real>::transpose(DeviceMatrix *dAt, const DeviceMatrix &A) {
    Dim dim = getMatrixShape(A, opTranspose);
    devAlloc_->allocateIfNull(dAt, dim);
    assertValidMatrix(*dAt, dim, __func__);
    devKernels_.transpose(dAt->d_data, A.d_data, A.rows, A.cols);
}


/* Matrix shape */
template<class real>
sqaod::Dim DeviceMathType<real>::getMatrixShape(const DeviceMatrix &A, MatrixOp opA) {
    Dim dim;
    dim.rows = opA == opNone ? A.rows : A.cols;
    dim.cols = opA == opNone ? A.cols : A.rows;
    return dim;
}


template<class real>
sqaod::Dim DeviceMathType<real>::getProductShape(const DeviceMatrix &A, MatrixOp opA,
                                                 const DeviceMatrix &B, MatrixOp opB) {
    Dim Adim = getMatrixShape(A, opA);
    Dim Bdim = getMatrixShape(B, opB);
    if (Adim.cols != Bdim.rows)
        return Dim();
    return Dim(B.rows, A.cols);
}

template<class real>
SizeType DeviceMathType<real>::getProductShape(const DeviceMatrix &A, MatrixOp opA,
                                               const DeviceVector &x) {
    Dim Adim = getMatrixShape(A, opA);
    throwErrorIf(Adim.cols != x.size, "Shape does not match on matrix-vector multiplication.");  
    return A.rows;
}

template<class real>
SizeType DeviceMathType<real>::getProductShape(const DeviceVector &x,
                                               const DeviceMatrix &A, MatrixOp opA) {
    Dim Adim = getMatrixShape(A, opA);
    throwErrorIf(Adim.rows != x.size, "Shape does not match on vector-matrix multiplication.");  
    return A.cols;
}

template<class real>
void DeviceMathType<real>::gemv(MatrixOp op, const DeviceScalar &d_alpha,
                                const DeviceMatrix &A, const DeviceVector &x,
                                const DeviceScalar &d_beta, DeviceVector &y) {
    /* transpose since A is in row-major format, though cublas accepts column-major format. */
    cublasOperation_t cop = (op == opNone) ? CUBLAS_OP_T : CUBLAS_OP_N;
    devKernels_.gemv(cop, A.cols, A.rows,
                     d_alpha.d_data, A.d_data, x.d_data,
                     d_beta.d_data, y.d_data);
}

template<class real>
void DeviceMathType<real>::gemm(MatrixOp opA, MatrixOp opB,
                                const DeviceScalar &d_alpha, const DeviceMatrix &A, const DeviceMatrix &B,
                                const DeviceScalar &d_beta, DeviceMatrix &C) {
    /* To get C in row-major format, actual calculation is CT = BT x AT.
     * We need to transpose to fix column-major format, and transpose again to get C in row-major format, thus, copA, copA is not transposed. */
    cublasOperation_t copA = (opA == opNone) ? CUBLAS_OP_N : CUBLAS_OP_T;
    cublasOperation_t copB = (opB == opNone) ? CUBLAS_OP_N : CUBLAS_OP_T;
    Dim dimA = getMatrixShape(A, opA);
    Dim dimProduct = getProductShape(A, opA, B, opB); // needed to transpose.
    devKernels_.gemm(copA, copB, dimProduct.cols, dimProduct.rows, dimA.cols,
                     d_alpha.d_data, B.d_data, A.d_data,
                     d_beta.d_data, C.d_data);
}


template<class real>
DeviceMathType<real>::DeviceMathType() {
    devStream_ = NULL;
}

template<class real>
DeviceMathType<real>::DeviceMathType(Device &device, DeviceStream *devStream){
    assignDevice(device, devStream);
}

#include "Device.h"

template<class real>
void DeviceMathType<real>::assignDevice(Device &device, DeviceStream *devStream) {
    if (devStream == NULL)
        devStream = device.defaultStream();
    devStream_ = devStream;

    devAlloc_ = device.objectAllocator<real>();
    devCopy_.set(device, devStream);
    devKernels_.setStream(devStream);
}


template struct sqaod_cuda::DeviceMathType<float>;
template struct sqaod_cuda::DeviceMathType<double>;

