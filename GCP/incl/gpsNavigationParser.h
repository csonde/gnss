#ifndef GPS_NAVIGATION_PARSER_H
#define GPS_NAVIGATION_PARSER_H

#include "rinexCommon.h"

typedef struct GPSNavigationHeader
{
    double rinex_version;
    FileType fileType;

    char creatingProgram[21];
    char creatingAgency[21];
    char creationDate[21];

    int commentLength;
    char* comment;

    double ionosphereA0;
    double ionosphereA1;
    double ionosphereA2;
    double ionosphereA3;

    double ionosphereB0;
    double ionosphereB1;
    double ionosphereB2;
    double ionosphereB3;

    double almanachA0;
    double almanachA1;
    long referenceTime;
    long referenceWeek;

    int leapSeconds;
}GPSNavigationHeader;

void initGPSNavigationHeader(GPSNavigationHeader* header);
void copyGPSNavigationHeader(GPSNavigationHeader** newHeader, GPSNavigationHeader* header, int allocNewMem);
void deleteGPSNavigationHeader(GPSNavigationHeader* header);
void printGPSNavigationHeader(GPSNavigationHeader* header);

typedef struct GPSNavigationData
{
    GPSNavigationHeader* header;

    //line 1
    //int prn; //will be stored in the index
    PreciseTime epoch;

    double svClockBias;
    double svClockDrift;
    double svClockDriftRate;

    //line 2
    double ephimeris;
    double crs;
    double deltaN;
    double m0;

    //line 3
    double cuc;
    double eccentricity;
    double cus;
    double sqrtA;

    //line 4
    double toe;
    double cic;
    double OMEGA;
    double cis;

    //line 5
    double i0;
    double crc;
    double omegaLowerCase;
    double OMEGADOT;

    //line 6
    double iDot;
    double L2Codes;
    double gpsWeek;
    double L2PDataFlag;

    //line 7
    double svAccuracy;
    double svHealth;
    double tgd;
    double iodc;

    //line 8
    double transmissionTime;
    double fitInterval;
}GPSNavigationData;

void initGPSNavigationData(GPSNavigationData* navData);
void copyGPSNavigationData(GPSNavigationData** dest, GPSNavigationData* source, int allocNewMem);
//this one is not necessary because there are no pointers in the struct
//void deleteGPSNavigationData(GPSNavigationData* observation);
void printGPSNavigationDatas(GPSNavigationData** navDatas, int satNum, int* totalNavDataCount);

int parseGPSNavigationFile(GPSNavigationData** navDatas, int* totalNavDataCount, GPSNavigationHeader** header, char* navFilePath);

#endif //GPS_NAVIGATION_PARSER_H