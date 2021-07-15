#include <eigen3/Eigen/Core>
#include <math.h>
#include "geographicalVector.h"
#include <assert.h>
#include <iostream>

using namespace std;

GeographicalVector::GeographicalVector():
    x(0),y(0),z(0),fi(0),lambda(0),h(0),a(0),b(0),e(0),N(0)
{
}



GeographicalVector::GeographicalVector(double firstCoord, double secondCoord, double thirdCoord, double a, double e, bool isDescartes)
{
    init(firstCoord, secondCoord, thirdCoord, a, e, isDescartes);
}

GeographicalVector::GeographicalVector(const GeographicalVector& other)
{
    *this = other;
}

GeographicalVector::GeographicalVector(const Vector3d& other, double a, double e, bool isDescartes)
{
    init(other[0], other[1], other[2], a, e, isDescartes);
}

GeographicalVector::~GeographicalVector()
{
}

GeographicalVector GeographicalVector::operator+(const Vector3d& other)
{
    GeographicalVector retval(x + other[0], y + other[1], z + other[2], a, e);
    return retval;
}

GeographicalVector GeographicalVector::operator-(const Vector3d& other)
{
    GeographicalVector retval(x - other[0], y - other[1], z - other[2], a, e);
    return retval;
}

Vector3d GeographicalVector::operator-(const GeographicalVector& other)
{
    Vector3d output(this->x - other.x, this->y - other.y, this->z - other.z);
    return output;
}

GeographicalVector& GeographicalVector::operator=(const GeographicalVector& other)
{
    this->x = other.x;
    this->y = other.y;
    this->z = other.z;

    this->fi = other.fi;
    this->lambda = other.lambda;
    this->h = other.h;

    this->N = other.N;

    this->a = other.a;
    this->b = other.b;
    this->e = other.e;
    return *this;
}

bool GeographicalVector::operator==(const GeographicalVector& other)
{
    return ((this->x - other.x) < 0.0001) &&
           ((this->y - other.y) < 0.0001) &&
           ((this->z - other.z) < 0.0001);
}

GeographicalVector::operator Vector3d()
{
    Vector3d output(x, y, z);
    return output;
}

void GeographicalVector::init(double firstCoord, double secondCoord, double thirdCoord, double a, double e, bool isDescartes)
{
    this->a = a;
    this->e = e;
    b = sqrt((1 - pow(e, 2)) * pow(a, 2));

    if (isDescartes)
    {
        x = firstCoord;
        y = secondCoord;
        z = thirdCoord;

        lambda = atan2(y,  x);

        double p = sqrt(pow(x, 2) + pow(y, 2));

        double finull = atan(z/ (p * (1 - pow(e, 2))));
        double Nnull = pow(a, 2) / sqrt(pow(a * cos(finull), 2) + pow(b * sin(finull), 2));
        h = p / cos(finull) - Nnull;
        fi = atan(z  / (p * (1 - pow(e, 2) * Nnull / (Nnull + h))));
        while (abs(finull - fi) > 0.00001)
        {
            finull = fi;
            Nnull = pow(a, 2) / sqrt(pow(a * cos(finull), 2) + pow(b * sin(finull), 2));
            h = p / cos(finull) - Nnull;
            fi = atan(z  / (p * (1 - pow(e, 2) * Nnull / (Nnull + h))));
        }
    }
    else
    {
        fi = firstCoord;
        lambda = secondCoord;
        h = thirdCoord;

        N = a / (1 - pow(e, 2) * pow(sin(fi), 2));

        x = (N + h) * cos(fi) * cos(lambda);
        y = (N + h) * cos(fi) * sin(lambda);
        z = ((1 - pow(e, 2)) * N + h) * sin(fi);
    }
}