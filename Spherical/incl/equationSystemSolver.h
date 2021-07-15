#ifndef EQUATION_SYSTEM_SOLVER_H
#define EQUATION_SYSTEM_SOLVER_H

#include <bigNumMatrix.h>

class EquationSystemSolver
{
public:
    static BigNumMatrix calculateKaczmarz(const BigNumMatrix& A, const BigNumMatrix& b, const unsigned int& maxIterations);
    static BigNumMatrix calculateKE(const BigNumMatrix& A, const BigNumMatrix& b, const unsigned int& maxIterations);
    static BigNumMatrix calculateKERP(const BigNumMatrix& A,
                                      const BigNumMatrix& b,
                                      const mpf_class& dampFactAlpha,
                                      const mpf_class& dampFactOmega,
                                      const unsigned int& maxIterations);
    static BigNumMatrix calculateDLSQ(const BigNumMatrix& A,
                                      const BigNumMatrix& l,
                                      BigNumMatrix& v,
                                      const unsigned int& maxIterations,
                                      const mpf_class& dampening);
};

#endif /*EQUATION_SYSTEM_SOLVER_H*/