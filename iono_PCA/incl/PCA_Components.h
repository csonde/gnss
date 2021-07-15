#ifndef PCA_COMPONENTS_H
#define PCA_COMPONENTS_H

#include <eigen3/Eigen/Core>

using namespace Eigen;

class PCA_Components
{
public:
    MatrixXcd principalComponents;
    MatrixXcd partialVariances;
    PCA_Components(){};
    PCA_Components(const PCA_Components & other)
    {
        this->principalComponents = other.principalComponents;
        this->partialVariances = other.partialVariances;
    }
    ~PCA_Components(){};
};

#endif /*PCA_COMPONENTS_H*/