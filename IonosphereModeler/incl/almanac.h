#ifndef ALMANAC_H
#define ALMANAC_H

#include <geometryPrimitives.h>

extern const long unixUTCMinusGPS;

long utcToGPST(long utc);
long gpstToUTC(long gpst);

//result must be full zero array
void calculateGPSSatellitePositions(double t, char* almanacFile, Vector*** results);
double calculateTimeOfGPSWeek(long gpsTime);



#endif //ALMANAC_H