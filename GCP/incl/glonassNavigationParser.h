#ifndef GLONASS_NAVIGATION_PARSER_H
#define GLONASS_NAVIGATION_PARSER_H

#include "rinexCommon.h"

typedef struct GlonassNavigationHeader
{
    double rinex_version;
    FileType fileType;

    char creatingProgram[21];
    char creatingAgency[21];
    char creationDate[21];

    int commentLength;
    char* comment;

    int refYear;
    int refMonth;
    int refDay;
    double timeCorrection;

    int leapSeconds;
}GlonassNavigationHeader;

void initGlonassNavigationHeader(GlonassNavigationHeader* header);
void copyGlonassNavigationHeader(GlonassNavigationHeader** newHeader, GlonassNavigationHeader* header, int allocNewMem);
void deleteGlonassNavigationHeader(GlonassNavigationHeader* header);
void printGlonassNavigationHeader(GlonassNavigationHeader* header);

typedef struct GlonassNavigationData
{
    GlonassNavigationHeader* header;

    //line 1
    //int prn; //will be stored in the index
    PreciseTime epoch;

    double svClockBias;
    double svRelFrqBias;
    double msgFrameTime;

    //line 2
    double satPositionX;
    double satVelocityX;
    double satAccelerationX;
    double health;

    //line 3
    double satPositionY;
    double satVelocityY;
    double satAccelerationY;
    double frequencyNumber;

    //line 4
    double satPositionZ;
    double satVelocityZ;
    double satAccelerationZ;
    double operInfAge;
}GlonassNavigationData;

void initGlonassNavigationData(GlonassNavigationData* navData);
void copyGlonassNavigationData(GlonassNavigationData** dest, GlonassNavigationData* source, int allocNewMem);
//this one is not necessary because there are no pointers in the struct
//void deleteGlonassNavigationData(GlonassNavigationData* observation);
void printGlonassNavigationDatas(GlonassNavigationData** navDatas, int satNum, int* totalNavDataCount);

int parseGlonassNavigationFile(GlonassNavigationData** navDatas, int* totalNavDataCount, GlonassNavigationHeader** header, char* navFilePath);

#endif //GLONASS_NAVIGATION_PARSER_H