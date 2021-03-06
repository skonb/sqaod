/* -*- c++ -*- */
#ifndef CPU_BIPARTITEGRAPH_BF_SOLVER_H__
#define CPU_BIPARTITEGRAPH_BF_SOLVER_H__

#include <common/Common.h>
#include <cpu/Random.h>


namespace sqaod {

template<class real>
class CPUBipartiteGraphBFSolver {
    typedef EigenMatrixType<real> EigenMatrix;
    typedef EigenRowVectorType<real> EigenRowVector;
    typedef MatrixType<real> Matrix;
    typedef VectorType<real> Vector;

public:
    CPUBipartiteGraphBFSolver();
    ~CPUBipartiteGraphBFSolver();

    void seed(unsigned long seed);

    void getProblemSize(int *N0, int *N1) const;

    void setProblem(const Vector &b0, const Vector &b1, const Matrix &W,
                    OptimizeMethod om);

    void setTileSize(SizeType tileSize0, SizeType tileSize1);

    const BitsPairArray &get_x() const;

    const Vector &get_E() const;

    void initSearch();

    void finSearch();

    void searchRange(PackedBits iBegin0, PackedBits iEnd0,
                     PackedBits iBegin1, PackedBits iEnd1);

    void search();
    
private:    
    Random random_;
    SizeType N0_, N1_;
    EigenRowVector b0_, b1_;
    EigenMatrix W_;
    OptimizeMethod om_;
    PackedBits tileSize0_, tileSize1_;
    PackedBits x0max_, x1max_;
    real minE_;
    Vector E_;
    PackedBitsPairArray xPackedPairs_;
    BitsPairArray xPairs_;
};

}

#endif
