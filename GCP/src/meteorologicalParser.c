#include "meteorologicalParser.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


void initMeteorologicalSensorType(MeteorologicalSensorType* sensorType)
{
    memset(sensorType, 0, sizeof(MeteorologicalSensorType));
}


void initMeteorologicalSensorPos(MeteorologicalSensorPos* sensorPos)
{
    memset(sensorPos, 0, sizeof(MeteorologicalSensorPos));
}


void initMeteorologicalHeader(MeteorologicalHeader* header)
{
    memset(header, 0, sizeof(MeteorologicalHeader));
}


void copyMeteorologicalHeader(MeteorologicalHeader** dest, MeteorologicalHeader* header, int allocNewMem)
{
    MeteorologicalHeader* newHeader = *dest;
    if (allocNewMem || !newHeader)
    {
        newHeader = malloc(sizeof(MeteorologicalHeader));
    }
    memcpy(newHeader, header, sizeof(MeteorologicalHeader));
    if (header->comment)
    {
        newHeader->comment = malloc(sizeof(char) * header->commentLength);
        memcpy(newHeader->comment, header->comment, sizeof(char) * header->commentLength);
    }
    if (header->obsTypes)
    {
        newHeader->obsTypes = malloc(sizeof(char) * 3 * newHeader->obsTypeNumber);
        memcpy(newHeader->obsTypes, header->obsTypes, sizeof(char) * 3 * newHeader->obsTypeNumber);
    }
    if (header->sensorTypes)
    {
         newHeader->sensorTypes = malloc(sizeof(MeteorologicalSensorType) *  newHeader->obsTypeNumber);
         memcpy(newHeader->sensorTypes, header->sensorTypes, sizeof(MeteorologicalSensorType) *  newHeader->obsTypeNumber);
    }
    if (header->sensorPositions)
    {
        newHeader->sensorPositions = malloc(sizeof(MeteorologicalSensorPos) * newHeader->sensorPosNum);
        memcpy(newHeader->sensorPositions, header->sensorPositions, sizeof(MeteorologicalSensorPos) * newHeader->sensorPosNum);
    }
}


void deleteMeteorologicalHeader(MeteorologicalHeader* header)
{
    if (header->comment != 0)
    {
        free(header->comment);
    }
    if (header->obsTypes)
    {
        free(header->obsTypes);
    }
    if (header->sensorTypes)
    {
        free(header->sensorTypes);
    }
    if (header->sensorPositions)
    {
        free(header->sensorPositions);
    }
}


void printMeteorologicalHeader(MeteorologicalHeader* header)
{
    printf("Version: %lf,    file type: %d\n", header->rinex_version, header->fileType);
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
    if (header->obsTypeNumber)
    {
        printf("Observation types:\n");
        int i;
        for (i = 0; i < header->obsTypeNumber; i++)
        {
            printf(" \n%s\n",  header->obsTypes[i]);
            printf("sensor model: %s\n", header->sensorTypes[i].model);
            printf("sensor type: %s\n", header->sensorTypes[i].type);
            printf("sensor type: %lf\n", header->sensorTypes[i].accuracy);
            printf("obsType: %s\n", header->sensorTypes[i].obsType);
        }
        printf("\n");
    }
    else
    {
        printf("No observation types given\n");
    }
    printf("%p\n", header->sensorPositions);
    if (header->sensorPosNum)
    {
        printf("Sensor positions:\n");
        int i;
        for (i = 0; i < header->sensorPosNum; i++)
        {
            printf("\nsensor position x: %lf\n", header->sensorPositions[i].posX);
            printf("sensor position y: %lf\n", header->sensorPositions[i].posY);
            printf("sensor position z: %lf\n", header->sensorPositions[i].posZ);
            printf("sensor position h: %lf\n", header->sensorPositions[i].ellipsH);
            printf("obsType: %s\n", header->sensorPositions[i].obsType);
        }
    }
}



void initMeteorologicalData(MeteorologicalData* observation)
{
    memset(observation, 0, sizeof(MeteorologicalData));
}

void copyMeteorologicalData(MeteorologicalData** dst, MeteorologicalData* source, int allocNewMem)
{
    MeteorologicalData* dest = *dst;
    if(allocNewMem || !dest)
    {
        dest = malloc(sizeof(MeteorologicalData));
    }
    memcpy(&(dest->epoch), &(source->epoch), sizeof(PreciseTime));
    dest->header = source->header;
    int obsTypeNumber = source->header->obsTypeNumber;

    dest->observations = malloc(sizeof(double) * obsTypeNumber);
    memcpy(dest->observations, source->observations, sizeof(double) * obsTypeNumber);
}

void deleteMeteorologicalData(MeteorologicalData* observation)
{
    if (observation->observations)
    {
        free(observation->observations);
    }
}

void printMeteorologicalData(MeteorologicalData** observations, int satNum, int* totalObservationCount)
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
                printf("    epoch: %ld.%d,    header: %p\n", observations[i][j].epoch.seconds, observations[i][j].epoch.nanos,
                                                             observations[i][j].header);
                int k;
                for (k = 0; k < observations[i][j].header->obsTypeNumber; k++)
                {
                    printf("        %s: %lf\n", observations[i][j].header->obsTypes[k],
                                                observations[i][j].observations[k]);
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
int parseMeteorologicalHeader(char* line, MeteorologicalHeader* header, char* commentTime)
{
    char funcName[] = "parseMeteorologicalHeader()";
    char* recordName = &(line[60]);

    if (strstr(recordName, "RINEX VERSION / TYPE"))
    {
        double tmp = 0;
        sscanf(line, "%lf", &tmp);
        if (tmp != rinexVersion)
        {
            printf("%s: bad rinex version, required: %lf, got: %lf\n", funcName, rinexVersion, tmp);
            return -1;
        }
        header->rinex_version = tmp;
        if (line[20] != 'M' && line[20] != 'm')
        {
            printf("%s: bad file type, required: 'M' or 'm', got: %c\n", funcName, line[20]);
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

    else if (strstr(recordName, "# / TYPES OF OBSERV"))
    {
        int tmp1 = 0;
        sscanf(line, "%d", &tmp1);
        if (tmp1)
        {
            header->obsTypeNumber = tmp1;
            header->obsTypes = malloc(sizeof(char[3]) * tmp1);
            memset(header->obsTypes, 0, sizeof(char[3]) * tmp1);
            header->sensorTypes = malloc(sizeof(MeteorologicalSensorType) * tmp1);
            memset(header->sensorTypes, 0, sizeof(MeteorologicalSensorType) * tmp1);
        }
        int i, j;
        for (j = 0; j < header->obsTypeNumber; j++)
        {
            if (header->obsTypes[j][0] == 0)
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
            char tmp2[3] = {0};
            sscanf(&(line[6 * (i + 1) + 4]), "%s", tmp2);
            if (tmp2[0] == ' ' || tmp2[0] == 0)
            {
                break;
            }
            memcpy(header->obsTypes[j], tmp2, 2);
        }
    }

    else if(strstr(recordName, "SENSOR MOD/TYPE/ACC"))
    {
        double tmp = 0;
        int i;
        for (i = 0; i < header->obsTypeNumber; i++)
        {
            if(!header->sensorTypes[i].obsType[0])
            {
                break;
            }
        }
        if(i == header->obsTypeNumber)
        {
            printf("%s: more sensor information received than observation type number:\n", funcName);
            return -1;
        }
        sscanf(&(line[46]), "%lf", &tmp);
        strncpy(header->sensorTypes[i].model, line, 20);
        header->sensorTypes[i].model[20] = 0;
        strncpy(header->sensorTypes[i].type, &(line[20]), 20);
        header->sensorTypes[i].type[20] = 0;
        header->sensorTypes[i].accuracy = tmp;
        sscanf(&(line[57]), "%s", header->sensorTypes[i].obsType);
    }

    else if(strstr(recordName, "SENSOR POS XYZ/H"))
    {
        MeteorologicalSensorPos* tmp = malloc(sizeof(MeteorologicalSensorPos) * (header->sensorPosNum + 1));
        memcpy(tmp, header->sensorPositions, sizeof(MeteorologicalSensorPos) * header->sensorPosNum);
        memset(&(tmp[header->sensorPosNum]), 0, sizeof(MeteorologicalSensorPos));
        sscanf(line, "%lf %lf %lf %lf %s", &(tmp[header->sensorPosNum].posX),
                                           &(tmp[header->sensorPosNum].posY),
                                           &(tmp[header->sensorPosNum].posZ),
                                           &(tmp[header->sensorPosNum].ellipsH),
                                           tmp[header->sensorPosNum].obsType);
        free(header->sensorPositions);
        header->sensorPositions = tmp;
        header->sensorPosNum++;
    }

    else if(strstr(recordName, "END OF HEADER"))
    {
        return 0;
    }

    return 1;
}



void insertMeteorologicalData(MeteorologicalData** observations, MeteorologicalData* observation, int* totalObservationCount, int satId)
{
    MeteorologicalData* tmp = malloc(sizeof(MeteorologicalData) * (totalObservationCount[satId] + 1));
    MeteorologicalData* ptr = 0;
    int i;
    int posFound = 0;
    for (i = 0; i < totalObservationCount[satId]; i++)
    {
        if(comparePreciseTime(&(observations[satId][i].
                                epoch), &(observation->epoch)) <= 0)
        {
            ptr = &(tmp[i]);
            copyMeteorologicalData(&ptr, &(observations[satId][i]), 0);
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
    copyMeteorologicalData(&ptr, observation, 0);
    for (;i < totalObservationCount[satId]; i++)
    {
        copyMeteorologicalData(&ptr, &(observations[satId][i]), 0);
    }
    for (i = 0; i < totalObservationCount[satId]; i++)
    {
        deleteMeteorologicalData(&(observations[satId][i]));
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
int parseMeteorologicalFile(MeteorologicalData** obsrv, int* totalObservationCount, MeteorologicalHeader** header, char* metFilePath)
{
    char funcName[] = "parseMeteorologicalFile()";
    if (*obsrv)
    {
        printf("%s: Observation pointer is not null\n", funcName);
        return 0;
    }
    if (*header)
    {
        printf("%s: Observation headers pointer is not null\n", funcName);
        return 0;
    }
    if (!metFilePath)
    {
        printf("%s: Null pointer passed as metFilePath\n", funcName);
        return 0;
    }
    int i = 0;
    while (metFilePath[i])
    {
        i++;
    }
    if (metFilePath[i-1] != 'M' && metFilePath[i-1] != 'm')
    {
        printf("%s: File %s is not a meteorological file (does not end with 'M')\n", funcName, metFilePath);
        return 0;
    }

    FILE* metFile = fopen(metFilePath, "r");
    if (!metFile)
    {
        printf("%s: Could not open observation file %s\n", funcName, metFilePath);
        return 0;
    }

    int headerSection = 1;
    int epochSection = 0;
    int observationDataSection = 0;
    *header = malloc(sizeof(MeteorologicalHeader));
    initMeteorologicalHeader(*header);
    char line[82] = {0};
    //29 char
    char commentTime[] = "Initial comment              ";
    MeteorologicalData singleObservation;
    initMeteorologicalData(&singleObservation);
    PreciseTime currentEpoch;
    currentEpoch.seconds = 0;
    currentEpoch.nanos = 0;
    int lineNum = 0;
    while (!feof(metFile))
    {
        memset(line, 0, 82);
        fgets(line, 82, metFile);
        lineNum++;
        //header information is coming
        if (headerSection)
        {
            if (checkEmptyLine(line))
            {
                continue;
            }
            headerSection = parseMeteorologicalHeader(line, *header, commentTime);
            if (headerSection == -1)
            {
                return 0;
            }
            else if (headerSection == 0)
            {
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

            epochSection = 0;
            observationDataSection = 1;
        }

        if (observationDataSection)
        {
            singleObservation.epoch = currentEpoch;
            singleObservation.header = *header;
            int obsTypeNumber = (*header)->obsTypeNumber;
            singleObservation.observations = malloc(sizeof(double) * obsTypeNumber);
            int maxObsPerLine = 8;
            if (obsTypeNumber < maxObsPerLine)
            {
                maxObsPerLine = obsTypeNumber;
            }
            int j;
            //read first line
            for (j = 0; j < maxObsPerLine; j++)
            {
                sscanf(&(line[18 + 7 * j]), "%lf", &(singleObservation.observations[j]));
            }
            //read following lines if there is any
            if (obsTypeNumber > 8)
            {
                maxObsPerLine = 10;
                int fullDataLineNum = (obsTypeNumber - 8) / maxObsPerLine;
                int remainingDataNum = (obsTypeNumber - 8) % maxObsPerLine;
                int observationIndex = 8;
                int j, k;
                for (j = 0; j < fullDataLineNum && !feof(metFile); j++)
                {
                    memset(line, 0, 82);
                    fgets(line, 82, metFile);
                    lineNum++;
                    for (k = 0; k < maxObsPerLine; k++, observationIndex++)
                    {
                        double observation = 0;
                        char doubleData[8];
                        strncpy(doubleData, &(line[4 + k * 7]), 7);
                        sscanf(doubleData, "%lf", &observation);
                        singleObservation.observations[observationIndex] = observation;
                    }
                }
                if (remainingDataNum && !feof(metFile))
                {
                    memset(line, 0, 82);
                    fgets(line, 82, metFile);
                    lineNum++;
                    for (k = 0; k < remainingDataNum; k++, observationIndex++)
                    {
                        double observation = 0;
                        char doubleData[15];
                        strncpy(doubleData, &(line[4 + k * 7]), 7);
                        sscanf(doubleData, "%lf", &observation);
                        singleObservation.observations[observationIndex] = observation;
                    }
                }

            }
            observationDataSection = 0;
            epochSection = 1;
            insertMeteorologicalData(obsrv, &singleObservation, totalObservationCount, 0);
            deleteMeteorologicalData(&singleObservation);
            initMeteorologicalData(&singleObservation);
        }
    }
    fclose(metFile);
    return 1;
}
