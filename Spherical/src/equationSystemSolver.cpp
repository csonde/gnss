#include "equationSystemSolver.h"
#include "bigNumMatrix.h"
#include <cstdlib>
#include <time.h>

BigNumMatrix EquationSystemSolver::calculateKaczmarz(const BigNumMatrix& A, const BigNumMatrix& b, const unsigned int& maxIterations)
{
    assert(A.getRowNum() == b.getRowNum());
    BigNumMatrix rowNorms(A.getRowNum(), 1, A.getPrec());
    for (unsigned int i = 0; i < A.getRowNum(); i++)
    {
        for (unsigned int j = 0; j < A.getColNum(); j++)
        {
            rowNorms(i, 0) += A.getElement(i,j) * A.getElement(i,j);
        }
    }

    mpf_class rowNormSum(0, A.getPrec());
    BigNumMatrix rowNormPartialSums(A.getRowNum(), 1, A.getPrec());
    for (unsigned int i = 0; i < rowNorms.getRowNum(); i++)
    {
        rowNormSum += rowNorms(i, 0);
        for (unsigned int j = i; j < rowNormPartialSums.getRowNum(); j++)
        {
            rowNormPartialSums(j, 0) += rowNorms(i, 0);
        }
    }

    BigNumMatrix solution(A.getColNum(), 1, A.getPrec());
    srand (time(NULL));
    for (unsigned int i = 0; i < maxIterations; i++)
    {
        int randomNumber = rand();
        unsigned int currentRowIndex = 0;
        for (unsigned int j = 0; j < rowNorms.getRowNum(); j++)
        {
            currentRowIndex = j;
            if (randomNumber < rowNormPartialSums(j, 0) / rowNormSum * RAND_MAX)
            {
                break;
            }
        }
        BigNumMatrix currentRow = A.getRow(currentRowIndex);
        solution = solution + currentRow.transpose() *
                              ((b.getElement(currentRowIndex, 0) - (mpf_class)(currentRow * solution)) / rowNormSum);
    }
    return solution;
}

BigNumMatrix EquationSystemSolver::calculateKE(const BigNumMatrix& A, const BigNumMatrix& b, const unsigned int& maxIterations)
{
    BigNumMatrix y = b;
    BigNumMatrix bk = b;
    BigNumMatrix x(1, A.getColNum(), A.getPrec());
    for (unsigned int i = 0; i < maxIterations; i++)
    {
        for (unsigned int j = 0; j < A.getColNum(); j++)
        {
            BigNumMatrix alpha = A.getCol(j);
            y = y - alpha * ((mpf_class)(y.transpose() * alpha) / (mpf_class)(alpha.transpose() * alpha));
        }
        bk = bk - y;
        for (unsigned int j = 0; j < A.getRowNum(); j++)
        {
            BigNumMatrix a = A.getRow(j);
            mpf_class bj = bk(j,0);
            x = x - a * (((mpf_class)(x * a.transpose()) - bj) / (mpf_class)(a * a.transpose()));
        }
    }
    return x.transpose();
}

BigNumMatrix EquationSystemSolver::calculateKERP(const BigNumMatrix& A,
                                                 const BigNumMatrix& b,
                                                 const mpf_class& dampFactAlpha,
                                                 const mpf_class& dampFactOmega,
                                                 const unsigned int& maxIterations)
{
    BigNumMatrix y = b;
    BigNumMatrix bk = b;
    BigNumMatrix x(1, A.getColNum(), A.getPrec());
    for (unsigned int i = 0; i < maxIterations; i++)
    {
        for (unsigned int j = 0; j < A.getColNum(); j++)
        {
            BigNumMatrix alpha = A.getCol(j);
            y = y * (1 - dampFactAlpha) - alpha * ((mpf_class)(y.transpose() * alpha) / (mpf_class)(alpha.transpose() * alpha)) * dampFactAlpha;
        }
        bk = bk - y;
        for (unsigned int j = 0; j < A.getRowNum(); j++)
        {
            BigNumMatrix a = A.getRow(j);
            mpf_class bj = bk(j,0);
            x = x * (1 - dampFactOmega) - a * (((mpf_class)(x * a.transpose()) - bj) / (mpf_class)(a * a.transpose())) * dampFactOmega;
        }
    }
    return x.transpose();
}

BigNumMatrix EquationSystemSolver::calculateDLSQ(const BigNumMatrix& A,
                                                 const BigNumMatrix& L,
                                                 BigNumMatrix& v,
                                                 const unsigned int& maxIterations,
                                                 const mpf_class& dampening)
{
    BigNumMatrix identity(A.getColNum(), A.getColNum(), A.getPrec());
    for (unsigned int i = 0; i < identity.getRowNum(); i++)
    {
        identity(i,i) = 1;
    }
    BigNumMatrix normalMatrix = A.transpose() * A;
    BigNumMatrix dampedNormalMatrix = normalMatrix + identity * dampening;
    BigNumMatrix inverse = dampedNormalMatrix.inverse();
    BigNumMatrix param(A.getColNum(), 1, A.getPrec());
    for (unsigned int i = 0; i < maxIterations; i++)
    {
        BigNumMatrix a0 = A * param;
        BigNumMatrix l = L - a0;
        BigNumMatrix x = inverse * A.transpose() * l;
        v = A * x - l;
        param = param + x;
    }
    return param;
}