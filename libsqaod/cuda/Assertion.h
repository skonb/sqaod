#ifndef LIBSQAOD_CUDA_ASSERTION_H__
#define LIBSQAOD_CUDA_ASSERTION_H__

#include <common/Matrix.h>
#include <cuda/DeviceMatrix.h>
#include <common/defines.h>

using namespace sqaod;

namespace sqaod_cuda {

template<class real>
void assertSameShape(const DeviceMatrixType<real> &mat0, const MatrixType<real> &mat1,
                     const char *func) {
    abortIf(mat0.d_data == NULL, "Matrix is null");
    abortIf(mat0.dim() != mat1.dim(), "Different shape.");
}

template<class real>
void assertSameShape(const MatrixType<real> &mat0, const DeviceMatrixType<real> &mat1,
                     const char *func) {
    abortIf(mat0.data == NULL, "Matrix is null");
    abortIf(mat0.dim() != mat1.dim(), "Different shape.");
}

template<class real>
void assertSameShape(const DeviceMatrixType<real> &mat0, const DeviceMatrixType<real> &mat1,
                     const char *func) {
    abortIf(mat0.d_data == NULL, "Matrix is null");
    abortIf(mat0.dim() != mat1.dim(), "Different shape.");
}


template<class real>
void assertSameShape(const DeviceVectorType<real> &vec0, const VectorType<real> &vec1,
                     const char *func) {
    abortIf(vec0.d_data == NULL, "Vector is null");
    abortIf(vec0.size != vec1.size, "Different shape.");
}

template<class real>
void assertSameShape(const VectorType<real> &vec0, const DeviceVectorType<real> &vec1,
                     const char *func) {
    abortIf(vec0.data == NULL, "Vector is null");
    abortIf(vec0.size != vec1.size, "Different size.");
}

template<class real>
void assertSameShape(const DeviceVectorType<real> &vec0, const DeviceVectorType<real> &vec1,
                     const char *func) {
    abortIf(vec0.d_data == NULL, "Vector is null");
    abortIf(vec0.size != vec1.size, "Different size.");
}



template<class real>
void assertValidMatrix(const DeviceMatrixType<real> &mat, const char *func) {
    abortIf(mat.d_data == NULL, "Matrix is null");
}

template<class real>
void assertValidMatrix(const DeviceMatrixType<real> &mat, const sqaod::Dim &dim, const char *func) {
    abortIf(mat.d_data == NULL, "Vector is null");
    abortIf(mat.dim() != dim, "Shape mismatch.");
}

template<class real>
void assertValidVector(const DeviceVectorType<real> &vec, const char *func) {
    abortIf(vec.d_data == NULL, "Vector is null");
}

template<class real>
void assertValidVector(const DeviceVectorType<real> &vec, const SizeType size, const char *func) {
    abortIf(vec.d_data == NULL, "Vector is null");
    abortIf(vec.size != size, "Shape mismatch.");
}

template<class real>
void assertValidScalar(const DeviceScalarType<real> &s, const char *func) {
    abortIf(s.d_data == NULL, "Scalar is null");
}

}

#endif
