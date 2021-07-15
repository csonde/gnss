#ifndef COORDINATES_H
#define COORDINATES_H

#include <ionosphereGrid.h>

typedef struct GPSSatCoords
{
    long gpsTime;
    Vector** coordinates;
    struct GPSSatCoords* left;
    struct GPSSatCoords* right;
}GPSSatCoords;

GPSSatCoords* createGPSSatCoords(long gpsTime, char* almanacFile);
int insertGPSSatCoords(long gpsTime, char* almanacFile, GPSSatCoords** gpsSatCoordsTree);
Vector** getGPSSatCoords(long gpsTime, GPSSatCoords* gpsSatCoordsTree);
void deleteGPSSatCoordsTree(GPSSatCoords** gpsSatCoordsTree);


typedef struct StationCoord
{
    char stationId[5];
    Vector coord;
    struct StationCoord* left;
    struct StationCoord* right;
}StationCoord;

StationCoord* createStationCoord(char* stationId, Vector coord);
int insertStationCoord(char* stationId, Vector coord, StationCoord** stationCoordTree);
Vector* getStationCoord(char* stationId, StationCoord* stationCoordTree);
void deleteStationCoordTree(StationCoord** stationCoordTree);
void printStationCoordTree(StationCoord* stationCoordTree);


typedef struct Measurement
{
    long gpsTime;
    int satId;
    char recId[5];
    double C1;
    double P2;
    LineSectorList* lineSectors;
    struct Measurement* next;
}Measurement;

void initMeasurement(Measurement* meas);
int insertMeasurementToListEnd(Measurement* meas, Measurement** measListRoot);
void deleteMeasurementList(Measurement** measListRoot);
void calculateLineSectors(Measurement* meas, GPSSatCoords* gpsSatcoordsRoot, StationCoord* stationCoordsRoot, QuadraticGrid* grid);


typedef struct StationDCB
{
    char stationId[5];
    double dcb;
    int preorderIndex;
    struct StationDCB* left;
    struct StationDCB* right;
}StationDCB;

StationDCB* createStationDCB(char* stationId, double dcb);
int insertStationDCB(char* stationId, double dcb, StationDCB** stationDCBRoot);
double* getStationDCB(char* stationId, StationDCB* stationDCBRoot);
void deleteStationDCBTree(StationDCB** stationDCBRoot);
int doStationDCBPreorderIndexing(StationDCB* stationDCBRoot, int lastIndex);
int getStationDCBPreorderIndex(char* stationId, StationDCB* stationDCBRoot);
void printStationDCBPreorder(StationDCB* stationDCBRoot);


typedef struct SatDCB
{
    int satId;
    double dcb;
    int preorderIndex;
    struct SatDCB* left;
    struct SatDCB* right;
}SatDCB;

SatDCB* createSatDCB(int satId, double dcb);
int insertSatDCB(int satId, double dcb, SatDCB** satDCBRoot);
double* getSatDCB(int satId, SatDCB* satDCBRoot);
void deleteSatDCBTree(SatDCB** satDCBRoot);
int doSatDCBPreorderIndexing(SatDCB* satDCBRoot, int lastIndex);
int getSatDCBPreorderIndex(int satId, SatDCB* satDCBRoot);


typedef struct CellParameter
{
    int cellId;
    double eDensity;
    int preorderIndex;
    struct CellParameter* left;
    struct CellParameter* right;
}CellParameter;

CellParameter* createCellParameter(int cellId, double eDens);
int insertCellParameter(int cellId, double eDens, CellParameter** cellParamRoot);
double* getCellParameter(int cellId, CellParameter* cellParamRoot);
void deleteCellParameter(CellParameter** cellParamRoot);
int doCellParameterPreorderIndexing(CellParameter* cellParamRoot, int lastIndex);
int getCellParameterPreorderIndex(int cellId, CellParameter* cellParamRoot);
void printCellParameterPreorder(CellParameter* cellParamRoot);


typedef struct IonoVariables
{
    StationDCB* stationDCBRoot;
    int stationCount;
    SatDCB* satDCBRoot;
    int satCount;
    CellParameter* cellParameters;
    int cellCount;
    double* corrections;
    int corrCount;
}IonoVariables;

IonoVariables* createIonoVariables(Measurement* measRoot, QuadraticGrid* grid);
void deleteIonoVariables(IonoVariables** IonoVariables);

#endif //COORDINATES_H