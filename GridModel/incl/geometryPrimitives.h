#ifndef GEOMETRY_PRIMITIVES_H
#define GEOMETRY_PRIMITIVES_H

#define epsilon 0.000001


//=============================
// Struct describing a vector.
//=============================
typedef struct Vector
{
    double x;
    double y;
    double z;

    double length;
}Vector;

/* 
 * Creator functions that creates a vector with the given coordinates.
 * Length is calculated from coordinates.
 */
Vector createVector(double x, double y, double z);

/*
 * This function returns the dot Product of the 2 vector.
 * dot product : |v1| * |v2| * cos (theta)
 * theta is the angle between v1 and v2
 */
double dotProduct(Vector v1, Vector v2);

/*
 * This function returns the cross Product of the 2 vector.
 * dot product : |v1| * |v2| * sin (theta) * n
 * theta is the angle between v1 and v2
 * n is the normal vector of the plane defined by v1 and v2
 */
Vector crossProduct(Vector v1, Vector v2);

/*
 * This function reduces the coordinates of the given vector
 * in a way that its orientation remains but the length will
 * be 1.
 */
Vector normalizeVector(Vector v1);

/*
 * This function returns the sum of the two vectors.
 */
Vector addVector(Vector v1, Vector v2);

/*
 * This function subtracts v1 from v2.
 */
Vector subtractVector(Vector v1, Vector v2);

/*
 * This function multiplies v.
 */
Vector scalarVectorMult(double s, Vector v);


//============================================
// Struct describing an axis aligned spheroid
//============================================
typedef struct Spheroid
{
    Vector center;

    double a;
    double b;
    double e;
}Spheroid;

/*
 * Creator function for spheroids
 * in mode 1 p1 = a, p2 = b
 * in mode 2 p1 = a, p2 = e
 * default mode is mode 1 (default means that some other value is passed)
 */
Spheroid createSpheroid(Vector center, double p1, double p2, int mode);


//===========================
// Struct describing a Plane
//===========================
typedef struct Plane
{
    Vector point;
    double d;

    Vector normal;
}Plane;

/*
 * Creator function for planes. Parameter d is calculated from p and normal.
 */
Plane createPlane(Vector p, Vector normal);

/*
 * This funtcion checks that the point specified by v is on which side of
 * the plane p. Positive value is in normal direction. If the result is
 * smaller than +-epsilon, 0 is returned, which means the point is on the
 * plane.
 */
double checkVectorAgainstPlane(Vector v, Plane p);

/*
 * This funtcion checks that the point specified by v is on which side of
 * the spheroid s. Positive value is outside the spheroid. If the result is
 * smaller than +-epsilon, 0 is returned. which means the point is on the
 * spheroid.
 */
double checkVectorAgainstSpheroid(Vector v, Spheroid s);


//==========================
// Struct describing a Line
//==========================
typedef struct Line
{
    Vector p1;
    Vector p2;

    Vector orientation;
}Line;

/*
 * Creating function for lines. Orientation is calculated from p1 and p2.
 */
Line createLine(Vector p1, Vector p2);

/*
 * This function calculates the intersection of line l and spheroid s.
 * The return value is the number intersections (0, 1 or 2). The results
 * are stored in "results". The caller must allocated memory for this, so
 * an array of 2 Vectors should be passed.
 */
int intersectLineSpheroid(Line l, Spheroid s, Vector* results);

/*
 * This function calculates the intersection of line l and Plane p.
 * The return value indicates the relation of the line to the plane.
 * 0 means it crosses it, 1 means its on the plane, 2 means its paralel.
 * The result (if there is any) is stored in "result". The caller must
 * allocated memory for this, so the address of a Vector should be passed.
 */
int intersectLinePlane(Line l, Plane p, Vector* result);


//===============================
// Struct describing a Tesseroid
//===============================
typedef struct Tesseroid
{
    Vector* vertexes;
    int vertexCount;
    Plane* boundaryPlanes;
    int planeCount;
    //these should be deleted from a global pointer, as they will get their values from one
    Spheroid* boundarySpheroids[2];
}Tesseroid;

/*
 * Initializes the given tesseroid with full zero values.
 */
void initTesseroid(Tesseroid* t);

/*
 * Frees all the pointers of the given tesseroid.
 */
void deleteTesseroid(Tesseroid* t);

#endif //GEOMETRY_PRIMITIVES_H