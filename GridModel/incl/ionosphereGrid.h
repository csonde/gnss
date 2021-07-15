#ifndef IONOSPHERE_GRID_H
#define IONOSPHERE_GRID_H

#include "geometryPrimitives.h"

extern const Spheroid WGS84_Spheroid;

extern const double ionosphereLowerBound;
extern const double ionosphereUpperBound;

//====================================================
// Representing struct for a geographical coordinates
//====================================================
// values are in radians
// ranges in degress:
// long: -180 - 180
// lat: -90 - 90
typedef struct GeoCoord
{
    double longitude;
    double latitude;
}GeoCoord;

//mode 1: degrees
//mode 2: radians
//default: mode 1
GeoCoord createGeoCoord(double latitude, double longitude, int mode);

/*
 * This function creates a tesseroid. The two boundary spheroid are
 * considered geocentrical. The boundary planes are specified by 3
 * points. This is 3 point is the origo, and 2 points on the lower
 * boundary spheroid derived from 2 adjacent geographic coordinates
 * from "vertexes". The first and the last geoCoord also considered
 * adjacent.
 */
Tesseroid createGeoTesseroid(Spheroid* lowerBound, Spheroid* upperBound, GeoCoord* vertexes, int vertexCount);


//=================================================================
// Representing struct for a cell that has a "quadratic" tesseroid
//=================================================================
typedef struct QuadraticGridCell
{
    Tesseroid t;
    // indexes of the cell inside the model
    int layerId;
    int lateralId;
    int longitudinalId;
    /*
     * Pointers to adjacent cells
     * 0: lower
     * 1: upper
     * 2: northern
     * 3: eastern
     * 4: southern
     * 5: western
     */
    struct QuadraticGridCell* neighbors[6];
}QuadraticGridCell;

void initQuadraticGridCell(QuadraticGridCell* cell);
void deleteQuadraticGridCell(QuadraticGridCell* cell);


//============================================================
// Representing struct for a cell that has a "quadratic" grid
//============================================================
//latitude over 90 degrees will be discarded
//layer numbering: 0 = lowest
//geoccord numbering: 0 - 0 = northwestern cell;
typedef struct QuadraticGrid
{
    GeoCoord center;
    QuadraticGridCell*** cells;
    Spheroid* boundarySpheroids;
    int eastNum;
    int westNum;
    int northNum;
    int southNum;
    int layerNum;
    //radians
    double latitudeUnit;
    double longitudeUnit;
}QuadraticGrid;

void initQuadraticGrid(QuadraticGrid* grid);
void deleteQuadraticGrid(QuadraticGrid* grid);

/*
 * This function returns a single index of the cell in the grid specified by
 * 3 indexes. The single index is the position in the top->lower, north->south,
 * west->east, ordering od the cells.
 */
int getSingleCellIDByIndexes(int layerIndex, int nsIndex, int weIndex, QuadraticGrid* grid);


/*
 * This function creates a quadratic grid with the given parameters.
 * The angle of one cell will be latitudeUnit and longitudeUnit if 
 * it is not truncated. Cell number limit is in one direction (like
 * radius, or rather major/minor axis) so total number of cells will
 * be doubled. If latitude reaches absolute +/-90 degree or longitute
 * relative +/-180 the model will be truncated. Latitude/longitude unit
 * should be passed as degrees, and must be always positive.
 */
QuadraticGrid createQuadraticGrid(GeoCoord center, double latitudeUnit, double longitudeUnit, int layerNum, int cellNumLimitLat, int cellNumLimitLong);


//====================================================
// Representing struct for a Line sector list element
//====================================================
typedef struct LineSectorList
{
    //pointer to the next sector
    struct LineSectorList* next;
    //identifiers for the cell that contains the sector
    int layerId;
    int lateralId;
    int longitudinalId;
    //length of the line sector
    double length;
    //coordinates of the entry and exit point of the line sector
    //in the cell with sat->rec direction.
    Vector cellIntersectionEntry;
    Vector cellIntersectionExit;
}LineSectorList;

void initLineSectorList(LineSectorList* lsl);
void deleteLineSectorList(LineSectorList** lsl);

/*
 * This function returns the line sectors of the line specified by
 * satPos and recPos across the grid. All receivers' geographical
 * coordinates must be inside the grid model. Any different cases
 * are unhandled. Trajectories that enter the model from N/S/W/E
 * directions will be discarded. Trajectories that have both ends
 * inside the model, but cross the most eastern/western boundaries,
 * will probably cause segfault. This should be fixed in the future
 * and those trajectories should rather be discarded.
 */
LineSectorList* getLineSectorsFromModel(Vector satPos, Vector recPos, QuadraticGrid* grid);

#endif //IONOSPHERE_GRID_H