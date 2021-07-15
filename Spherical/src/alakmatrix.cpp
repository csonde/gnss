#include "alakmatrix.h"
#include "almanac.h"

#include <eigen3/Eigen/Geometry>
#include <iostream>

const double commonDen = 1000000;

Alakmatrix::Alakmatrix(unsigned int         K,
                       unsigned int         N,
                       const vertical_profile_t&   verticalProfiles,
                       const sat_coord_t&          satCoords,
                       const rec_coord_t&          recCoords,
                       unsigned int         sectorCount,
                       const measurement_vector_t& measurements)
{
    this->K = K;
    this->N = N;
    assert(verticalProfiles.size() == K);
    this->verticalProfiles = verticalProfiles;
    this->satCoords = satCoords;
    this->recCoords = recCoords;
    this->sectorCount = sectorCount;
    this->measurements = measurements;
    this->commonCoeff = 0;
    unsigned int colNum = K * (N + 1) * N  / 2 * 2 - K * N /*+ satCoords.size() + recCoords.size()*/;
    alakmatrix = MatrixXd(measurements.size(), colNum);
}

Alakmatrix::~Alakmatrix()
{
}

double Alakmatrix::calcLegendrePolynomial(unsigned int n, unsigned int m, double angle)
{
    assert (n >= m);
    if (n == 0 && m == 0)
    {
        return 1;
    }
    if (n == 1 && m == 0)
    {
        return sqrt(3) * cos(angle);
    }
    if (n == 1 && m == 1)
    {
        return sqrt(3) * sin(angle);
    }
    if (n == 2 && m == 0)
    {
        return 0.5 * sqrt(5) * (3 * pow(cos(angle), 2) - 1);
    }
    if (n == 2 && m == 1)
    {
        return sqrt(15) * cos(angle) * sin(angle);
    }
    if (n == 2 && m == 2)
    {
        return 0.5 * sqrt(15) * pow(sin(angle), 2);
    }
    if (n == m)
    {
        return sqrt((2.0 * (double)n + 1.0) / (2.0 * (double)n)) * sin(angle) * calcLegendrePolynomial(n-1, n-1, angle);
    }
    if (n == (m + 1))
    {
        return sqrt(2.0 * (double)n + 1.0) * cos(angle) * calcLegendrePolynomial(n-1, n-1, angle);
    }
    return sqrt(((2.0 * (double)n + 1.0) * (2.0 * (double)n - 1.0)) / (((double)n + (double)m) * ((double)n - (double)m))) * cos(angle) * calcLegendrePolynomial(n-1, m, angle) -
            sqrt(((2.0 * (double)n + 1.0) * ((double)n + (double)m - 1.0) * ((double)n - (double)m - 1.0)) / ((2.0 * (double)n - 3.0) * ((double)n + (double)m) * ((double)n - (double)m))) * calcLegendrePolynomial(n-2, m, angle);
}

// k = 0..K-1, n = 0..N, m=0..n
void Alakmatrix::calcSumCoeffs(unsigned int rowIndex, unsigned int k, unsigned int n, unsigned int m)
{
    //std::cout << "k = " << k << "    n = " << n << "    m = " << m << std::endl;
    //get receiver-satellite vector data
    Measurement& currMeas = measurements[rowIndex];
    Vector3d orientation = satCoords[currMeas.satId][calculateTimeOfGPSWeek(currMeas.gpsTime)] - recCoords[currMeas.recId];
    double distance = orientation.norm();
    orientation.normalize();
    //get vertical profile
    map<unsigned int, double>& currVertProf = verticalProfiles[k];
    //get position in the matrix
    unsigned int colIndexA = k * (N + 1) * N / 2 + (n + 1) * n / 2 + m;

    unsigned int colIndexB = K * (N + 1) * N / 2 +
                             k * (N + 1) * N / 2 + (n + 1) * n / 2 + m -
                             k * N - (n + 1);
    //get integration interval
    double dS = distance / sectorCount;

    //integral sums
    double integralA = 0;
    double integralB = 0;

    GeographicalVector sectionMid = recCoords[currMeas.recId] + orientation * 0.5 * dS;
    //integrate

    for (unsigned int i = 0; i < sectorCount; i++)
    {
        //interval coordinates
        //nearest known electrondensity data
        map<unsigned int, double>::iterator lowerBoundIt = currVertProf.begin();
        map<unsigned int, double>::iterator upperBoundIt = currVertProf.begin();

        //finding nearest known electrondensity data
        map<unsigned int, double>::iterator endIt = currVertProf.begin();
        for (endIt; endIt != currVertProf.end(); endIt++)
        {
            //we have found it
            if (sectionMid.h < endIt->first)
            {
                upperBoundIt = endIt;
                break;
            }
            //no match ascending
            else
            {
                lowerBoundIt = endIt;
            }
        }
        //under the lowest data (lower atmosphere), add 0 to sum
        if (endIt == currVertProf.begin() && upperBoundIt == currVertProf.begin())
        {
            /*if (rowIndex == 15)
            {
                std::cout<<"15os sor also legkor"<<std::endl;
            }*/
            sectionMid = sectionMid + orientation * dS;
            continue;
        }
        //over the highest data (outer space), add 0 to sum
        if (endIt == currVertProf.end() && upperBoundIt == currVertProf.begin())
        {
            /*if (rowIndex == 15)
            {
                std::cout<<"sectionMid.h: " << sectionMid.h <<std::endl;
            }*/
            sectionMid = sectionMid + orientation * dS;
            continue;
        }

        //calculate the vertical profile value
        double partialProfileGrad = (upperBoundIt->second - lowerBoundIt->second) / (upperBoundIt->first - lowerBoundIt->first);
        double profileValue = lowerBoundIt->second + (sectionMid.h - lowerBoundIt->first) * partialProfileGrad;

        if (k == 0)
        {
            //std::cout << "lowerBoundIt->second: " << lowerBoundIt->second <<  "    sectionMid.h:" << sectionMid.h << "    lowerBoundIt->first: " << lowerBoundIt->first << "    partialProfileGrad: " << partialProfileGrad << std::endl;
            //std::cout << "lower bound: " << lowerBoundIt->first << "    upperbound: " << upperBoundIt->first << "    lowerProfValue: " << lowerBoundIt->second << "    upperProfValue: " << upperBoundIt->second << "    profileValue: " << profileValue << std::endl;
        }

        //calculate legendre polynomial
        double Pnm = calcLegendrePolynomial(n, m, sectionMid.fi);

        //add rectangle to integral sum
        integralA += cos(m * sectionMid.lambda) * Pnm * profileValue * dS;
        if (m != 0)
        {
            integralB += sin(m * sectionMid.lambda) * Pnm * profileValue * dS;
        }
        sectionMid = sectionMid + orientation * dS;
        /*if(k == 0 && n == 0 && m == 0)
        {
            std::cout << "n: " << n << "    m: " << m << "    lambda: " << sectionMid.lambda << "    Pnm: " << Pnm << "    sectionMid.h: " << sectionMid.h << "    profileValue: " << profileValue << "    dS: " << dS << std::endl;
            std::cout << "integral b: " << integralB << endl;
        }*/
    }
    //put values in matrix
    alakmatrix(rowIndex, colIndexA) = integralA * commonCoeff;
    if(m != 0)
    {
        alakmatrix(rowIndex, colIndexB) = integralB * commonCoeff;
    }
}

void Alakmatrix::calcDCBCoeffs(unsigned int rowIndex)
{
    unsigned int colOffset = K * (N + 1) * N  / 2 * 2 - K * N;
    Measurement& currMeas = measurements[rowIndex];
    unsigned int satIndex = 0;
    unsigned int recIndex = 0;
    for (sat_coord_t::iterator it = satCoords.begin(); it != satCoords.end(); it++, satIndex++)
    {
        if (it == satCoords.find(currMeas.satId))
        {
            assert(!(it->second.empty()));
            break;
        }
    }
    for (rec_coord_t::iterator it = recCoords.begin(); it != recCoords.end(); it++, recIndex++)
    {
        if (it == recCoords.find(currMeas.recId))
        {
            break;
        }
    }
    /*if (satCoords.size() + recIndex == 85)
    {
        cout << "hibas allomas: " << currMeas.recId << endl;
    }*/
    for (unsigned int i = 0; i < satCoords.size() + recCoords.size(); i++)
    {
        if (i == satIndex)
        {
            alakmatrix(rowIndex, colOffset + i) = 1 / commonCoeff /*/ 100000000000*/;
        }
        else if ((i == satCoords.size() + recIndex))
        {
            alakmatrix(rowIndex, colOffset + i) = 1 / commonCoeff /*/ 100000000000*/;
        }
        else
        {
            alakmatrix(rowIndex, colOffset + i) = 0;
        }
    }
}

void Alakmatrix::calcRow(unsigned int rowIndex)
{
    for (unsigned int k = 0; k < K; k++)
    {
        for (unsigned int n = 0; n < N; n++)
        {
            for (unsigned int m = 0; m <= n; m++)
            {
                calcSumCoeffs(rowIndex, k, n, m);
            }
        }
    }
    //calcDCBCoeffs(rowIndex);
}

MatrixXd Alakmatrix::calcAlakmatrix()
{
    commonCoeff = 40.3 * (freqP2 * freqP2 - freqC1 * freqC1) / (freqC1 * freqC1 * freqP2 * freqP2);
    for (unsigned int i = 0; i < measurements.size(); i++)
    {
        calcRow(i);
    }
    return alakmatrix;
}
