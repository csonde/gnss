#include "geometryPrimitives.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

double calculateVectorLength(Vector v)
{
    return sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

Vector createVector(double x, double y, double z)
{
    Vector v;
    v.x = x;
    v.y = y;
    v.z = z;
    v.length = calculateVectorLength(v);
    return v;
}

double dotProduct(Vector v1, Vector v2)
{
    return (v1.x * v2.x + v1.y * v2.y + v1.z * v2.z);
}

Vector crossProduct(Vector v1, Vector v2)
{
    Vector v;
    v.x = v1.y * v2.z - v1.z * v2.y;
    v.y = v1.z * v2.x - v1.x * v2.z;
    v.z = v1.x * v2.y - v1.y * v2.x;
    v.length = calculateVectorLength(v);
    return v;
}

Vector normalizeVector(Vector v)
{
    if (v.length < epsilon)
    {
        return v;
    }
    Vector retVec;
    retVec.x = v.x / v.length;
    retVec.y = v.y / v.length;
    retVec.z = v.z / v.length;
    retVec.length = calculateVectorLength(retVec);
    return retVec;
}

Vector addVector(Vector v1, Vector v2)
{
    Vector v;
    v.x = v1.x + v2.x;
    v.y = v1.y + v2.y;
    v.z = v1.z + v2.z;

    v.length = calculateVectorLength(v);
    return v;
}

Vector subtractVector(Vector v1, Vector v2)
{
    Vector v;
    v.x = v2.x - v1.x;
    v.y = v2.y - v1.y;
    v.z = v2.z - v1.z;
    v.length = calculateVectorLength(v);
    return v;
}

Vector scalarVectorMult(double s, Vector v)
{
    Vector retVec;
    retVec.x = s * v.x;
    retVec.y = s * v.y;
    retVec.z = s * v.z;
    retVec.length = s * v.length;
    return retVec;
}

double calculateSpheroidEccentricity(double a, double b)
{
    if (b > a)
    {
        double temp = a;
        a = b;
        b = temp;
    }
    return sqrt(1 - (b * b) / (a * a));
}

double calculateSpheroidMinorAxis(double a, double e)
{
    return sqrt(a * a - e * e * a * a);
}

Spheroid createSpheroid(Vector center, double p1, double p2, int mode)
{
    Spheroid sph;
    sph.center = center;
    sph.a = p1;
    if (mode == 2)
    {
        sph.e = p2;
        sph.b = calculateSpheroidMinorAxis(sph.a, sph.e);
    }
    else
    {
        sph.b = p2;
        sph.e = calculateSpheroidEccentricity(sph.a, sph.b);
    }
    return sph;
}

Plane createPlane(Vector point, Vector normal)
{
    Plane p;
    p.point = point;
    p.normal = normal;
    p.normal = normalizeVector(p.normal);
    p.d = -1 * dotProduct(normal, point);
    return p;
}

double checkVectorAgainstPlane(Vector v, Plane p)
{
    double retVal = dotProduct(v, p.normal) + p.d;
    if (retVal < epsilon && retVal > -epsilon)
    {
        retVal = 0;
    }
    return retVal;
}

double checkVectorAgainstSpheroid(Vector v, Spheroid s)
{
    double retVal = (pow(v.x, 2) + pow(v.y, 2)) / pow(s.a, 2) + pow(v.z, 2) / pow(s.b, 2) - 1;
    if (retVal < epsilon && retVal > -epsilon)
    {
        retVal = 0;
    }
    return retVal;
}

Line createLine(Vector p1, Vector p2)
{
    Line l;
    l.p1 = p1;
    l.p2 = p2;
    l.orientation = subtractVector(p1, p2);
    l.orientation = normalizeVector(l.orientation);
    return l;
}

//the caller allocates memory for result
int intersectLinePlane(Line l, Plane p, Vector* result)
{
    //check if the line and the plane are paralel
    double denom = dotProduct(p.normal, subtractVector(l.p1, l.p2));
    double isPlanePoint = -dotProduct(p.normal, l.p1) - p.d;
    if (denom < epsilon && denom > -epsilon)
    {
        //the line is on the plane
        if (isPlanePoint < epsilon && isPlanePoint > -epsilon)
        {
            return 1;
        }
        return 2;
    }
    double t = isPlanePoint/denom;
    *result = addVector(l.p1, scalarVectorMult(t, subtractVector(l.p1, l.p2)));
    return 0;
}

//the caller allocates memory for results
int solveSecondDegreePolyReal(double a, double b, double c, double* results)
{
    double discriminant = b * b - 4 * a * c;
    if (discriminant < 0)
    {
        return 0;
    }
    if (discriminant == 0)
    {
        results[0] = - b / (2 * a);
        return 1;
    }
    results[0] = (- b + sqrt(discriminant)) / (2 * a);
    results[1] = (- b - sqrt(discriminant)) / (2 * a);
    return 2;
}

//the caller allocates memory for results
int intersectLineSpheroid(Line l, Spheroid s, Vector* results)
{
    double a = (pow((l.p2.x - l.p1.x), 2) +
               pow((l.p2.y - l.p1.y), 2)) / pow(s.a, 2) +
               pow((l.p2.z - l.p1.z), 2) / pow(s.b, 2);
    double b = 2 * (((l.p2.x - l.p1.x) * l.p1.x +
               (l.p2.y - l.p1.y) * l.p1.y) / pow(s.a, 2) +
               ((l.p2.z - l.p1.z) * l.p1.z) / pow(s.b, 2));
    double c = (pow(l.p1.x, 2) + pow(l.p1.y, 2)) / pow(s.a, 2) +
               pow(l.p1.z, 2) / pow(s.b, 2) - 1;
    double t[2];
    int resultNum = solveSecondDegreePolyReal(a, b, c, t);
    if (resultNum == 0)
    {
        return 0;
    }
    results[0] = addVector(l.p1, scalarVectorMult(t[0], subtractVector(l.p1, l.p2)));
    if (resultNum == 1)
    {
        return 1;
    }
    results[1] = addVector(l.p1, scalarVectorMult(t[1], subtractVector(l.p1, l.p2)));
    return 2;
}

void initTesseroid(Tesseroid* t)
{
    memset(t, 0, sizeof(Tesseroid));
}

void deleteTesseroid(Tesseroid* t)
{
    if (t->vertexes)
    {
        free(t->vertexes);
    }
    if (t->boundaryPlanes)
    {
        free(t->boundaryPlanes);
    }
    memset(t, 0, sizeof(Tesseroid));
}