#ifndef BIGNUMMATRIX_H
#define BIGNUMMATRIX_H

#include <gmpxx.h>
#include <eigen3/Eigen/Core>
#include <iostream>

using namespace Eigen;

class BigNumMatrix
{

private:
    unsigned int rowNum;
    unsigned int colNum;
    unsigned int prec;
    mpf_class** matrix;

    mpf_class threshold;

public:
    BigNumMatrix();
    BigNumMatrix(const BigNumMatrix& other);
    BigNumMatrix(const MatrixXd& other);
    BigNumMatrix(unsigned int rowNum, unsigned int colNum, unsigned int prec = 128);
    BigNumMatrix(unsigned int rowNum, unsigned int colNum, mpf_class filler, unsigned int prec = 128, mpf_class threshold = mpf_class("1e-30", 128));
    //BigNumMatrix(unsigned int rowNum, unsigned int colNum, mpf_class filler, unsigned int prec, mpf_class threshold)
    ~BigNumMatrix();

    void setPrec(unsigned int prec);
    unsigned int getPrec() const;
    unsigned int getColNum() const;
    unsigned int getRowNum() const;
    void setThreshold(mpf_class threshold);
    mpf_class getThreshold() const;
    mpf_class getElement(const unsigned int rowIndex, const unsigned int colIndex) const;
    void resize(unsigned int rowNum, unsigned int colNum);

    BigNumMatrix& operator=(const BigNumMatrix& other);
    BigNumMatrix operator+(const BigNumMatrix& other) const;
    BigNumMatrix operator-(const BigNumMatrix& other) const;
    BigNumMatrix operator*(const BigNumMatrix& other) const;
    BigNumMatrix operator*(const mpf_class& scalar) const;
    mpf_class& operator()(unsigned int rowIndex, unsigned int colIndex);
    const mpf_class& operator()(unsigned int rowIndex, unsigned int colIndex) const;
    operator mpf_class();

    BigNumMatrix getRow(const unsigned int rowIndex) const;
    BigNumMatrix getCol(const unsigned int colIndex) const;

    friend std::ostream& operator<<(std::ostream& os, const BigNumMatrix& toPrint)
    {
        for (int i = 0; i < toPrint.rowNum; i++)
        {
            for (int j = 0; j < toPrint.colNum; j++)
            {
                os << toPrint.matrix[i][j] << " ";
            }
            os << std::endl;
        }
        return os;
    }

    unsigned int getRank();
    BigNumMatrix inverse();
    BigNumMatrix transpose() const;
    void roundToZero();
    void swapRows(unsigned int row1, unsigned int row2);
private:
    void init(unsigned int rowNum, unsigned int colNum, mpf_class filler, unsigned int prec, mpf_class threshold);
    void erase();
};

#endif /*BIGNUMMATRIX_H*/