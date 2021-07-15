#include "bigNumMatrix.h"
#include <assert.h>
#include <iostream>

using namespace std;

BigNumMatrix::BigNumMatrix()
{
    init(3, 3, mpf_class(0, 128), 128, mpf_class("1e-30", 128));
}

BigNumMatrix::BigNumMatrix(const BigNumMatrix& other)
{
    matrix = 0;
    *this = other;
}

BigNumMatrix::BigNumMatrix(const MatrixXd& other)
{
    init(other.innerSize(), other.outerSize(), mpf_class(0, 128), 128, mpf_class("1e-30", 128));
    for (int i = 0; i < rowNum; i++)
    {
        for (int j = 0; j < colNum; j++)
        {
            matrix[i][j] = mpf_class(other(i, j), 128);
        }
    }
}

BigNumMatrix::BigNumMatrix(unsigned int rowNum, unsigned int colNum, unsigned int prec)
{
    init(rowNum, colNum, mpf_class(0, prec), prec, mpf_class("1e-30", prec));
}

BigNumMatrix::BigNumMatrix(unsigned int rowNum, unsigned int colNum, mpf_class filler, unsigned int prec, mpf_class threshold)
{
    init(rowNum, colNum, filler, prec, threshold);
}

/*BigNumMatrix::BigNumMatrix(unsigned int rowNum, unsigned int colNum, mpf_class filler, unsigned int prec, mpf_class threshold)
{
    init(rowNum, colNum, filler, prec, threshold);
}*/

BigNumMatrix::~BigNumMatrix()
{
    erase();
}

void BigNumMatrix::setPrec(unsigned int prec)
{
    this->prec = prec;
    for (int i = 0; i < rowNum; i++)
    {
        for (int j = 0; j < colNum; j++)
        {
            matrix[i][j] = mpf_class(matrix[i][j], prec);
        }
    }
}

unsigned int BigNumMatrix::getPrec() const
{
    return prec;
}

unsigned int BigNumMatrix::getColNum() const
{
    return colNum;
}

unsigned int BigNumMatrix::getRowNum() const
{
    return rowNum;
}

void BigNumMatrix::setThreshold(mpf_class threshold)
{
    this->threshold = threshold;
    if (threshold.get_prec() > prec)
    {
        setPrec(threshold.get_prec());
    }
}

mpf_class BigNumMatrix::getThreshold() const
{
    return threshold;
}

mpf_class BigNumMatrix::getElement(const unsigned int rowIndex, const unsigned int colIndex) const
{
    return matrix[rowIndex][colIndex];
}

void BigNumMatrix::resize(unsigned int rowNum, unsigned int colNum)
{
    erase();
    init(rowNum, colNum, mpf_class(0, prec), prec, threshold);
}

BigNumMatrix& BigNumMatrix::operator=(const BigNumMatrix& other)
{
    erase();
    init(other.getRowNum(), other.getColNum(), mpf_class(0, other.getPrec()), other.getPrec(), other.getThreshold());
    for (int i = 0; i < rowNum; i++)
    {
        for (int j = 0; j < colNum; j++)
        {
            matrix[i][j] = other.getElement(i, j);
        }
    }
    return *this;
}

BigNumMatrix BigNumMatrix::operator+(const BigNumMatrix& other) const
{
    assert(this->colNum == other.getColNum() && this->rowNum == other.getRowNum());
    unsigned int maxPrec = this->prec < other.getPrec() ? other.getPrec() : this->prec;
    mpf_class minThreshold = this->threshold < other.getThreshold() ? this->threshold : other.getThreshold();
    BigNumMatrix retMatrix(this->rowNum, this->colNum, mpf_class("0", maxPrec), maxPrec, minThreshold);
    for (int i = 0; i < retMatrix.getRowNum(); i++)
    {
        for (int j = 0; j < retMatrix.getColNum(); j++)
        {
            retMatrix(i, j) = mpf_class(this->matrix[i][j], maxPrec) + mpf_class(other.getElement(i, j), maxPrec);
        }
    }
    return retMatrix;
}

BigNumMatrix BigNumMatrix::operator-(const BigNumMatrix& other) const
{
    assert(this->colNum == other.getColNum() && this->rowNum == other.getRowNum());
    unsigned int maxPrec = this->prec < other.getPrec() ? other.getPrec() : this->prec;
    mpf_class minThreshold = this->threshold < other.getThreshold() ? this->threshold : other.getThreshold();
    BigNumMatrix retMatrix(this->rowNum, this->colNum, mpf_class("0", maxPrec), maxPrec, minThreshold);
    for (int i = 0; i < retMatrix.getRowNum(); i++)
    {
        for (int j = 0; j < retMatrix.getColNum(); j++)
        {
            retMatrix(i, j) = mpf_class(this->matrix[i][j], maxPrec) - mpf_class(other.getElement(i, j), maxPrec);
        }
    }
    return retMatrix;
}

BigNumMatrix BigNumMatrix::operator*(const BigNumMatrix& other) const
{
    assert(this->colNum == other.getRowNum());
    unsigned int maxPrec = this->prec < other.getPrec() ? other.getPrec() : this->prec;
    mpf_class minThreshold = this->threshold < other.getThreshold() ? this->threshold : other.getThreshold();
    BigNumMatrix retMatrix(this->rowNum, other.getColNum(), mpf_class("0", maxPrec), maxPrec, minThreshold);
    for (int i = 0; i < retMatrix.getRowNum(); i++)
    {
        for (int j = 0; j < retMatrix.getColNum(); j++)
        {
            mpf_class sum(0, maxPrec);
            for (int k = 0; k < this->colNum; k++)
            {
                sum += mpf_class(this->matrix[i][k], maxPrec) * mpf_class(other.getElement(k, j), maxPrec);
            }
            retMatrix(i, j) = sum;
        }
    }
    return retMatrix;
}

BigNumMatrix BigNumMatrix::operator*(const mpf_class& scalar) const
{
    BigNumMatrix retVal(rowNum, colNum, prec);
    for (unsigned int i = 0; i < rowNum; i++)
    {
        for (unsigned int j = 0; j < colNum; j++)
        {
            retVal(i,j) = mpf_class(matrix[i][j] * scalar, prec);
        }
    }
    return retVal;
}

mpf_class& BigNumMatrix::operator()(unsigned int rowIndex, unsigned int colIndex)
{
    return matrix[rowIndex][colIndex];
}

const mpf_class& BigNumMatrix::operator()(unsigned int rowIndex, unsigned int colIndex) const
{
    return matrix[rowIndex][colIndex];
}

BigNumMatrix::operator mpf_class()
{
    assert(rowNum == 1 && colNum == 1);
    return matrix[0][0];
}

BigNumMatrix BigNumMatrix::getRow(const unsigned int rowIndex) const
{
    BigNumMatrix retVal(1, colNum, prec);
    for (unsigned int i = 0; i < colNum; i++)
    {
        retVal(0, i) = matrix[rowIndex][i];
    }
    return retVal;
}

BigNumMatrix BigNumMatrix::getCol(const unsigned int colIndex) const
{
    BigNumMatrix retVal(rowNum, 1, prec);
    for (unsigned int i = 0; i < rowNum; i++)
    {
        retVal(i, 0) = matrix[i][colIndex];
    }
    return retVal;
}

unsigned int BigNumMatrix::getRank()
{
    return 0;
}

BigNumMatrix BigNumMatrix::inverse()
{
    if (rowNum != colNum)
    {
        cout << "Matrix is not quadratic, returning original matrix." << endl;
        return *this;
    }
    BigNumMatrix eMatrix = *this;
    BigNumMatrix identity(rowNum, colNum, mpf_class(0, eMatrix.getPrec()), eMatrix.getPrec(), eMatrix.getThreshold());
    for (int i = 0; i < rowNum; i++)
    {
        identity(i, i) = mpf_class(1, identity.getPrec());
    }
    /*cout << "invertion steps" << endl;
    cout << "e:" << endl << eMatrix << endl;
    cout << "i:" << endl << identity << endl;*/

    unsigned int * rowMapping = new unsigned int[eMatrix.getRowNum()];
    for (unsigned int i = 0; i < eMatrix.getRowNum(); i++)
    {
        rowMapping[i] = i;
    }

    for (int i = 0; i < eMatrix.getRowNum(); i++)
    {
        //TODO check if zero
        if (eMatrix(i, i) < eMatrix.getThreshold() && eMatrix(i, i) > mpf_class(-1, eMatrix.getPrec()) * eMatrix.getThreshold())
        {
            if (i == 282)
            {
                cout << "failed element " << eMatrix(i, i) << endl;
                cout << "threshold " << eMatrix.getThreshold() << endl;
            }
            int j = 0;
            for (j = i + 1; j < eMatrix.getRowNum(); j++)
            {
                if (eMatrix(j, i) > eMatrix.getThreshold() || eMatrix(j, i) < mpf_class(-1, eMatrix.getPrec()) * eMatrix.getThreshold())
                {
                    //should use some other method instead of swapping for proper elimination
                    eMatrix.swapRows(i, j);
                    identity.swapRows(i, j);
                    unsigned int tmp = rowMapping[i];
                    rowMapping[i] = rowMapping[j];
                    rowMapping[j] = tmp;
                    break;
                }
            }
            if (j == eMatrix.getRowNum())
            {
                cout << "Matrix proven to be singular in step " << i << endl;
                cout << "Current state of elimination:"  << endl;
                cout << "original index of current row: " << rowMapping[i] << endl;
                cout << eMatrix << endl;
                bool singularMatrix = 0;
                assert(singularMatrix == 1);
            }
        }
        for (int j = 0; j < eMatrix.getRowNum(); j++)
        {
            if (i != j)
            {
                mpf_class q = (eMatrix(j, i) / eMatrix(i, i));
                for (int k = i; k < eMatrix.getColNum(); k++)
                {
                    eMatrix(j, k) = eMatrix(j, k) - q * eMatrix(i, k);
                }
                //TODO put values in inverse
                for (int k = 0; k < identity.getColNum(); k++)
                {
                    identity(j, k) = identity(j, k) - q * identity(i, k);
                }
            }
        }
        mpf_class q = eMatrix(i, i);
        for (int j = 0; j < eMatrix.getColNum(); j++)
        {
            eMatrix(i, j) = eMatrix(i, j) / q;
            identity(i, j) = identity(i, j) / q;
        }
        /*cout << "e:" << endl << eMatrix << endl;
        cout << "i:" << endl << identity << endl;*/
    }
    return identity;
}

BigNumMatrix BigNumMatrix::transpose() const
{
    BigNumMatrix retMat(colNum, rowNum, mpf_class(0, prec), prec, threshold);
    for (int i = 0; i < colNum; i++)
    {
        for (int j = 0; j < rowNum; j++)
        {
            retMat(i, j) = matrix[j][i];
        }
    }
    return retMat;
}

void BigNumMatrix::roundToZero()
{
    for (int i = 0; i < rowNum; i++)
    {
        for (int j = 0; j < colNum; j++)
        {
            if(matrix[i][j] < threshold && matrix[i][j] > mpf_class(-1, prec) * threshold)
            {
                matrix[i][j] = mpf_class(0, prec);
            }
        }
    }
}

void BigNumMatrix::swapRows(unsigned int row1, unsigned int row2)
{
    mpf_class* tmp = matrix[row1];
    matrix[row1] = matrix[row2];
    matrix[row2] = tmp;
}

void BigNumMatrix::init(unsigned int rowNum, unsigned int colNum, mpf_class filler, unsigned int prec, mpf_class threshold)
{
    this->colNum = colNum;
    this->rowNum = rowNum;
    this->prec = prec;
    this->threshold = threshold;
    this->matrix = new mpf_class*[rowNum];
    for (int i = 0; i < rowNum; i++)
    {
        matrix[i] = new mpf_class[colNum];
        for (int j = 0; j < colNum; j++)
        {
            matrix[i][j].set_prec(prec);
            matrix[i][j] = mpf_class(filler, prec);
        }
    }
}

void BigNumMatrix::erase()
{
    if (matrix)
    {
        for (int i = 0; i < rowNum; i++)
        {
            delete[] matrix[i];
        }
        delete[] matrix;
        matrix = 0;
    }
}