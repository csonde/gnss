#ifndef OBSERVATION_PARSER_H
#define OBSERVATION_PARSER_H

#include "rinexCommon.h"

typedef enum
{
    G,
    R,
    S,
    E,
    M
}SatelliteSystem;


typedef enum
{
    SINGLE_FREQ,
    FULL_CYCLE,
    HALF_CYCLE
}WaveLengthFactor;


typedef enum
{
    GPS_TIME,
    GLONASS_TIME,
    GALILEO_TIME
}TimeSystem;


typedef struct WaveLengthFactorRecord
{
    WaveLengthFactor L1;
    WaveLengthFactor L2;
    int satelliteNumber;
    int* satellites;
}WaveLengthFactorRecord;

void initWaveLengthFactorRecord(WaveLengthFactorRecord* waveLengthFactorRecord);
void deleteWaveLengthFactorRecord(WaveLengthFactorRecord* waveLengthFactorRecord);


typedef struct ObservationCount
{
    int prn;
    int* obsCount;
}ObservationCount;

void initObservationCount(ObservationCount* observationCount);
void deleteObservationCount(ObservationCount* observationCount);


typedef struct GNSSObservationHeader
{
    double rinex_version;
    FileType fileType;
    SatelliteSystem satelliteSystem;

    char creatingProgram[21];
    char creatingAgency[21];
    char creationDate[21];

    int commentLength;
    char* comment;

    char markerName[61];
    char markerNumber[21];

    char observerName[21];
    char agency[41];

    char receiverNumber[21];
    char receiverType[21];
    char receiverVersion[21];

    char antennaNumber[21];
    char antennaType[21];

    double approxMarkerX;
    double approxMarkerY;
    double approxMarkerZ;

    double antennaHeight;
    double antennaExcEast;
    double antennaExcNorth;

    WaveLengthFactor defaultWavelengthFactorL1;
    WaveLengthFactor defaultWavelengthFactorL2;
    int defaultWavelengthFactorSet;
    int waveLengthFactorRecordNumber;
    WaveLengthFactorRecord* nonDefaultWavelengthFactors;

    int obsTypeNumber;
    char *observationCodes;
    int *frequencyCodes;
    double obsInterval;
    struct tm firstObsTime;
    TimeSystem firstObsTimeSystem;
    struct tm lastObsTime;
    TimeSystem lastObsTimeSystem;

    int isClockOffsetApplied;

    int leapSeconds;

    int satelliteNumber;
    ObservationCount* obsCounts;

    struct GNSSObservationHeader* nextHeader;
}GNSSObservationHeader;

void initGNSSObservationHeader(GNSSObservationHeader* header);
void copyGNSSObservationHeader(GNSSObservationHeader** newHeader, GNSSObservationHeader* header, int allocNewMem);
GNSSObservationHeader* getGNSSObservationHeaderInPos(GNSSObservationHeader* headersHead, int pos);
void deleteGNSSObservationHeader(GNSSObservationHeader* header);
void printGNSSObservationHeader(GNSSObservationHeader* header);


typedef struct GNSSObservation
{
    PreciseTime epoch;
    GNSSObservationHeader* header;
    double* observations;
    int* lli;
    int* signalStrength;

    int eventFlagNum;
    int* eventFlags;
}GNSSObservation;

void initGNSSObservation(GNSSObservation* observation);
void copyGNSSObservation(GNSSObservation** dest, GNSSObservation* source, int allocNewMem);
void deleteGNSSObservation(GNSSObservation* observation);
void printGNSSObservations(GNSSObservation** observations, int satNum, int* totalObservationCount);


/*
 * observations must be a satTypeNum * satPerType size array of null pointers
 * headers must be the address of a null pointer
 *
 * return value is 1 if parsing was successful and 0 if it was not.
 */
int parseGNSSObservationFile(GNSSObservation** observations, int* totalObservationCount, GNSSObservationHeader** headers, char* obsFilePath, int* headerCount);

#endif //OBSERVATION_PARSER_H