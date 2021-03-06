#include "CPUBipartiteGraphAnnealer.h"
#include <cmath>
#include <float.h>
#include <algorithm>
#include <exception>
#include "CPUFormulas.h"


using namespace sqaod;

template<class real>
CPUBipartiteGraphAnnealer<real>::CPUBipartiteGraphAnnealer() {
    m_ = -1;
    annState_ = annNone;
}

template<class real>
CPUBipartiteGraphAnnealer<real>::~CPUBipartiteGraphAnnealer() {
}


template<class real>
void CPUBipartiteGraphAnnealer<real>::seed(unsigned long seed) {
    random_.seed(seed);
    annState_ |= annRandSeedGiven;
}

template<class real>
void CPUBipartiteGraphAnnealer<real>::getProblemSize(SizeType *N0, SizeType *N1, SizeType *m) const {
    *N0 = N0_;
    *N1 = N1_;
    *m = m_;
}

template<class real>
void CPUBipartiteGraphAnnealer<real>::setProblem(const Vector &b0, const Vector &b1,
                                                 const Matrix &W, OptimizeMethod om) {
    N0_ = (int)b0.size;
    N1_ = (int)b1.size;
    h0_.resize(N0_);
    h1_.resize(N1_);
    J_.resize(N1_, N0_);
    Vector h0(h0_), h1(h1_);
    Matrix J(J_);
    BGFuncs<real>::calculate_hJc(&h0, &h1, &J, &c_, b0, b1, W);
    
    om_ = om;
    if (om_ == optMaximize) {
        h0_ *= real(-1.);
        h1_ *= real(-1.);
        J_ *= real(-1.);
        c_ *= real(-1.);
    }
}

template<class real>
void CPUBipartiteGraphAnnealer<real>::setNumTrotters(SizeType nTrotters) {
    THROW_IF(nTrotters <= 0, "nTrotters must be a positive integer.");
    m_ = nTrotters;
    matQ0_.resize(m_, N0_);
    matQ1_.resize(m_, N1_);
    E_.resize(m_);
    annState_ |= annNTrottersGiven;
}

template<class real>
const BitsPairArray &CPUBipartiteGraphAnnealer<real>::get_x() const {
    return bitsPairX_;
}

template<class real>
void CPUBipartiteGraphAnnealer<real>::set_x(const Bits &x0, const Bits &x1) {
    EigenRowVector ex0 = x0.mapToRowVector().cast<real>();
    EigenRowVector ex1 = x0.mapToRowVector().cast<real>();
    matQ0_.rowwise() = (ex0.array() * 2 - 1).matrix();
    matQ1_.rowwise() = (ex1.array() * 2 - 1).matrix();
    annState_ |= annQSet;
}


template<class real>
const VectorType<real> &CPUBipartiteGraphAnnealer<real>::get_E() const {
    return E_;
}


template<class real>
void CPUBipartiteGraphAnnealer<real>::get_hJc(Vector *h0, Vector *h1,
                                              Matrix *J, real *c) const {
    h0->mapToRowVector() = h0_;
    h1->mapToRowVector() = h1_;
    J->map() = J_;
    *c = c_;
}


template<class real>
const BitsPairArray &CPUBipartiteGraphAnnealer<real>::get_q() const {
    return bitsPairQ_;
}

template<class real>
void CPUBipartiteGraphAnnealer<real>::randomize_q() {
    real *q = matQ0_.data();
    for (int idx = 0; idx < IdxType(N0_ * m_); ++idx)
        q[idx] = random_.randInt(2) ? real(1.) : real(-1.);
    q = matQ1_.data();
    for (int idx = 0; idx < IdxType(N1_ * m_); ++idx)
        q[idx] = random_.randInt(2) ? real(1.) : real(-1.);
    annState_ |= annQSet;
}

template<class real>
void CPUBipartiteGraphAnnealer<real>::calculate_E() {
    BGFuncs<real>::calculate_E(&E_, h0_, h1_, J_, c_, matQ0_, matQ1_);
    if (om_ == optMaximize)
        E_.mapToRowVector() *= real(-1.);
}

template<class real>
void CPUBipartiteGraphAnnealer<real>::initAnneal() {
    if (!(annState_ & annRandSeedGiven))
        random_.seed();
    annState_ |= annRandSeedGiven;
    if (!(annState_ & annNTrottersGiven))
        setNumTrotters((N0_ + N1_) / 4);
    annState_ |= annNTrottersGiven;
    if (!(annState_ & annQSet))
        randomize_q();
    annState_ |= annQSet;
}

template<class real>
void CPUBipartiteGraphAnnealer<real>::finAnneal() {
    syncBits();
    calculate_E();
}


template<class real>
void CPUBipartiteGraphAnnealer<real>::annealOneStep(real G, real kT) {
    annealHalfStep(N1_, matQ1_, h1_, J_, matQ0_, G, kT);
    annealHalfStep(N0_, matQ0_, h0_, J_.transpose(), matQ1_, G, kT);
}

template<class real>
void CPUBipartiteGraphAnnealer<real>::
annealHalfStep(int N, EigenMatrix &qAnneal,
               const EigenRowVector &h, const EigenMatrix &J,
               const EigenMatrix &qFixed, real G, real kT) {
    EigenMatrix dEmat = J * qFixed.transpose();
    real twoDivM = real(2.) / m_;
    real tempCoef = std::log(std::tanh(G / kT / m_)) / kT;
    real invKT = real(1.) / kT;

    for (int loop = 0; loop < IdxType(N * m_); ++loop) {
        int iq = random_.randInt(N);
        int im = random_.randInt(m_);
        real q = qAnneal(im, iq);
        real dE = - twoDivM * q * (h[iq] + dEmat(iq, im));
        int mNeibour0 = (im + m_ - 1) % m_;
        int mNeibour1 = (im + 1) % m_;
        dE -= q * (qAnneal(mNeibour0, iq) + qAnneal(mNeibour1, iq)) * tempCoef;
        real thresh = dE < real(0.) ? real(1.) : std::exp(- dE * invKT);
        if (thresh > random_.random<real>())
            qAnneal(im, iq) = -q;
    }
}                    
        

template<class real>
void CPUBipartiteGraphAnnealer<real>::syncBits() {
    bitsPairX_.clear();
    bitsPairQ_.clear();
    Bits x0, x1;
    for (int idx = 0; idx < IdxType(m_); ++idx) {
        EigenBitMatrix eq0 = matQ0_.transpose().col(idx).template cast<char>();
        EigenBitMatrix eq1 = matQ1_.transpose().col(idx).template cast<char>();
        bitsPairQ_.pushBack(BitsPairArray::ValueType(Bits(eq0), Bits(eq1)));
        Bits x0 = Bits((eq0.array() + 1) / 2);
        Bits x1 = Bits((eq1.array() + 1) / 2);
        bitsPairX_.pushBack(BitsPairArray::ValueType(x0, x1));
    }
}


template class sqaod::CPUBipartiteGraphAnnealer<float>;
template class sqaod::CPUBipartiteGraphAnnealer<double>;
