#ifndef IONO_PCA_H
#define IONO_PCA_H

#include "PCA_Components.h"

using namespace Eigen;


class Iono_PCA
{
public:
    Iono_PCA();
    ~Iono_PCA();

    void LoadDataFromDir(const char* dir);
    PCA_Components calculatePCAs();

    MatrixXd rawDensityData;
    VectorXi heightData;
};


#endif /*IONO_PCA_H*/