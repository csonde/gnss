#include <ionosphereGrid.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define _USE_MATH_DEFINES

const Spheroid WGS84_Spheroid = {{0,0,0},
                                 6378137,
                                 6356752.314245,
                                 0.0818191908429643};

const double ionosphereLowerBound =  100000;
const double ionosphereUpperBound =  1000000;

GeoCoord createGeoCoord(double latitude, double longitude, int mode)
{
    GeoCoord retVal = {0,0};
    if (mode != 2)
    {
        longitude = longitude / 180 * M_PI;
        latitude = latitude / 180 * M_PI;
    }
    if (latitude > M_PI/2)
    {
        latitude = M_PI/2;
    }
    if (latitude < -M_PI/2)
    {
        latitude = -M_PI/2;
    }
    longitude = remainder(longitude, M_PI*2);
    retVal.longitude = longitude;
    retVal.latitude = latitude;
    return retVal;
}

Tesseroid createGeoTesseroid(Spheroid* lowerBound, Spheroid* upperBound, GeoCoord* vertexes, int vertexCount)
{
    char funcName[] = "createGeoTesseroid()";
    if (vertexCount < 3)
    {
        printf("%s: Tesseroid is depraved to ellipsoid shell or even worse. Invalid model. Exiting.\n", funcName);
        exit(-1);
    }
    Tesseroid t;
    initTesseroid(&t);
    t.vertexCount = 2 * vertexCount;
    t.vertexes = malloc(sizeof(Vector) * t.vertexCount);
    t.planeCount = vertexCount;
    t.boundaryPlanes = malloc(sizeof(Plane) * t.planeCount);
    t.boundarySpheroids[0] = lowerBound;
    t.boundarySpheroids[1] = upperBound;
    Vector o = createVector(0, 0, 0);
    int i;
    for (i = 0; i < vertexCount; i++)
    {
        double hgRad = WGS84_Spheroid.a / sqrt(1 - pow(WGS84_Spheroid.e, 2) * pow(sin(vertexes[i].latitude), 2));

        double wgs_x = hgRad * cos(vertexes[i].latitude) * cos(vertexes[i].longitude);
        double wgs_y = hgRad * cos(vertexes[i].latitude) * sin(vertexes[i].longitude);
        double wgs_z = hgRad * (1 - pow(WGS84_Spheroid.e, 2)) * sin(vertexes[i].latitude);
        Vector wgsCoords = createVector(wgs_x, wgs_y, wgs_z);
        Line projectionLine = createLine(o, wgsCoords);
        Vector intersections[2];
        int intersectionNum = intersectLineSpheroid(projectionLine, *lowerBound, intersections);
        if (intersectionNum != 2)
        {
            printf("%s: Geocentric Line-sphere intersection gave less than 2 result. Bad geometry specified.\n", funcName);
            exit(-1);
        }
        double isRightResult0 = dotProduct(wgsCoords, intersections[0]);
        double isRightResult1 = dotProduct(wgsCoords, intersections[1]);
        if ((isRightResult0 < 0 && isRightResult1 < 0) || (isRightResult0 > 0 && isRightResult1 > 0))
        {
            printf("%s: Geocentric Line-sphere intersection results are on the same side. Bad geometry specified.\n", funcName);
            exit(-1);
        }
        if (isRightResult0 > 0)
        {
            t.vertexes[2 * i] = intersections[0];
        }
        else
        {
            t.vertexes[2 * i] = intersections[1];
        }

        intersectionNum = intersectLineSpheroid(projectionLine, *upperBound, intersections);
        if (intersectionNum != 2)
        {
            printf("%s: Geocentric Line-sphere intersection gave less than 2 result. Bad geometry specified.\n", funcName);
            exit(-1);
        }
        isRightResult0 = dotProduct(wgsCoords, intersections[0]);
        isRightResult1 = dotProduct(wgsCoords, intersections[1]);
        if ((isRightResult0 < 0 && isRightResult1 < 0) || (isRightResult0 > 0 && isRightResult1 > 0))
        {
            printf("%s: Geocentric Line-sphere intersection results are on the same side. Bad geometry specified.\n", funcName);
            exit(-1);
        }
        if (isRightResult0 > 0)
        {
            t.vertexes[2 * i + 1] = intersections[0];
        }
        else
        {
            t.vertexes[2 * i + 1] = intersections[1];
        }
    }
    for (i = 0; i < vertexCount; i++)
    {
        int j = i + 1;
        if (i == vertexCount - 1)
        {
            j = 0;
        }
        //on the side planes either all of the normal vectors are showing outwards
        Vector planeNormal = crossProduct(t.vertexes[2 * i], t.vertexes[2 * j]);
        t.boundaryPlanes[i] = createPlane(o, planeNormal);
    }
    return t;
}

void initQuadraticGridCell(QuadraticGridCell* cell)
{
    memset(cell, 0, sizeof(QuadraticGridCell));
}

void deleteQuadraticGridCell(QuadraticGridCell* cell)
{
    deleteTesseroid(&(cell->t));
}

void initQuadraticGrid(QuadraticGrid* grid)
{
    memset(grid, 0, sizeof(QuadraticGrid));
}

void deleteQuadraticGrid(QuadraticGrid* grid)
{
    if (grid->cells)
    {
        int i;
        for (i = 0; i < grid->layerNum; i++)
        {
            if (grid->cells[i])
            {
                int j;
                for (j = 0; j < grid->northNum + grid->southNum; j++)
                {
                    if (grid->cells[i][j])
                    {
                        int k;
                        for (k = 0; k < grid->eastNum + grid->westNum; k++)
                        {
                            deleteQuadraticGridCell(&(grid->cells[i][j][k]));
                        }
                        free(grid->cells[i][j]);
                    }
                }
                free(grid->cells[i]);
            }
        }
        free(grid->cells);
    }
    if (grid->boundarySpheroids)
    {
        free (grid->boundarySpheroids);
    }
}

QuadraticGrid createQuadraticGrid(GeoCoord center, double latitudeUnit, double longitudeUnit, int layerNum, int cellNumLimitLat, int cellNumLimitLong)
{
    if (latitudeUnit < epsilon || longitudeUnit < epsilon)
    {
        printf("Too small latitude or longitude unit specified for model, minimal value is %lf degrees\n" \
              "latitude unit: %lf,    longitude unit: %lf\n", epsilon, latitudeUnit, longitudeUnit);
              exit(-1);
    }
    QuadraticGrid grid;
    initQuadraticGrid(&grid);
    grid.center = center;
    grid.layerNum = layerNum;
    latitudeUnit = latitudeUnit / 180 * M_PI;
    longitudeUnit = longitudeUnit / 180 * M_PI;
    grid.latitudeUnit = latitudeUnit;
    grid.longitudeUnit = longitudeUnit;
    grid.northNum = cellNumLimitLat;
    grid.southNum = cellNumLimitLat;
    grid.eastNum = cellNumLimitLong;
    grid.westNum = cellNumLimitLong;
    double latitudeRemainder = 0;
    double longitudeRemainder = 0;
    if (center.latitude - latitudeUnit * cellNumLimitLat < -M_PI/2)
    {
        long quot = (M_PI/2 + center.latitude) / latitudeUnit;
        latitudeRemainder = M_PI/2 + center.latitude - latitudeUnit * (double)quot;
        if (latitudeRemainder < 0)
        {
            latitudeRemainder += latitudeUnit;
        }
        grid.southNum = quot;
        if (latitudeRemainder > 0)
        {
            grid.southNum++;
        }
    }
    if (center.latitude + latitudeUnit * cellNumLimitLat > M_PI/2)
    {
        long quot = (M_PI/2 - center.latitude) / latitudeUnit;
        latitudeRemainder = M_PI/2 - center.latitude - latitudeUnit * (double)quot;
        if (latitudeRemainder < 0)
        {
            latitudeRemainder += latitudeUnit;
        }
        grid.northNum = quot;
        if (latitudeRemainder > 0)
        {
            grid.northNum++;
        }
    }
    if (longitudeUnit * cellNumLimitLong > M_PI)
    {
        long quot = M_PI / longitudeUnit;
        longitudeRemainder = M_PI - longitudeUnit * (double)quot;
        if (longitudeRemainder < 0)
        {
            longitudeRemainder += longitudeUnit;
        }
        grid.eastNum = quot;
        grid.westNum = quot;
        if (longitudeRemainder > 0)
        {
            grid.eastNum++;
            grid.westNum++;
        }
    }

    double layerWidth = (1000000 - 100000) / layerNum;
    grid.boundarySpheroids = malloc(sizeof(Spheroid) * (layerNum + 1));
    Vector o = createVector(0,0,0);
    int i;
    for (i = 0; i < layerNum + 1; i++)
    {
        grid.boundarySpheroids[i] = createSpheroid(o, WGS84_Spheroid.a + ionosphereLowerBound + i * layerWidth, WGS84_Spheroid.e, 2);
    }
    grid.cells = malloc(sizeof(QuadraticGridCell**) * layerNum);
    for (i = 0; i < layerNum; i++)
    {
        grid.cells[i] = malloc(sizeof(QuadraticGridCell*) * (grid.northNum + grid.southNum));
        int j;
        for (j = 0; j < grid.northNum + grid.southNum; j++)
        {
            grid.cells[i][j] = malloc(sizeof(QuadraticGridCell) * (grid.eastNum + grid.westNum));
            int k;
            for (k = 0; k < grid.eastNum + grid.westNum; k++)
            {
                GeoCoord vertexes[4];
                int vertexNum = 4;
                int northPole = 0;
                int southPole = 0;
                int currentVertex = 0;
                double nLat = grid.center.latitude + grid.latitudeUnit * (grid.northNum - j);
                double sLat = grid.center.latitude + grid.latitudeUnit * (grid.northNum - j - 1);
                double wLong = grid.center.longitude - grid.longitudeUnit * (grid.westNum - k);
                double eLong = grid.center.longitude - grid.longitudeUnit * (grid.westNum - k - 1);
                if (sLat < -M_PI/2)
                {
                    sLat = -M_PI/2;
                    vertexNum--;
                    southPole = 1;
                }
                if (nLat > M_PI/2)
                {
                    nLat = M_PI/2;
                    vertexNum--;
                    northPole = 1;
                }
                if (eLong > grid.center.longitude + M_PI)
                {
                    eLong = grid.center.longitude + M_PI;
                }
                if (wLong < grid.center.longitude - M_PI)
                {
                    wLong = grid.center.longitude - M_PI;
                }
                vertexes[0] = createGeoCoord(nLat, wLong, 2);
                currentVertex++;
                if (!northPole)
                {
                    vertexes[1] = createGeoCoord(nLat, eLong, 2);
                    currentVertex++;
                }
                vertexes[currentVertex] = createGeoCoord(sLat, eLong, 2);
                currentVertex++;
                if (!southPole)
                {
                    vertexes[currentVertex] = createGeoCoord(sLat, wLong, 2);
                }

                initQuadraticGridCell(&(grid.cells[i][j][k]));

                grid.cells[i][j][k].t = createGeoTesseroid(&(grid.boundarySpheroids[i]),
                                                           &(grid.boundarySpheroids[i+1]),
                                                           vertexes,
                                                           vertexNum);
                grid.cells[i][j][k].layerId = i;
                grid.cells[i][j][k].longitudinalId = k;
                grid.cells[i][j][k].lateralId = j;
            }
        }
    }
    for (i = 0; i < layerNum; i++)
    {
        int j;
        for (j = 0; j < grid.northNum + grid.southNum; j++)
        {
            int k;
            for (k = 0; k < grid.eastNum + grid.westNum; k++)
            {
                //lower neighbor
                if (i != 0)
                {
                    grid.cells[i][j][k].neighbors[0] = &(grid.cells[i-1][j][k]);
                }
                //upper neighbor
                if (i != layerNum - 1)
                {
                    grid.cells[i][j][k].neighbors[1] = &(grid.cells[i+1][j][k]);
                }
                //northern neighbor
                if (j != 0)
                {
                     grid.cells[i][j][k].neighbors[2] = &(grid.cells[i][j-1][k]);
                }
                //southern neighbor
                if (j != (grid.northNum + grid.southNum) - 1)
                {
                     grid.cells[i][j][k].neighbors[4] = &(grid.cells[i][j+1][k]);
                }
                //western neighbor
                if (k != 0)
                {
                     grid.cells[i][j][k].neighbors[5] = &(grid.cells[i][j][k-1]);
                }
                //if grid is reaching across the globe
                else if (grid.westNum * grid.longitudeUnit >= M_PI)
                {
                    grid.cells[i][j][k].neighbors[5] = &(grid.cells[i][j][grid.eastNum + grid.westNum - 1]);
                }
                //estern neighbor
                if (k != (grid.eastNum + grid.westNum) - 1)
                {
                     grid.cells[i][j][k].neighbors[3] = &(grid.cells[i][j][k+1]);
                }
                //if grid is reaching across the globe
                else if (grid.eastNum * grid.longitudeUnit >= M_PI)
                {
                    grid.cells[i][j][k].neighbors[3] = &(grid.cells[i][j][0]);
                }
            }
        }
    }
    return grid;
}

int eastNum;
    int westNum;
    int northNum;
    int southNum;
    int layerNum;

int getSingleCellIDByIndexes(int layerIndex, int nsIndex, int weIndex, QuadraticGrid* grid)
{
    assert(layerIndex < grid->layerNum &&
           nsIndex < (grid->northNum + grid->southNum) &&
           weIndex < (grid->westNum + grid->eastNum));
    return layerIndex * (grid->eastNum + grid->westNum) * (grid->northNum + grid->southNum) +
           nsIndex * (grid->eastNum + grid->westNum) +
           weIndex;
}


void initLineSectorList(LineSectorList* lsl)
{
    memset(lsl, 0, sizeof(LineSectorList));
}

void deleteLineSectorList(LineSectorList** lsl)
{
    if(!(*lsl))
    {
        return;
    }
    deleteLineSectorList(&((*lsl)->next));
    free(*lsl);
    *lsl = 0;
}

//  -1: outer point
//   0: inner point
//   a: point is on boundary of index a-1
//  ab: point is on boundary of index a-1 and index b-1
// abc: point is on boundary of index a-1 and b-1 and c-1
// more values are not permitted
int innerPointChecker(Vector v, QuadraticGridCell* currentCell)
{
    int retVal = 0;
    int boundCount = currentCell->t.planeCount + 2;
    int i;
    for (i = 0; i < 2; i++)
    {
        double subst = checkVectorAgainstSpheroid(v, *(currentCell->t.boundarySpheroids[i]));
        if(subst < epsilon && subst > -epsilon)
        {
            retVal = retVal * 10 + i + 1;
        }
        else if ((i == 0 && subst < 0) || (i == 1 && subst > 0))
        {
            return -1;
        }
    }
    for (i = 0; i < boundCount - 2; i++)
    {
        double subst = checkVectorAgainstPlane(v, currentCell->t.boundaryPlanes[i]);
        //this is not elegant, just hacking
        //this should be rather handled when specifying neighbors
        //requires smaller redesign
        int skipNorthbound = 0;
        int skipSouthbound = 0;
        if (boundCount == 5)
        {
            if (currentCell->t.vertexes[0].x < epsilon || currentCell->t.vertexes[0].x > -epsilon)
            {
                skipNorthbound = 1;
            }
            else if (currentCell->t.vertexes[4].x < epsilon || currentCell->t.vertexes[4].x > -epsilon)
            {
                skipSouthbound = 1;
            }
        }
        else if (boundCount == 4)
        {
            skipNorthbound = 1;
            skipSouthbound = 1;
        }
        if (subst < epsilon && subst > -epsilon)
        {
            if (boundCount == 5)
            {
                if(i < 2)
                {
                    retVal = retVal * 10 + i + 3 + skipNorthbound;
                }
                else
                {
                    retVal = retVal * 10 + i + 3 + skipNorthbound + skipSouthbound;
                }
            }
            else if (boundCount == 4)
            {
                if(i < 2)
                {
                    retVal = retVal * 10 + i + 3 + skipNorthbound;
                }
                else
                {
                    retVal = retVal * 10 + i + 3 + skipNorthbound + skipSouthbound;
                }
            }
            else
            {
                retVal = retVal * 10 + i + 3;
            }
        }
        else if(subst > 0)
        {
            return -1;
        }
    }
    return retVal;
}

LineSectorList* getLineSectorOfCell(Line l, QuadraticGridCell* currentCell, QuadraticGridCell** nextCell)
{
    Vector satPos = l.p1;
    if (l.p1.length < l.p2.length)
    {
        satPos = l.p2;
    }
    LineSectorList* lsl = malloc(sizeof(LineSectorList));
    initLineSectorList(lsl);
    int boundCount = currentCell->t.planeCount + 2;
    //indexing according to neighbors
    Vector** intersections = malloc(sizeof(Vector*) * boundCount);
    memset(intersections, 0, sizeof(Vector*) * boundCount);
    int* isInnerPoint = malloc(sizeof(int) * boundCount);
    memset(isInnerPoint, 0, sizeof(int) * boundCount);
    //calculate upper spheroid
    int i;
    for (i = 0; i < 2; i++)
    {
        Vector intersectResults[2];
        int resultNum = intersectLineSpheroid(l, *(currentCell->t.boundarySpheroids[i]), intersectResults);
        if (resultNum != 2)
        {
            printf("Satellite-receiver trajectory has less then 2 intersections with\n" \
                   "boundary spheroid, which should not be possible since receiver\n" \
                   "must be inside and satellite must be outside.\n");
            exit(-1);
        }
        intersections[i] = malloc(sizeof(Vector));
        if (subtractVector(intersectResults[0], satPos).length < subtractVector(intersectResults[1], satPos).length)
        {
            *intersections[i] = intersectResults[0];
        }
        else
        {
            *intersections[i] = intersectResults[1];
        }
    }
    for (i = 0; i < boundCount - 2; i++)
    {
        Vector intersectResult;
        int resultCode = intersectLinePlane(l, currentCell->t.boundaryPlanes[i], &intersectResult);
        if (resultCode == 0)
        {
            intersections[i+2] = malloc(sizeof(Vector));
            *intersections[i+2] = intersectResult;
        }
        else if (resultCode == 1)
        {
            lsl->layerId = currentCell->layerId;
            lsl->lateralId = currentCell->lateralId;
            lsl->longitudinalId = currentCell->longitudinalId;
            lsl->cellIntersectionEntry = *intersections[0];
            lsl->cellIntersectionExit = *intersections[1];
            lsl->length = subtractVector(*intersections[1], *intersections[0]).length;
            *nextCell = currentCell->neighbors[0];
            int j;
            for (j = 0; j < boundCount; j++)
            {
                if (intersections[j])
                {
                    free(intersections[j]);
                }
            }
            free(intersections);
            free(isInnerPoint);
            return lsl;
        }
    }
    for (i = 0; i < boundCount; i++)
    {
        isInnerPoint[i] = innerPointChecker(*intersections[i], currentCell);
    }
    Vector* differentPoints[6] = {0};
    int isDiffInnerPoint[6] = {0};
    int differentPointCount = 0;
    for (i = 0; i < boundCount; i++)
    {
        assert(isInnerPoint[i] != 0);
        if (isInnerPoint[i] == -1)
        {
            continue;
        }
        int storedDifferentPoint = 0;
        int j;
        for (j = 0; j < differentPointCount; j++)
        {
            Vector diff = subtractVector(*(intersections[i]), *differentPoints[j]);
            if (diff.length < epsilon)
            {
                storedDifferentPoint = 1;
                break;
            }
        }
        if (!storedDifferentPoint)
        {
            differentPoints[differentPointCount] = intersections[i];
            isDiffInnerPoint[differentPointCount] = isInnerPoint[i];
            differentPointCount++;
        }
    }
    assert(differentPointCount == 2);
    int entryIndex;
    int exitIndex;
    if ((*differentPoints[0]).length > (*differentPoints[1]).length)
    {
        entryIndex = 0;
        exitIndex = 1;
    }
    else
    {
        entryIndex = 1;
        exitIndex = 0;
    }
    lsl->layerId = currentCell->layerId;
    lsl->lateralId = currentCell->lateralId;
    lsl->longitudinalId = currentCell->longitudinalId;
    lsl->cellIntersectionEntry = *differentPoints[entryIndex];
    lsl->cellIntersectionExit = *differentPoints[exitIndex];
    lsl->length = subtractVector(*differentPoints[entryIndex], *differentPoints[exitIndex]).length;
    int exitNeighbor = isDiffInnerPoint[exitIndex];
    assert(exitNeighbor >= 0);
    int neighborIds[3] = {0};
    for (i = 0; i < 3; i++)
    {
        neighborIds[i] = (exitNeighbor % 10);
        exitNeighbor /= 10;
    }
    assert(exitNeighbor == 0);
    int isLowerBoundExit = 0;
    for(i = 0; i < 3; i++)
    {
        if(neighborIds[i] == 1)
        {
            isLowerBoundExit = 1;
        }
    }
    *nextCell = currentCell;
    i = 0;
    for(i = 0; i < 3 && neighborIds[i] != 0; i++)
    {
        *nextCell = (*nextCell)->neighbors[neighborIds[i] - 1];
        //line exits the model before reaches the lower bound
        if(!(*nextCell != 0 || (currentCell->layerId == 0 && isLowerBoundExit == 1)))
        {
            deleteLineSectorList(&lsl);
            return 0;
        }
        if(*nextCell == 0)
        {
            break;
        }
    }

    for (i = 0; i < boundCount; i++)
    {
        if (intersections[i])
        {
            free(intersections[i]);
        }
    }
    free(intersections);
    free(isInnerPoint);
    return lsl;
}

LineSectorList* getLineSectorsFromModel(Vector satPos, Vector recPos, QuadraticGrid* grid)
{
    //search for intersection on most upper spheroid
    Line l = createLine(satPos, recPos);
    Vector intersectResults[2];
    int resultNum = intersectLineSpheroid(l, grid->boundarySpheroids[grid->layerNum], intersectResults);
    if (resultNum != 2)
    {
        printf("Satellite-receiver trajectory has less then 2 intersections with\n" \
               "boundary spheroid, which should not be possible since receiver\n" \
               "must be inside and satellite must be outside.\n");
        exit(-1);
    }
    LineSectorList* result = 0;
    LineSectorList* currentLineSector = 0;
    Vector entryCoord;
    Vector exitCoord;
    int layerId;
    int latId;
    int longId;
    //get the upper intersection
    if (subtractVector(intersectResults[0], satPos).length < subtractVector(intersectResults[1], satPos).length)
    {
        entryCoord = intersectResults[0];
    }
    else
    {
        entryCoord = intersectResults[1];
    }
    resultNum = intersectLineSpheroid(l, grid->boundarySpheroids[0], intersectResults);
    assert(resultNum == 2);
    if (subtractVector(intersectResults[0], satPos).length < subtractVector(intersectResults[1], satPos).length)
    {
        exitCoord = intersectResults[0];
    }
    else
    {
        exitCoord = intersectResults[1];
    }
    double totalLength = subtractVector(entryCoord, exitCoord).length;

    int i;
    int mismatchFound = 0;
    //check the cells to find out which one contains the intersection
    for (i = 0; i < grid->northNum + grid->southNum; i++)
    {
        int j;
        for (j = 0; j < grid->eastNum + grid->westNum; j++)
        {
            mismatchFound = 0;
            int k;
            for (k = 0; k < grid->cells[grid->layerNum-1][i][j].t.planeCount; k++)
            {
                double subst = (checkVectorAgainstPlane(entryCoord, grid->cells[grid->layerNum-1][i][j].t.boundaryPlanes[k]));
                if (subst >= epsilon)
                {
                    mismatchFound = 1;
                    break;
                }
                else if (subst > -epsilon)
                {
                    double direction = dotProduct(l.orientation, grid->cells[grid->layerNum-1][i][j].t.boundaryPlanes[k].normal);
                    if (direction >= epsilon)
                    {
                        mismatchFound = 1;
                        break;
                    }
                }
            }
            if (!mismatchFound)
            {
                layerId = grid->layerNum - 1;
                latId = i;
                longId = j;
                break;
            }
        }
        if (!mismatchFound)
        {
            break;
        }
    }
    if (mismatchFound)
    {
        return 0;
    }
    //start calculating the line sectors
    else
    {
        i = 1;
        QuadraticGridCell* currentCell = &(grid->cells[layerId][latId][longId]);
        QuadraticGridCell* nextCell = 0;
        do
        {
            if (!nextCell)
            {
                result = currentLineSector = getLineSectorOfCell(l, currentCell, &nextCell);
            }
            else
            {
                i++;
                LineSectorList* tmpLineSector = currentLineSector;
                currentLineSector = getLineSectorOfCell(l, currentCell, &nextCell);
                tmpLineSector->next = currentLineSector;
            }
            if(!currentLineSector)
            {
                if(result)
                {
                    deleteLineSectorList(&result);
                }
                return 0;
            }
            currentCell = nextCell;
        }
        while(nextCell);
    }
    if(result)
    {
        double sum = 0;
        LineSectorList* tmpLineSector = result;
        while(tmpLineSector)
        {
            sum += tmpLineSector->length;
            tmpLineSector = tmpLineSector->next;
        }
        assert(totalLength - sum < 1 && totalLength - sum > -1);
    }
    return result;
}
