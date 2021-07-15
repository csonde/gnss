#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <almanac.h>
#include <containers.h>


GPSSatCoords* createGPSSatCoords(long gpsTime, char* almanacFile)
{
    GPSSatCoords* retVal = malloc(sizeof(GPSSatCoords));
    memset(retVal, 0, sizeof(GPSSatCoords));
    retVal->gpsTime = gpsTime;
    double t = calculateTimeOfGPSWeek(gpsTime);
    calculateGPSSatellitePositions(t, almanacFile, &(retVal->coordinates));
    retVal->left = 0;
    retVal->right = 0;
    return retVal;
}

int insertGPSSatCoords(long gpsTime, char* almanacFile, GPSSatCoords** gpsSatCoordsTree)
{
    if (!(*gpsSatCoordsTree))
    {
        *gpsSatCoordsTree = createGPSSatCoords(gpsTime, almanacFile);
        return 1;
    }
    if ((*gpsSatCoordsTree)->gpsTime < gpsTime)
    {
        return insertGPSSatCoords(gpsTime, almanacFile, &((*gpsSatCoordsTree)->left));
    }
    else if ((*gpsSatCoordsTree)->gpsTime > gpsTime)
    {
        return insertGPSSatCoords(gpsTime, almanacFile, &((*gpsSatCoordsTree)->right));
    }
    return 0;
}

Vector** getGPSSatCoords(long gpsTime, GPSSatCoords* gpsSatCoordsTree)
{
    if (gpsSatCoordsTree)
    {
        if (gpsSatCoordsTree->gpsTime == gpsTime)
        {
            return gpsSatCoordsTree->coordinates;
        }
        else if (gpsSatCoordsTree->gpsTime < gpsTime)
        {
            return getGPSSatCoords(gpsTime, gpsSatCoordsTree->left);
        }
        else
        {
            return getGPSSatCoords(gpsTime, gpsSatCoordsTree->right);
        }
    }
    return 0;
}

void deleteGPSSatCoordsTree(GPSSatCoords** gpsSatCoordsTree)
{
    if (*gpsSatCoordsTree)
    {
        deleteGPSSatCoordsTree(&((*gpsSatCoordsTree)->left));
        deleteGPSSatCoordsTree(&((*gpsSatCoordsTree)->right));
        if ((*gpsSatCoordsTree)->coordinates)
        {
            int i;
            for (i = 0; i < 32; i++)
            {
                if((*gpsSatCoordsTree)->coordinates[i])
                {
                    free((*gpsSatCoordsTree)->coordinates[i]);
                }
            }
            free((*gpsSatCoordsTree)->coordinates);
        }
        free(*gpsSatCoordsTree);
        *gpsSatCoordsTree = 0;
    }
    return;
}


StationCoord* createStationCoord(char* stationId, Vector coord)
{
    StationCoord* retVal = malloc(sizeof(StationCoord));
    strcpy(retVal->stationId, stationId);
    retVal->coord = coord;
    retVal->left = 0;
    retVal->right = 0;
    return retVal;
}

int insertStationCoord(char* stationId, Vector coord, StationCoord** stationCoordTree)
{
    if(!(*stationCoordTree))
    {
        *stationCoordTree = createStationCoord(stationId, coord);
        return 1;
    }
    if (strcmp(stationId, (*stationCoordTree)->stationId) < 0)
    {
        return insertStationCoord(stationId, coord, &((*stationCoordTree)->left));
    }
    else if (strcmp(stationId, (*stationCoordTree)->stationId) > 0)
    {
        return insertStationCoord(stationId, coord, &((*stationCoordTree)->right));
    }
    return 0;
}

Vector* getStationCoord(char* stationId, StationCoord* stationCoordTree)
{
    if(stationCoordTree)
    {
        if(strcmp(stationCoordTree->stationId, stationId) == 0)
        {
            return &(stationCoordTree->coord);
        }
        else if(strcmp(stationCoordTree->stationId, stationId) > 0)
        {
            return getStationCoord(stationId, stationCoordTree->left);
        }
        else
        {
            return getStationCoord(stationId, stationCoordTree->right);
        }
    }
    return 0;
}

void deleteStationCoordTree(StationCoord** stationCoordTree)
{
    if (*stationCoordTree)
    {
        deleteStationCoordTree(&((*stationCoordTree)->left));
        deleteStationCoordTree(&((*stationCoordTree)->right));
        free(*stationCoordTree);
        *stationCoordTree = 0;
    }
    return;
}

void printStationCoordTree(StationCoord* stationCoordTree)
{
    if(stationCoordTree)
    {
        printStationCoordTree(stationCoordTree->left);
        printf("station ID: %s,     X: %lf,    Y: %lf,    Z: %lf\n", stationCoordTree->stationId,
                                                                     stationCoordTree->coord.x,
                                                                     stationCoordTree->coord.y,
                                                                     stationCoordTree->coord.z);
        printStationCoordTree(stationCoordTree->right);
    }
    return;
}


void initMeasurement(Measurement* meas)
{
    memset(meas, 0, sizeof(Measurement));
}

int insertMeasurementToListEnd(Measurement* meas, Measurement** measListRoot)
{
    if (*measListRoot)
    {
        return insertMeasurementToListEnd(meas, &((*measListRoot)->next));
    }
    else
    {
        *measListRoot = meas;
        return 1;
    }
}

void deleteMeasurementList(Measurement** measListRoot)
{
    if(!(*measListRoot))
    {
        return;
    }
    else
    {
        deleteMeasurementList(&((*measListRoot)->next));
        deleteLineSectorList(&((*measListRoot)->lineSectors));
        free(*measListRoot);
        *measListRoot = 0;
    }
}

void calculateLineSectors(Measurement* meas, GPSSatCoords* gpsSatcoordsRoot, StationCoord* stationCoordsRoot, QuadraticGrid* grid)
{
    Vector** gpsSatCoords = getGPSSatCoords(meas->gpsTime, gpsSatcoordsRoot);
    Vector* satCoord = gpsSatCoords[meas->satId];
    if(!satCoord)
    {
        meas->lineSectors = 0;
        return;
    }
    Vector* recCoord = getStationCoord(meas->recId, stationCoordsRoot);
    meas->lineSectors = getLineSectorsFromModel(*satCoord, *recCoord, grid);
}


StationDCB* createStationDCB(char* stationId, double dcb)
{
    StationDCB* stationDCB = malloc(sizeof(StationDCB));
    memset(stationDCB, 0, sizeof(StationDCB));
    memcpy(stationDCB->stationId, stationId, 5);
    stationDCB->dcb = dcb;
    return stationDCB;
}

int insertStationDCB(char* stationId, double dcb, StationDCB** stationDCBRoot)
{
    if(!(*stationDCBRoot))
    {
        *stationDCBRoot = createStationDCB(stationId, dcb);
        return 1;
    }
    if (strcmp(stationId, (*stationDCBRoot)->stationId) < 0)
    {
        return insertStationDCB(stationId, dcb, &((*stationDCBRoot)->left));
    }
    else if (strcmp(stationId, (*stationDCBRoot)->stationId) > 0)
    {
        return insertStationDCB(stationId, dcb, &((*stationDCBRoot)->right));
    }
    return 0;
}

double* getStationDCB(char* stationId, StationDCB* stationDCBRoot)
{
    if(stationDCBRoot)
    {
        if(strcmp(stationDCBRoot->stationId, stationId) == 0)
        {
            return &(stationDCBRoot->dcb);
        }
        else if(strcmp(stationDCBRoot->stationId, stationId) > 0)
        {
            return getStationDCB(stationId, stationDCBRoot->left);
        }
        else
        {
            return getStationDCB(stationId, stationDCBRoot->right);
        }
    }
    return 0;
}

void deleteStationDCBTree(StationDCB** stationDCBRoot)
{
    if (*stationDCBRoot)
    {
        deleteStationDCBTree(&((*stationDCBRoot)->left));
        deleteStationDCBTree(&((*stationDCBRoot)->right));
        free(*stationDCBRoot);
        *stationDCBRoot = 0;
    }
    return;
}

int doStationDCBPreorderIndexing(StationDCB* stationDCBRoot, int lastIndex)
{
    if(!stationDCBRoot)
    {
        return -1;
    }
    int newLastIndex = lastIndex;
    if(stationDCBRoot->left)
    {
        newLastIndex = doStationDCBPreorderIndexing(stationDCBRoot->left, newLastIndex);
    }
    stationDCBRoot->preorderIndex = newLastIndex;
    newLastIndex++;
    if(stationDCBRoot->right)
    {
        newLastIndex = doStationDCBPreorderIndexing(stationDCBRoot->right, newLastIndex);
    }
    return newLastIndex;
}

int getStationDCBPreorderIndex(char* stationId, StationDCB* stationDCBRoot)
{
    if(stationDCBRoot)
    {
        if(strcmp(stationDCBRoot->stationId, stationId) == 0)
        {
            return stationDCBRoot->preorderIndex;
        }
        else if(strcmp(stationDCBRoot->stationId, stationId) > 0)
        {
            return getStationDCBPreorderIndex(stationId, stationDCBRoot->left);
        }
        else
        {
            return getStationDCBPreorderIndex(stationId, stationDCBRoot->right);
        }
    }
    return -1;
}

void printStationDCBPreorder(StationDCB* stationDCBRoot)
{
    if(!stationDCBRoot)
    {
        return;
    }
    printStationDCBPreorder(stationDCBRoot->left);
    printf("station dcb variable: %d,    station Id: %s\n", stationDCBRoot->preorderIndex, stationDCBRoot->stationId);
    printStationDCBPreorder(stationDCBRoot->right);
}


SatDCB* createSatDCB(int satId, double dcb)
{
    SatDCB* satDCB = malloc(sizeof(SatDCB));
    memset(satDCB, 0, sizeof(SatDCB));
    satDCB->satId = satId;
    satDCB->dcb = dcb;
    return satDCB;
}

int insertSatDCB(int satId, double dcb, SatDCB** satDCBRoot)
{
    if(!(*satDCBRoot))
    {
        *satDCBRoot = createSatDCB(satId, dcb);
        return 1;
    }
    if (satId < (*satDCBRoot)->satId)
    {
        return insertSatDCB(satId, dcb, &((*satDCBRoot)->left));
    }
    else if (satId > (*satDCBRoot)->satId)
    {
        return insertSatDCB(satId, dcb, &((*satDCBRoot)->right));
    }
    return 0;
}

double* getSatDCB(int satId, SatDCB* satDCBRoot)
{
    if(satDCBRoot)
    {
        if(satDCBRoot->satId == satId)
        {
            return &(satDCBRoot->dcb);
        }
        else if(satDCBRoot->satId > satId)
        {
            return getSatDCB(satId, satDCBRoot->left);
        }
        else
        {
            return getSatDCB(satId, satDCBRoot->right);
        }
    }
    return 0;
}

void deleteSatDCBTree(SatDCB** satDCBRoot)
{
    if (*satDCBRoot)
    {
        deleteSatDCBTree(&((*satDCBRoot)->left));
        deleteSatDCBTree(&((*satDCBRoot)->right));
        free(*satDCBRoot);
        *satDCBRoot = 0;
    }
    return;
}

int doSatDCBPreorderIndexing(SatDCB* satDCBRoot, int lastIndex)
{
    if(!satDCBRoot)
    {
        return -1;
    }
    int newLastIndex = lastIndex;
    if(satDCBRoot->left)
    {
        newLastIndex = doSatDCBPreorderIndexing(satDCBRoot->left, newLastIndex);
    }
    satDCBRoot->preorderIndex = newLastIndex;
    newLastIndex++;
    if(satDCBRoot->right)
    {
        newLastIndex = doSatDCBPreorderIndexing(satDCBRoot->right, newLastIndex);
    }
    return newLastIndex;
}

int getSatDCBPreorderIndex(int satId, SatDCB* satDCBRoot)
{
    if(satDCBRoot)
    {
        if(satDCBRoot->satId == satId)
        {
            return satDCBRoot->preorderIndex;
        }
        else if(satDCBRoot->satId > satId)
        {
            return getSatDCBPreorderIndex(satId, satDCBRoot->left);
        }
        else
        {
            return getSatDCBPreorderIndex(satId, satDCBRoot->right);
        }
    }
    return -1;
}


CellParameter* createCellParameter(int cellId, double eDens)
{
    CellParameter* cellParam = malloc(sizeof(CellParameter));
    memset(cellParam, 0, sizeof(CellParameter));
    cellParam->cellId = cellId;
    cellParam->eDensity = eDens;
    return cellParam;
}

int insertCellParameter(int cellId, double eDens, CellParameter** cellParamRoot)
{
    if(!(*cellParamRoot))
    {
        *cellParamRoot = createCellParameter(cellId, eDens);
        return 1;
    }
    if (cellId < (*cellParamRoot)->cellId)
    {
        return insertCellParameter(cellId, eDens, &((*cellParamRoot)->left));
    }
    else if (cellId > (*cellParamRoot)->cellId)
    {
        return insertCellParameter(cellId, eDens, &((*cellParamRoot)->right));
    }
    return 0;
}

double* getCellParameter(int cellId, CellParameter* cellParamRoot)
{
    if(cellParamRoot)
    {
        if(cellParamRoot->cellId == cellId)
        {
            return &(cellParamRoot->eDensity);
        }
        else if(cellParamRoot->cellId > cellId)
        {
            return getCellParameter(cellId, cellParamRoot->left);
        }
        else
        {
            return getCellParameter(cellId, cellParamRoot->right);
        }
    }
    return 0;
}

void deleteCellParameter(CellParameter** cellParamRoot)
{
    if (*cellParamRoot)
    {
        deleteCellParameter(&((*cellParamRoot)->left));
        deleteCellParameter(&((*cellParamRoot)->right));
        free(*cellParamRoot);
        *cellParamRoot = 0;
    }
    return;
}

int doCellParameterPreorderIndexing(CellParameter* cellParamRoot, int lastIndex)
{
    if(!cellParamRoot)
    {
        return -1;
    }
    int newLastIndex = lastIndex;
    if(cellParamRoot->left)
    {
        newLastIndex = doCellParameterPreorderIndexing(cellParamRoot->left, newLastIndex);
    }
    cellParamRoot->preorderIndex = newLastIndex;
    newLastIndex++;
    if(cellParamRoot->right)
    {
        newLastIndex = doCellParameterPreorderIndexing(cellParamRoot->right, newLastIndex);
    }
    return newLastIndex;
}

int getCellParameterPreorderIndex(int cellId, CellParameter* cellParamRoot)
{
    if(cellParamRoot)
    {
        if(cellParamRoot->cellId == cellId)
        {
            return cellParamRoot->preorderIndex;
        }
        else if(cellParamRoot->cellId > cellId)
        {
            return getCellParameterPreorderIndex(cellId, cellParamRoot->left);
        }
        else
        {
            return getCellParameterPreorderIndex(cellId, cellParamRoot->right);
        }
    }
    return -1;
}

void printCellParameterPreorder(CellParameter* cellParamRoot)
{
    if(!cellParamRoot)
    {
        return;
    }
    printCellParameterPreorder(cellParamRoot->left);
    printf("cellvariable: %d,    cellId: %d\n", cellParamRoot->preorderIndex, cellParamRoot->cellId);
    printCellParameterPreorder(cellParamRoot->right);
}


IonoVariables* createIonoVariables(Measurement* measRoot, QuadraticGrid* grid)
{
    IonoVariables* retVal = malloc(sizeof(IonoVariables));
    memset(retVal, 0, sizeof(IonoVariables));
    int measurementCount = 0;
    while (measRoot)
    {
        if (insertStationDCB(measRoot->recId, 0, &(retVal->stationDCBRoot)))
        {
            retVal->stationCount++;
        }
        if (insertSatDCB(measRoot->satId, 0, &(retVal->satDCBRoot)))
        {
            retVal->satCount++;
        }
        LineSectorList* tmplsl = measRoot->lineSectors;
        while(tmplsl)
        {
            int cellId = getSingleCellIDByIndexes(tmplsl->layerId, tmplsl->lateralId, tmplsl->longitudinalId, grid);
            if(insertCellParameter(cellId, 0, &(retVal->cellParameters)))
            {
                retVal->cellCount++;
            }
            tmplsl = tmplsl->next;
        }
        measurementCount++;
        measRoot = measRoot->next;
    }
    retVal->corrCount = measurementCount;
    retVal->corrections = malloc(sizeof(double) * measurementCount);
    return retVal;
}

void deleteIonoVariables(IonoVariables** ionoVariables)
{
    if(*ionoVariables)
    {
        deleteStationDCBTree(&((*ionoVariables)->stationDCBRoot));
        deleteSatDCBTree(&((*ionoVariables)->satDCBRoot));
        deleteCellParameter(&((*ionoVariables)->cellParameters));
        if((*ionoVariables)->corrections)
        {
            free((*ionoVariables)->corrections);
            (*ionoVariables)->corrections = 0;
        }
        free(*ionoVariables);
        ionoVariables = 0;
    }
}