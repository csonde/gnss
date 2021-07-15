#include "observationParser.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>



void initWaveLengthFactorRecord(WaveLengthFactorRecord* waveLengthFactorRecord)
{
    memset(waveLengthFactorRecord, 0, sizeof(WaveLengthFactorRecord));
}

void deleteWaveLengthFactorRecord(WaveLengthFactorRecord* waveLengthFactorRecord)
{
    if (waveLengthFactorRecord->satellites!=0)
    {
        free(waveLengthFactorRecord->satellites);
    }
}



void initObservationCount(ObservationCount* observationCount)
{
    memset(observationCount, 0, sizeof(ObservationCount));
}

void deleteObservationCount(ObservationCount* observationCount)
{
    if (observationCount->obsCount)
    {
        free(observationCount->obsCount);
    }
}



void initGNSSObservationHeader(GNSSObservationHeader* header)
{
    memset(header, 0, sizeof(GNSSObservationHeader));
}

void copyGNSSObservationHeader(GNSSObservationHeader** dest, GNSSObservationHeader* header, int allocNewMem)
{
    GNSSObservationHeader* newHeader = *dest;
    if (allocNewMem || !newHeader)
    {
        newHeader = malloc(sizeof(GNSSObservationHeader));
    }
    memcpy(newHeader, header, sizeof(GNSSObservationHeader));
    if(header->comment)
    {
        newHeader->comment = malloc(sizeof(char) * header->commentLength);
        memcpy(newHeader->comment, header->comment, sizeof(char) * header->commentLength);
    }
    if (header->nonDefaultWavelengthFactors)
    {
        newHeader->nonDefaultWavelengthFactors = malloc(sizeof(WaveLengthFactorRecord) * header->waveLengthFactorRecordNumber);
        memcpy(newHeader->nonDefaultWavelengthFactors, header->nonDefaultWavelengthFactors, sizeof(WaveLengthFactorRecord) * header->waveLengthFactorRecordNumber);
        int i;
        for (i=0; i < header->waveLengthFactorRecordNumber; i++)
        {
            if (header->nonDefaultWavelengthFactors[i].satellites)
            {
                newHeader->nonDefaultWavelengthFactors[i].satellites = malloc(sizeof(int) * header->nonDefaultWavelengthFactors[i].satelliteNumber);
                memcpy(newHeader->nonDefaultWavelengthFactors[i].satellites, header->nonDefaultWavelengthFactors[i].satellites, sizeof(int) * header->nonDefaultWavelengthFactors[i].satelliteNumber);
            }
        }
    }
    if(header->obsTypeNumber)
    {
        newHeader->observationCodes = malloc(sizeof(char) * header->obsTypeNumber);
        memcpy(newHeader->observationCodes, header->observationCodes, sizeof(char) * header->obsTypeNumber);
        newHeader->frequencyCodes = malloc(sizeof(int) * header->obsTypeNumber);
        memcpy(newHeader->frequencyCodes, header->frequencyCodes, sizeof(int) * header->obsTypeNumber);
    }
    if(header->obsCounts)
    {
        newHeader->obsCounts = malloc(sizeof(ObservationCount) * header->satelliteNumber);
        memcpy(newHeader->obsCounts, header->obsCounts, sizeof(ObservationCount) * header->satelliteNumber);
        int i;
        for (i = 0; i < header->satelliteNumber; i++)
        {
            if(header->obsCounts[i].obsCount)
            {
                newHeader->obsCounts[i].obsCount = malloc(sizeof(int) * header->obsTypeNumber);
                memcpy(newHeader->obsCounts[i].obsCount, header->obsCounts[i].obsCount, sizeof(int) * header->obsTypeNumber);
            }
        }
    }
}

GNSSObservationHeader* getGNSSObservationHeaderInPos(GNSSObservationHeader* headersHead, int pos)
{
    char funcName[] = "getGNSSObservationHeaderInPos()";
    if (!headersHead)
    {
        printf("%s: null pointer passed as headersHead\n", funcName);
        return 0;
    }
    GNSSObservationHeader* retVal = headersHead;
    for (; pos > 0; pos--)
    {
        retVal = retVal->nextHeader;
        if(!retVal)
        {
            printf("%s: given position is over the header count\n", funcName);
            break;
        }
    }
    return retVal;
}


void deleteGNSSObservationHeader(GNSSObservationHeader* header)
{
    if (header->nextHeader)
    {
        deleteGNSSObservationHeader(header->nextHeader);
        free(header->nextHeader);
    }
    if (header->comment != 0)
    {
        free(header->comment);
    }
    if (header->nonDefaultWavelengthFactors != 0)
    {
        int i;
        for (i = 0; i < header->waveLengthFactorRecordNumber; i++)
        {
            deleteWaveLengthFactorRecord(&(header->nonDefaultWavelengthFactors[i]));
        }
        free(header->nonDefaultWavelengthFactors);
    }
    if (header->observationCodes != 0)
    {
        free(header->observationCodes);
    }
    if (header->frequencyCodes != 0)
    {
        free(header->frequencyCodes);
    }
    if (header->obsCounts!=0)
    {
        int i;
        for (i = 0; i < header->waveLengthFactorRecordNumber; i++)
        {
            deleteObservationCount(&(header->obsCounts[i]));
        }
        free(header->obsCounts);
    }
}


void printGNSSObservationHeader(GNSSObservationHeader* header)
{
    printf("Version: %lf,    file type: %d,    satellite system:%d\n", header->rinex_version, header->fileType, header->satelliteSystem);
    printf("Creating Program: %s,    Agency: %s,    Date: %s,    \n", header->creatingProgram, header->creatingAgency, header->creationDate);
    printf("Comments:\n");
    if (header->comment)
    {
        printf("%s", header->comment);
    }
    else
    {
        printf("\n");
    }
    printf("Marker name: %s,    number: %s\n", header->markerName, header->markerNumber);
    printf("Observer name: %s,    agency: %s\n", header->observerName, header->agency);
    printf("Receiver number: %s,    type: %s,    version: %s\n", header->receiverNumber, header->receiverType, header->receiverVersion);
    printf("Antenna number: %s,    type: %s\n", header->antennaNumber, header->antennaType);
    printf("Approx marker x: %lf,    y: %lf,    z: %lf\n", header->approxMarkerX, header->approxMarkerY, header->approxMarkerZ);
    printf("Antenna height: %lf,    exc east: %lf,    exc north: %lf\n", header->antennaHeight, header->antennaExcEast, header->antennaExcNorth);
    printf("Def. WL fac L1: %d,    L2: %d\n", header->defaultWavelengthFactorL1, header->defaultWavelengthFactorL2);
    if(header->waveLengthFactorRecordNumber)
    {
        int i;
        for(i = 0; i < header->waveLengthFactorRecordNumber; i++)
        {
            printf("WaveLengthfactor L1: %d,    L2: %d    for satellites:", header->nonDefaultWavelengthFactors[i].L1, header->nonDefaultWavelengthFactors[i].L2);
            int j;
            for(j = 0; j < header->nonDefaultWavelengthFactors[i].satelliteNumber; j++)
            {
                printf(" %d,", header->nonDefaultWavelengthFactors[i].satellites[j]);
            }
            printf("\n");
        }
    }
    else
    {
        printf("There are nor non default WL factors\n");
    }
    if(header->obsTypeNumber)
    {
        printf("Observation types:");
        int i;
        for(i = 0; i < header->obsTypeNumber; i++)
        {
            printf(" %c%d",  header->observationCodes[i], header->frequencyCodes[i]);
        }
        printf("\n");
    }
    else
    {
        printf("No observation types given\n");
    }
    printf("Observation interval: %lf\n", header->obsInterval);
    printf("First obs time: %d %d %d %d %d %d\n", header->firstObsTime.tm_year,
                                                header->firstObsTime.tm_mon,
                                                header->firstObsTime.tm_mday,
                                                header->firstObsTime.tm_hour,
                                                header->firstObsTime.tm_min,
                                                header->firstObsTime.tm_sec);
    printf("First obs time system: %d\n", header->firstObsTimeSystem);
    printf("Last obs time: %d %d %d %d %d %d\n", header->lastObsTime.tm_year,
                                                header->lastObsTime.tm_mon,
                                                header->lastObsTime.tm_mday,
                                                header->lastObsTime.tm_hour,
                                                header->lastObsTime.tm_min,
                                                header->lastObsTime.tm_sec);
    printf("Last obs time system: %d\n", header->lastObsTimeSystem);
    printf("Clock offset applied: %d\n", header->isClockOffsetApplied);
    printf("Leap seconds: %d\n", header-> leapSeconds);
}



void initGNSSObservation(GNSSObservation* observation)
{
    memset(observation, 0, sizeof(GNSSObservation));
}

void copyGNSSObservation(GNSSObservation** dst, GNSSObservation* source, int allocNewMem)
{
    GNSSObservation* dest = *dst;
    if(allocNewMem || !dest)
    {
        dest = malloc(sizeof(GNSSObservation));
    }
    memcpy(&(dest->epoch), &(source->epoch), sizeof(PreciseTime));
    dest->header = source->header;
    dest->eventFlagNum = source->eventFlagNum;
    int obsTypeNumber = source->header->obsTypeNumber;

    dest->observations = malloc(sizeof(double) * obsTypeNumber);
    memcpy(dest->observations, source->observations, sizeof(double) * obsTypeNumber);
    dest->lli = malloc(sizeof(int) * obsTypeNumber);
    memcpy(dest->lli, source->lli, sizeof(int) * obsTypeNumber);
    dest->signalStrength = malloc(sizeof(int) * obsTypeNumber);
    memcpy(dest->signalStrength, source->signalStrength, sizeof(int) * obsTypeNumber);
    dest->eventFlags = malloc(sizeof(int) * source->eventFlagNum);
    memcpy(dest->eventFlags,  source->eventFlags, sizeof(int) * source->eventFlagNum);
}

void deleteGNSSObservation(GNSSObservation* observation)
{
    if (observation->observations)
    {
        free(observation->observations);
    }
    if (observation->lli)
    {
        free(observation->lli);
    }
    if (observation->signalStrength)
    {
        free(observation->signalStrength);
    }
    if(observation->eventFlags)
    {
        free(observation->eventFlags);
    }
}

void printGNSSObservations(GNSSObservation** observations, int satNum, int* totalObservationCount)
{
    int i;
    for (i = 0; i < satNum; i++)
    {
        if (observations[i])
        {
            printf("Observations for satellite %d:\n", i);
            int j;
            for (j = 0; j < totalObservationCount[i]; j++)
            {
                printf("    epoch: %ld.%d,    header: %p\n    event flags (%d):", observations[i][j].epoch.seconds,
                                                                                  observations[i][j].epoch.nanos,
                                                                                  observations[i][j].header,
                                                                                  observations[i][j].eventFlagNum);
                int k;
                for (k = 0; k < observations[i][j].eventFlagNum; k++)
                {
                    printf(" %d", observations[i][j].eventFlags[k]);
                }
                printf("\n");
                for (k = 0; k < observations[i][j].header->obsTypeNumber; k++)
                {
                    printf("        %c%d: %lf,    %d,    %d\n", observations[i][j].header->observationCodes[k],
                                                               observations[i][j].header->frequencyCodes[k],
                                                               observations[i][j].observations[k],
                                                               observations[i][j].lli[k],
                                                               observations[i][j].signalStrength[k]);
                }
            }
        }
    }
}



/*
 *  return value :
 *  1: everything is fine, still in header
 *  0: everything is fine, header is over
 * -1: error occured
 */
int parseGNSSObservationHeader(char* line, GNSSObservationHeader* header, char* commentTime, char* obsFilePath)
{
    char funcName[] = "parseGNSSObservationHeader()";
    char* recordName = &(line[60]);

    if (strstr(recordName, "RINEX VERSION / TYPE"))
    {
        double tmp = 0;
        sscanf(line, "%lf", &tmp);
        if (tmp != rinexVersion)
        {
            printf("%s: bad rinex version, required: %lf, got: %lf\n    in file %s\n", funcName, rinexVersion, tmp, obsFilePath);
            return -1;
        }
        header->rinex_version = tmp;
        if (line[20] != 'O' && line[20] != 'o')
        {
            printf("%s: bad file type, required: 'O' or 'o', got: %c\n    in file %s\n", funcName, line[20], obsFilePath);
            return -1;
        }
        header->fileType = OBS_DATA;
        if (line[40] == 'G' || line[20] == 0)
        {
            header->satelliteSystem = G;
        }
        else if (line[40] == 'R')
        {
            header->satelliteSystem = R;
        }
        else if (line[40] == 'M')
        {
            header->satelliteSystem = M;
        }
        else
        {
            printf("%s: unsupported satellite system: %c\n    in file %s\n", funcName, line[40], obsFilePath);
            return -1;
        }
    }

    else if (strstr(recordName, "PGM / RUN BY / DATE"))
    {
        strncpy(header->creatingProgram, line, 20);
        header->creatingProgram[20] = 0;
        strncpy(header->creatingAgency, &(line[20]), 20);
        header->creatingAgency[20] = 0;
        strncpy(header->creationDate, &(line[40]), 20);
        header->creationDate[20] = 0;
    }

    else if (strstr(recordName, "COMMENT"))
    {
        char* tmp = malloc(sizeof(char) * (header->commentLength + 90 + 1));
        if (header->comment)
        {
            strncpy(tmp, header->comment, header->commentLength);
        }
        strncpy(&(tmp[header->commentLength]), commentTime, 29);
        strncpy(&(tmp[header->commentLength + 29]), line, 60);
        tmp[header->commentLength + 89] = '\n';
        tmp[header->commentLength + 90] = 0;
        free(header->comment);
        header->comment = tmp;

        header->commentLength += 90;
    }

    else if (strstr(recordName, "MARKER NAME"))
    {
        strncpy(header->markerName, line, 60);
        header->markerName[60] = 0;
    }

    else if (strstr(recordName, "MARKER NUMBER"))
    {
        strncpy(header->markerNumber, line, 20);
        header->markerNumber[20] = 0;
    }

    else if (strstr(recordName, "OBSERVER / AGENCY"))
    {
        strncpy(header->observerName, line, 20);
        header->observerName[20] = 0;
        strncpy(header->agency, line, 40);
        header->agency[40] = 0;
    }

    else if (strstr(recordName, "REC # / TYPE / VERS"))
    {
        strncpy(header->receiverNumber, line, 20);
        header->receiverNumber[20] = 0;
        strncpy(header->receiverType, &(line[20]), 20);
        header->receiverType[20] = 0;
        strncpy(header->receiverVersion, &(line[40]), 20);
        header->receiverVersion[20] = 0;
    }

    else if (strstr(recordName, "ANT # / TYPE"))
    {
        strncpy(header->antennaNumber, line, 20);
        header->antennaNumber[20] = 0;
        strncpy(header->antennaType, &(line[20]), 20);
        header->antennaType[20] = 0;
    }

    else if (strstr(recordName, "APPROX POSITION XYZ"))
    {
        sscanf(line, "%lf%lf%lf", &(header->approxMarkerX), &(header->approxMarkerY), &(header->approxMarkerZ));
    }

    else if (strstr(recordName, "ANTENNA: DELTA H/E/N"))
    {
        sscanf(line, "%lf%lf%lf", &(header->antennaHeight), &(header->antennaExcEast), &(header->antennaExcNorth));
    }

    else if (strstr(recordName, "WAVELENGTH FACT L1/2"))
    {
        if (!header->defaultWavelengthFactorSet)
        {
             sscanf(line, "%d%d", (int*)&(header->defaultWavelengthFactorL1), (int*)&(header->defaultWavelengthFactorL2));
             header->defaultWavelengthFactorSet = 1;
        }
        else
        {
            WaveLengthFactorRecord* tmp = malloc(sizeof(WaveLengthFactorRecord) * (header->waveLengthFactorRecordNumber + 1));
            if (header->nonDefaultWavelengthFactors)
            {
                memcpy(tmp, header->nonDefaultWavelengthFactors, sizeof(WaveLengthFactorRecord) * header->waveLengthFactorRecordNumber);
            }
            WaveLengthFactorRecord* ptr = &(tmp[header->waveLengthFactorRecordNumber]);
            sscanf(line, "%d%d%d", (int*)&(ptr->L1),
                                   (int*)&(ptr->L2),
                                   &(ptr->satelliteNumber));
            ptr->satellites = malloc(sizeof(int) * (ptr->satelliteNumber));
            int i;
            for (i = 0; i < ptr->satelliteNumber; i++)
            {
                char satSysID = 0;
                int satNum = 0;
                sscanf(&(line[21 + i * 6]), "%c%d", &satSysID, &satNum);
                if (satSysID == 'R')
                {
                    satNum += 100;
                }
                else if(satSysID != 'G')
                {
                    printf("%s: unsupported satellite system at wavelength factors: %c\n   in file %s\n", funcName, satSysID, obsFilePath);
                    return -1;
                }
                ptr->satellites[i] = satNum;
            }
            free(header->nonDefaultWavelengthFactors);
            header->nonDefaultWavelengthFactors = tmp;
            header->waveLengthFactorRecordNumber++;
        }
    }

    else if (strstr(recordName, "# / TYPES OF OBSERV"))
    {
        int tmp1 = 0;
        sscanf(line, "%d", &tmp1);
        if (tmp1)
        {
            header->obsTypeNumber = tmp1;
            header->observationCodes = malloc(sizeof(char) * tmp1);
            header->frequencyCodes = malloc(sizeof(int) * tmp1);
            memset(header->observationCodes, 0, sizeof(char) * tmp1);
            memset(header->frequencyCodes, 0, sizeof(char) * tmp1);
        }
        int i, j;
        for (j = 0; j < header->obsTypeNumber; j++)
        {
            if (header->observationCodes[j] == 0)
            {
                break;
            }
        }
        for (i = 0; i < 9; i++, j++)
        {
            if (j == header->obsTypeNumber)
            {
                break;
            }
            char tmp2 = 0;
            sscanf(&(line[6 * (i + 1) + 4]), "%c", &tmp2);
            if (header->observationCodes[i] == ' ')
            {
                break;
            }
            header->observationCodes[j] = tmp2;
            int tmp3 = 0;
            sscanf(&(line[6 * (i + 1) + 5]), "%d", &tmp3);
            header->frequencyCodes[j] = tmp3;
        }
    }

    else if (strstr(recordName, "INTERVAL"))
    {
        sscanf(line, "%lf", &(header->obsInterval));
    }

    else if (strstr(recordName, "TIME OF FIRST OBS") || strstr(recordName, "TIME OF LAST OBS"))
    {
        struct tm* time;
        TimeSystem* ts;
        if (strstr(recordName, "TIME OF FIRST OBS"))
        {
            time = &(header->firstObsTime);
            ts = &(header->firstObsTimeSystem);
        }
        else
        {
            time = &(header->lastObsTime);
            ts = &(header->lastObsTimeSystem);
        }
        int year, month;
        sscanf(line, "%d%d%d%d%d%d", &year,
                                     &month,
                                     &(time->tm_mday),
                                     &(time->tm_hour),
                                     &(time->tm_min),
                                     &(time->tm_sec));
        time->tm_year = year - 1900;
        time->tm_mon = month - 1;
        char timeSys[4] = {0};
        char tmp[13] = {0};
        strncpy(tmp, &(line[48]), sizeof(char) * 12);
        sscanf(tmp, "%s", timeSys);
        if (strstr(timeSys, "GPS"))
        {
            *ts = GPS_TIME;
        }
        else if (strstr(timeSys, "GLO"))
        {
            *ts = GLONASS_TIME;
        }
        else if (strlen(timeSys) != 0)
        {
            printf("%s: Unsupported time system: %s\n    in file %s\n", funcName, timeSys, obsFilePath);
            return -1;
        }
        else if (header->fileType == (FileType)G || header->fileType == (FileType)M)
        {
            *ts = GPS_TIME;
        }
        else if (header->fileType == (FileType)R)
        {
            *ts = GLONASS_TIME;
        }
    }

    else if (strstr(recordName, "RCV CLOCK OFFS APPL"))
    {
        sscanf(line, "%d", &(header->isClockOffsetApplied));
    }

    else if(strstr(recordName, "LEAP SECONDS"))
    {
        sscanf(line, "%d", &(header->leapSeconds));
    }

    else if(strstr(recordName, "# OF SATELLITES"))
    {
        sscanf(line, "%d", &(header->satelliteNumber));
    }

    else if(strstr(recordName, "PRN / # OF OBS"))
    {
        if (!header->obsCounts)
        {
            header->obsCounts = malloc(sizeof(ObservationCount) * header->satelliteNumber);
            memset(header->obsCounts, 0, sizeof(ObservationCount) * header->satelliteNumber);
        }
        int i,j;
        for (i = 0, j = 0; i < header->satelliteNumber; i++)
        {
            int shallIBreak = 0;
            if (!((header->obsCounts)[i].obsCount))
            {
                break;
            }
            for (; j < header->obsTypeNumber; j++)
            {
                if(!((header->obsCounts)[i].obsCount[j]));
                shallIBreak = 1;
                break;
            }
            if(shallIBreak)
            {
                break;
            }
        }
        char satType = 0;
        sscanf(line, "%c", &satType);
        if (satType)
        {
            if (j)
            {
                printf("%s: Format error in \"PRN / # OF OBS\" record, probable cause: prn is repeated in followup line\n" \
                       "    in file %s\n", funcName, obsFilePath);
                return -1;
            }
            int satNum = 0;
            sscanf(&(line[4]), "%d", &satNum);
            if (satType == 'R')
            {
                satNum += 100;
            }
            else if (satType != 'G')
            {
                printf("%s: unsupported satellite system at number of observations: %c\n    in file %s\n", funcName, satType, obsFilePath);
                return -1;
            }
            (header->obsCounts)[i].prn = satNum;
            (header->obsCounts)[i].obsCount = malloc(sizeof(int) * header->obsTypeNumber);
        }
        int k;
        for(k = 0; k < 9; k++)
        {
            if (j == header->obsTypeNumber)
            {
                break;
            }
            int tmp = -1;
            sscanf(&(line[(k + 1) * 6]), "%d", &tmp);
            if (tmp == -1)
            {
                break;
            }
            (header->obsCounts)[i].obsCount[j] = tmp;
            j++;
        }
    }

    else if(strstr(recordName, "END OF HEADER"))
    {
        return 0;
    }

    return 1;
}



void insertObservation(GNSSObservation** observations, GNSSObservation* observation, int* totalObservationCount, int satId)
{
    GNSSObservation* tmp = malloc(sizeof(GNSSObservation) * (totalObservationCount[satId] + 1));
    GNSSObservation* ptr = 0;
    int i;
    int posFound = 0;
    for (i = 0; i < totalObservationCount[satId]; i++)
    {
        if(comparePreciseTime(&(observations[satId][i].
                                epoch), &(observation->epoch)) <= 0)
        {
            ptr = &(tmp[i]);
            copyGNSSObservation(&ptr, &(observations[satId][i]), 0);
        }
        else
        {
            posFound = 1;
        }
        if(posFound)
        {
            break;
        }
    }
    ptr = &(tmp[i]);
    copyGNSSObservation(&ptr, observation, 0);
    for (;i < totalObservationCount[satId]; i++)
    {
        copyGNSSObservation(&ptr, &(observations[satId][i]), 0);
    }
    for (i = 0; i < totalObservationCount[satId]; i++)
    {
        deleteGNSSObservation(&(observations[satId][i]));
    }
    free(observations[satId]);
    observations[satId] = tmp;
    (totalObservationCount[satId])++;
}

/*
 * observations must be a satTypeNum * satPerType size array of null pointers
 * headers must be the address of a null pointer
 *
 * return value is 1 if parsing was successful and 0 if it was not.
 */
int parseGNSSObservationFile(GNSSObservation** obsrv, int* totalObservationCount, GNSSObservationHeader** headers, char* obsFilePath, int* headerCount)
{
    char funcName[] = "parseObservationFile()";
    int i;
    for (i = 0; i < satTypeNum * satPerType; i++)
    {
        if (obsrv[i])
        {
            printf("%s: Observation pointer with prn %d in observations is not null\n", funcName, i);
            return 0;
        }
    }
    if (*headers)
    {
         printf("%s: Observation headers pointer is not null\n", funcName);
         return 0;
    }
    if (!obsFilePath)
    {
        printf("%s: Null pointer passed as obsFilePath\n", funcName);
        return 0;
    }
    i = 0;
    while (obsFilePath[i])
    {
        i++;
    }
    if (obsFilePath[i-1] != 'O' && obsFilePath[i-1] != 'o')
    {
        printf("%s: File %s is not an observation file (does not end with 'O')\n", funcName, obsFilePath);
        return 0;
    }

    FILE* obsFile = fopen(obsFilePath, "r");
    if (!obsFile)
    {
        printf("%s: Could not open observation file %s\n", funcName, obsFilePath);
        return 0;
    }

    int headerSection = 1;
    int epochSection = 0;
    int observationDataSection = 0;

    int eventNum = 0;
    int* eventFlags = 0;
    int* midDataHeaderLineNum = 0;
    char*** midDataHeaderInformation = 0;

    *headers = malloc(sizeof(GNSSObservationHeader));
    initGNSSObservationHeader(*headers);
    GNSSObservationHeader* currentHeader = *headers;

    char line[82] = {0};
    int lineNum = 0;

    //29 char
    char commentTime[] = "Initial comment              ";

    GNSSObservation singleObservations[satTypeNum * satPerType];
    for(i = 0; i < satTypeNum * satPerType; i++)
    {
        initGNSSObservation(&(singleObservations[i]));
    }

    PreciseTime currentEpoch;
    int currentSatelliteNum = 0;
    int* currentSatellites = 0;
    double currentClockOffset = 0;

    while (!feof(obsFile))
    {
        memset(line, 0, 82);
        fgets(line, 82, obsFile);
        lineNum++;
        //header information is coming
        if (headerSection)
        {
            if (checkEmptyLine(line))
            {
                continue;
            }
            headerSection = parseGNSSObservationHeader(line, currentHeader, commentTime, obsFilePath);
            if (headerSection == -1)
            {
                return 0;
            }
            else if (headerSection == 0)
            {
                (*headerCount)++;
                epochSection = 1;
            }
        }
        //observation epoch
        else if (epochSection)
        {
            if (checkEmptyLine(line))
            {
                continue;
            }
            int* newEventFlags = malloc(sizeof(int) * (eventNum + 1));
            int j;
            for (j = 0; j < eventNum; j++)
            {
                newEventFlags[j] = eventFlags[j];
            }
            newEventFlags[j] = line[28] - '0';
            free(eventFlags);
            eventFlags = newEventFlags;
            eventNum++;
            if (line[28] > '1' && line[28] != '6')
            {
                char*** tmp1 = malloc(sizeof(char**) * (eventNum));
                int* tmp2 = malloc(sizeof(int) * (eventNum));
                for (j = 0; j < eventNum-1; j++)
                {
                    tmp1[j] = midDataHeaderInformation[j];
                    tmp2[j] = midDataHeaderLineNum[j];
                }
                tmp1[eventNum-1] = 0;
                tmp2[eventNum-1] = 0;
                sscanf(&(line[29]), "%d", &(tmp2[eventNum-1]));
                tmp1[eventNum-1] = malloc(sizeof(char*) * (tmp2[eventNum-1] + 1));
                for (j = 0; j < tmp2[eventNum-1] + 1; j++)
                {
                    tmp1[eventNum-1][j] = malloc(sizeof(char) * 82);
                    if (j != 0)
                    {
                        if (!feof(obsFile))
                        {
                            memset(line, 0, 82);
                            fgets(line, 82, obsFile);
                            lineNum++;
                        }
                        else
                        {
                            printf("%s: Unexpected end of file %s at line %d\n", funcName, obsFilePath, lineNum);
                            //itt nagyin sokmindent kellene freezni
                            return 0;
                        }
                    }
                    strncpy(tmp1[eventNum-1][j], line, 82);
                }
                free(midDataHeaderInformation);
                midDataHeaderInformation = tmp1;
                free(midDataHeaderLineNum);
                midDataHeaderLineNum = tmp2;
                continue;
            }
            else if (eventNum > 1)
            {
                GNSSObservationHeader* tmp = malloc(sizeof(GNSSObservationHeader));
                initGNSSObservationHeader(tmp);
                copyGNSSObservationHeader(&tmp, currentHeader, 0);
                for (j = 0; j < eventNum-1; j++)
                {
                    memset(commentTime, 0, sizeof(char) * 30);
                    if (midDataHeaderInformation[j][0][1] == ' ')
                    {
                        strncpy(commentTime, line, 26);
                    }
                    else
                    {
                        strncpy(commentTime, midDataHeaderInformation[j][0], 26);
                    }
                    memset(&(commentTime[26]), ' ', sizeof(char) * 3);
                    int k;
                    for (k = 0; k < midDataHeaderLineNum[j] + 1; k++)
                    {
                        if (k != 0)
                        {
                            if (parseGNSSObservationHeader(midDataHeaderInformation[j][k], tmp, commentTime, obsFilePath) == -1)
                            {
                                return 0;
                            }
                        }
                        free(midDataHeaderInformation[j][k]);
                    }
                    free(midDataHeaderInformation[j]);
                }
                free(midDataHeaderInformation);
                free(midDataHeaderLineNum);
                midDataHeaderInformation = 0;
                midDataHeaderLineNum = 0;
                currentHeader->nextHeader = tmp;
                currentHeader = tmp;
                (*headerCount)++;
            }
            //reading time data
            struct tm time;
            memset(&time, 0, sizeof(struct tm));
            int year, month;
            sscanf(line, "%d%d%d%d%d%d", &year,
                                         &month,
                                         &(time.tm_mday),
                                         &(time.tm_hour),
                                         &(time.tm_min),
                                         &(time.tm_sec));
            if(year < 80)
            {
                time.tm_year = year + 100;
            }
            else
            {
                time.tm_year = year;
            }
            time.tm_mon = month - 1;
            currentEpoch.seconds = timegm(&time);
            sscanf(&(line[19]), "%d", &(currentEpoch.nanos));
            currentEpoch.nanos *= 100;

            //reading satellites
            sscanf(&(line[29]), "%d", &currentSatelliteNum);
            int currentSatLineNum = 0;
            if(currentSatellites)
            {
                free(currentSatellites);
            }
            currentSatellites = malloc(sizeof(int) * currentSatelliteNum);
            int allSatRead = 0;
            while (!allSatRead)
            {
                for (i = 0; i < 12 ; i++)
                {
                    char satType = 0;
                    int satNum = 0;
                    sscanf(&(line[32+3*i]), "%c%d", &satType, &satNum);
                    if (satType == 'R')
                    {
                        satNum += 100;
                    }
                    else if (satType != 'G' && (satType != ' '))
                    {
                        printf("%s: unsupported satellite system at number of observations: %c\n    in file %s at line %d\n", funcName, satType, obsFilePath, lineNum);
                        if(currentSatellites)
                        {
                            free(currentSatellites);
                        }
                        return 0;
                    }
                    currentSatellites[currentSatLineNum * 12 + i] = satNum;
                    //reading clock offset
                    sscanf(&(line[67]), "%lf", &currentClockOffset);
                    if ((currentSatLineNum * 12 + i + 1) == currentSatelliteNum)
                    {
                        allSatRead = 1;
                        break;
                    }
                }
                currentSatLineNum++;
                if (!allSatRead)
                {
                    if (!feof(obsFile))
                    {
                        memset(line, 0, 82);
                        fgets(line, 82, obsFile);
                        lineNum++;
                    }
                    else
                    {
                        printf("%s: Unexpected end of file %s at line %d\n", funcName, obsFilePath, lineNum);
                        return 0;
                    }
                }
            }
            epochSection = 0;
            observationDataSection = 1;
        }

        if (observationDataSection)
        {
            int obsTypeNumber = currentHeader->obsTypeNumber;
            int fullDataLineNum = obsTypeNumber / 5;
            int remainingDataNum = obsTypeNumber % 5;
            int j, k, l;
            for (j = 0; j < currentSatelliteNum && !feof(obsFile) ; j++)
            {
                singleObservations[currentSatellites[j]].epoch.seconds = currentEpoch.seconds;
                singleObservations[currentSatellites[j]].epoch.nanos = currentEpoch.nanos;
                singleObservations[currentSatellites[j]].header = currentHeader;
                singleObservations[currentSatellites[j]].eventFlagNum = eventNum;
                singleObservations[currentSatellites[j]].eventFlags = malloc(sizeof(int) * eventNum);
                memcpy(singleObservations[currentSatellites[j]].eventFlags,  eventFlags, sizeof(int) * eventNum);
                int observationIndex = 0;
                singleObservations[currentSatellites[j]].observations = malloc(sizeof(double) * obsTypeNumber);
                singleObservations[currentSatellites[j]].lli = malloc(sizeof(int) * obsTypeNumber);
                singleObservations[currentSatellites[j]].signalStrength = malloc(sizeof(int) * obsTypeNumber);
                for (k = 0; k < fullDataLineNum && !feof(obsFile); k++)
                {
                    memset(line, 0, 82);
                    fgets(line, 82, obsFile);
                    lineNum++;
                    for (l = 0; l < 5; l++, observationIndex++)
                    {
                        double observations = 0;
                        int lli = 0;
                        int signalStrength = 0;
                        char doubleData[15];
                        strncpy(doubleData, &(line[l * 16]), 14);
                        sscanf(doubleData, "%lf", &observations);
                        lli = line[l * 16 + 14];
                        if (lli == ' ' || lli == '\n')
                        {
                            lli = 0;
                        }
                        else if (lli != 0)
                        {
                            lli -= '0';
                        }
                        signalStrength = line[l * 16 + 15];
                        if (signalStrength == ' ' || signalStrength == '\n')
                        {
                            signalStrength = 0;
                        }
                        else if (signalStrength != 0)
                        {
                            signalStrength -= '0';
                        }
                        (singleObservations[currentSatellites[j]].observations)[k * 5 + l] = observations;
                        (singleObservations[currentSatellites[j]].lli)[k * 5 + l]= lli;
                        (singleObservations[currentSatellites[j]].signalStrength)[k * 5 + l] = signalStrength;
                    }
                }
                if (remainingDataNum && !feof(obsFile))
                {
                    memset(line, 0, 82);
                    fgets(line, 82, obsFile);
                    lineNum++;
                    for (l = 0; l < remainingDataNum; l++)
                    {
                        double observations = 0;
                        int lli = 0;
                        int signalStrength = 0;
                        char doubleData[15] = {0};
                        strncpy(doubleData, &(line[l * 16]), 14);
                        sscanf(doubleData, "%lf", &observations);
                        lli = line[l * 16 + 14];
                        if (lli == ' ' || lli == '\n')
                        {
                            lli = 0;
                        }
                        else if (lli != 0)
                        {
                            lli -= '0';
                        }
                        signalStrength = line[l * 16 + 15];
                        if (signalStrength == ' ' || signalStrength == '\n')
                        {
                            signalStrength = 0;
                        }
                        else if (signalStrength != 0)
                        {
                            signalStrength -= '0';
                        }
                        (singleObservations[currentSatellites[j]].observations)[k * 5 + l] = observations;
                        (singleObservations[currentSatellites[j]].lli)[k * 5 + l] = lli;
                        (singleObservations[currentSatellites[j]].signalStrength)[k * 5 + l] = signalStrength;
                    }
                }
            }
            for (j = 0; j < currentSatelliteNum && !feof(obsFile) ; j++)
            {
                int satId = currentSatellites[j];
                insertObservation(obsrv, &(singleObservations[satId]), totalObservationCount, satId);
            }
            //reset single obs
            for(j = 0; j < satTypeNum * satPerType; j++)
            {
                deleteGNSSObservation(&(singleObservations[j]));
                initGNSSObservation(&(singleObservations[j]));
            }
            if(eventFlags)
            {
                free(eventFlags);
            }
            eventFlags = 0;
            eventNum = 0;
            observationDataSection = 0;
            epochSection = 1;
        }
    }
    if(currentSatellites)
    {
        free(currentSatellites);
    }
    fclose(obsFile);
    return 1;
}
