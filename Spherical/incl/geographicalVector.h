#ifndef GEOGRAPHICAL_VECTOR_H
#define GEOGRAPHICAL_VECTOR_H

#include <eigen3/Eigen/Core>

using namespace Eigen;

class GeographicalVector
{
public:
    double x;
    double y;
    double z;

    double fi;
    double lambda;
    double h;

    double a;
    double b;
    double e;

    double N;

    GeographicalVector();
    GeographicalVector(double firstCoord, double secondCoord, double thirdCoord, double a, double e, bool isDescartes = true);
    GeographicalVector(const GeographicalVector& other);
    GeographicalVector(const Vector3d& other, double a, double e, bool isDescartes = true);
    ~GeographicalVector();

    GeographicalVector operator+(const Vector3d& other);
    GeographicalVector operator-(const Vector3d& other);
    Vector3d operator-(const GeographicalVector& other);
    GeographicalVector& operator=(const GeographicalVector& other);
    bool operator==(const GeographicalVector& other);
    operator Vector3d();

private:
    void init(double firstCoord, double secondCoord, double thirdCoord, double a, double e, bool isDescartes = true);
};

#endif /*GEOGRAPHICAL_VECTOR_H*/