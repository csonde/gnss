#ifndef ALAKMATRIX_H
#define ALAKMATRIX_H

#include <eigen3/Eigen/Core>
#include "geographicalVector.h"
#include "measurement.h"
#include <vector>
#include <map>

using namespace Eigen;
using namespace std;

typedef map<unsigned int, map<long, GeographicalVector> > sat_coord_t;
typedef map<string, GeographicalVector> rec_coord_t;
typedef vector<Measurement> measurement_vector_t;
typedef vector<map <unsigned int, double> > vertical_profile_t;

class Alakmatrix
{
public:
    static const double freqP2 = 1227600000;
    static const double freqC1 = 1575420000;
    static const double e = 0.0818191908429643;
    static const double a = 6378137;

    unsigned int         K;
    unsigned int         N;
    //ascending height order
    vertical_profile_t   verticalProfiles;
    sat_coord_t          satCoords;
    rec_coord_t          recCoords;

    measurement_vector_t measurements;

    MatrixXd             alakmatrix;

    unsigned int         sectorCount;

    double               commonCoeff;

    Alakmatrix(unsigned int         K,
               unsigned int         N,
               const vertical_profile_t&   verticalProfiles,
               const sat_coord_t&          satCoords,
               const rec_coord_t&          recCoords,
               unsigned int         sectorCount,
               const measurement_vector_t& measurements);

    ~Alakmatrix();

private:
    void calcSumCoeffs(unsigned int rowIndex, unsigned int k, unsigned int n, unsigned int m);

    void calcDCBCoeffs(unsigned int rowIndex);

    void calcRow(unsigned int rowIndex);

public:
    static double calcLegendrePolynomial(unsigned int n, unsigned int m, double x);

    MatrixXd calcAlakmatrix();
};

#endif /*ALAKMATRIX_H*/