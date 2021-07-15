#ifndef METEOROLOGICAL_PARSER_H
#define METEOROLOGICAL_PARSER_H

#include "rinexCommon.h"

typedef struct MeteorologicalSensorType
{
    char model[21];
    char type[21];
    double accuracy;
    char obsType[3];
}MeteorologicalSensorType;

void initMeteorologicalSensorType(MeteorologicalSensorType* sensorType);


typedef struct MeteorologicalSensorPos
{
    double posX;
    double posY;
    double posZ;
    double ellipsH;
    char obsType[3];
}MeteorologicalSensorPos;

void initMeteorologicalSensorPos(MeteorologicalSensorPos* sensorPos);


typedef struct MeteorologicalHeader
{
    double rinex_version;
    FileType fileType;

    char creatingProgram[21];
    char creatingAgency[21];
    char creationDate[21];

    int commentLength;
    char* comment;

    char markerName[61];
    char markerNumber[21];

    int obsTypeNumber;
    char (*obsTypes)[3];

    MeteorologicalSensorType* sensorTypes;

    int sensorPosNum;
    MeteorologicalSensorPos* sensorPositions;
}MeteorologicalHeader;

void initMeteorologicalHeader(MeteorologicalHeader* header);
void copyMeteorologicalHeader(MeteorologicalHeader** newHeader, MeteorologicalHeader* header, int allocNewMem);
void deleteMeteorologicalHeader(MeteorologicalHeader* header);
void printMeteorologicalHeader(MeteorologicalHeader* header);


typedef struct MeteorologicalData
{
    PreciseTime epoch;
    MeteorologicalHeader* header;
    double* observations;
}MeteorologicalData;

void initMeteorologicalData(MeteorologicalData* observation);
void copyMeteorologicalData(MeteorologicalData** dest, MeteorologicalData* source, int allocNewMem);
void deleteMeteorologicalData(MeteorologicalData* observation);
void printMeteorologicalData(MeteorologicalData** observations, int satNum, int* totalObservationCount);


/*
 * observations must be a satTypeNum * satPerType size array of null pointers
 * headers must be the address of a null pointer
 *
 * return value is 1 if parsing was successful and 0 if it was not.
 */
int parseMeteorologicalFile(MeteorologicalData** observations, int* totalObservationCount, MeteorologicalHeader** header, char* metFilePath);

#endif //METEOROLOGICAL_PARSER_H