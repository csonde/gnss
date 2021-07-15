#include <stdio.h>
#include "geometryPrimitives.h"
#include "ionosphereGrid.h"
#include "math.h"

#define _USE_MATH_DEFINES

int main()
{
    Vector v1 = createVector(-5,0,0);
    Vector v2 = createVector(5,0,0);
    printf("V1, V2 length: %lf, %lf\n", v1.length, v2.length);
    Vector v3 = crossProduct(v1, v2);
    printf("V3(crossprod): %lf, %lf, %lf,    %lf\n", v3.x, v3.y, v3.z, v3.length);
    Vector v4 = createVector(0,0,0);
    Spheroid sph = createSpheroid(v4, 3, 1, 1);
    printf("SPH ecc: %lf\n", sph.e);
    Line l = createLine(v1, v2);
    printf("LINE ori: %lf, %lf, %lf\n", l.orientation.x, l.orientation.y, l.orientation.z);
    Plane p = createPlane(v4, v2);
    printf("PLANE coeff: %lf\n", p.d);
    Vector result[2];
    intersectLineSpheroid(l, sph, result);
    printf("INTERSECTION 1: %lf, %lf, %lf\n", result[0].x, result[0].y, result[0].z);
    printf("INTERSECTION 2: %lf, %lf, %lf\n", result[1].x, result[1].y, result[1].z);
    intersectLinePlane(l, p, result);
    printf("INTERSECTION 3: %lf, %lf, %lf\n", result[0].x, result[0].y, result[0].z);

    GeoCoord center = createGeoCoord(45, 0, 1);
    QuadraticGrid grid = createQuadraticGrid(center, 11, 11, 10, 10, 20);
    int i = 0;
    for (i = 0; i < grid.layerNum; i++)
    {
        printf("Center: %lf, %lf\n\n", grid.center.latitude * 180 / M_PI, grid.center.longitude * 180 / M_PI);
        printf("Layer %d\n", i);
        int j;
        for (j = 0; j < grid.northNum + grid.southNum; j++)
        {
            printf("    lateral %d\n", j);
            int k;
            for (k = 0; k < grid.eastNum + grid.westNum; k++)
            {
                printf("        longitudinal %d\n", k);
                int l;
                printf("            vertexes:\n");
                for (l = 0; l < grid.cells[i][j][k].t.vertexCount/2; l++)
                {
                    printf("                %lf, %lf, %lf,    length: %lf\n", grid.cells[i][j][k].t.vertexes[2*l].x,
                                                                              grid.cells[i][j][k].t.vertexes[2*l].y,
                                                                              grid.cells[i][j][k].t.vertexes[2*l].z,
                                                                              grid.cells[i][j][k].t.vertexes[2*l].length);
                }
                for (l = 0; l < grid.cells[i][j][k].t.vertexCount/2; l++)
                {
                    printf("                %lf, %lf, %lf,    length: %lf\n", grid.cells[i][j][k].t.vertexes[2*l+1].x,
                                                                             grid.cells[i][j][k].t.vertexes[2*l+1].y,
                                                                             grid.cells[i][j][k].t.vertexes[2*l+1].z,
                                                                             grid.cells[i][j][k].t.vertexes[2*l+1].length);
                }
                printf("            boundary spheroids:\n");
                printf("                center: %lf, %lf, %lf,   a: %lf,    b: %lf,    e: %lf\n", grid.cells[i][j][k].t.boundarySpheroids[0]->center.x,
                                                                                                  grid.cells[i][j][k].t.boundarySpheroids[0]->center.y,
                                                                                                  grid.cells[i][j][k].t.boundarySpheroids[0]->center.z,
                                                                                                  grid.cells[i][j][k].t.boundarySpheroids[0]->a,
                                                                                                  grid.cells[i][j][k].t.boundarySpheroids[0]->b,
                                                                                                  grid.cells[i][j][k].t.boundarySpheroids[0]->e);
                printf("                center: %lf, %lf, %lf,   a: %lf,    b: %lf,    e: %lf\n", grid.cells[i][j][k].t.boundarySpheroids[1]->center.x,
                                                                                                  grid.cells[i][j][k].t.boundarySpheroids[1]->center.y,
                                                                                                  grid.cells[i][j][k].t.boundarySpheroids[1]->center.z,
                                                                                                  grid.cells[i][j][k].t.boundarySpheroids[1]->a,
                                                                                                  grid.cells[i][j][k].t.boundarySpheroids[1]->b,
                                                                                                  grid.cells[i][j][k].t.boundarySpheroids[1]->e);
                printf("            boundary planes:\n");
                for (l = 0; l < grid.cells[i][j][k].t.planeCount; l++)
                {
                    printf("                point: %lf, %lf, %lf,   normal: %lf, %lf, %lf\n", grid.cells[i][j][k].t.boundaryPlanes[l].point.x,
                                                                                              grid.cells[i][j][k].t.boundaryPlanes[l].point.y,
                                                                                              grid.cells[i][j][k].t.boundaryPlanes[l].point.z,
                                                                                              grid.cells[i][j][k].t.boundaryPlanes[l].normal.x,
                                                                                              grid.cells[i][j][k].t.boundaryPlanes[l].normal.y,
                                                                                              grid.cells[i][j][k].t.boundaryPlanes[l].normal.z);
                }
            }
        }
    }
    Vector satPos = createVector(13860000, 13860000, 100000);
    Vector recPos = createVector(4510000, 4510000, 100000);
    LineSectorList* lsl = getLineSectorsFromModel(satPos, recPos, &grid);
    LineSectorList* tmp = lsl;
    while(tmp)
    {
        printf("Linesector %d:    cell: (%d, %d, %d)    length: %lf\n", i, tmp->layerId, tmp->lateralId, tmp->longitudinalId, tmp->length);
        tmp = tmp->next;
        i++;
    }
    return 1;
}