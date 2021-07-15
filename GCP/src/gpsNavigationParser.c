#include "gpsNavigationParser.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


void initGPSNavigationHeader(GPSNavigationHeader* header)
{
    memset(header, 0, sizeof(GPSNavigationHeader));
}


void copyGPSNavigationHeaderHeader(GPSNavigationHeader** dest, GPSNavigationHeader* header, int allocNewMem)
{
    GPSNavigationHeader* newHeader = *dest;
    if (allocNewMem || !newHeader)
    {
        newHeader = malloc(sizeof(GPSNavigationHeader));
    }
    memcpy(newHeader, header, sizeof(GPSNavigationHeader));
    if(header->comment)
    {
        newHeader->comment = malloc(sizeof(char) * header->commentLength);
        memcpy(newHeader->comment, header->comment, sizeof(char) * header->commentLength);
    }
}


void deleteGPSNavigationHeader(GPSNavigationHeader* header)
{
    if (header->comment != 0)
    {
        free(header->comment);
    }
}


void printGPSNavigationHeader(GPSNavigationHeader* header)
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
    printf("Ionosphere data:\n");
    printf("A0: %lf,    A1: %lf,    A2: %lf,    A3: %lf\n", header->ionosphereA0, header->ionosphereA1, header->ionosphereA2, header->ionosphereA3);
    printf("B0: %lf,    B1: %lf,    B2: %lf,    b3: %lf\n", header->ionosphereB0, header->ionosphereB1, header->ionosphereB2, header->ionosphereB3);
    printf("Almanach parameters:\n");
    printf("A0: %lf,    A1: %lf,    T: %ld,    W: %ld\n", header->almanachA0, header->almanachA1, header->referenceTime, header->referenceWeek);
    printf("Leap seconds: %d\n", header-> leapSeconds);
}



void initGPSNavigationData(GPSNavigationData* navData)
{
    memset(navData, 0, sizeof(GPSNavigationData));
}


void copyGPSNavigationData(GPSNavigationData** dst, GPSNavigationData* source, int allocNewMem)
{
    GPSNavigationData* dest = *dst;
    if(allocNewMem || !dest)
    {
        dest = malloc(sizeof(GPSNavigationData));
    }
    memcpy(dest, source, sizeof(GPSNavigationData));
}


void printGPSNavigationDatas(GPSNavigationData** navDatas, int satNum, int* totalNavDataCount)
{
    int i;
    for (i = 0; i < satNum; i++)
    {
        if (navDatas[i])
        {
            printf("\n\n\n\n\nNavigation data for satellite %d:\n", i);
            int j;
            for (j = 0; j < totalNavDataCount[i]; j++)
            {
                printf("    epoch: %ld.%d,    header: %p\n", navDatas[i][j].epoch.seconds, navDatas[i][j].epoch.nanos,
                                                             navDatas[i][j].header);
                printf("svClockBias: %le,    svClockDrift: %le,    svClockDriftRate: %le\n", navDatas[i][j].svClockBias,
                                                                                             navDatas[i][j].svClockDrift,
                                                                                             navDatas[i][j].svClockDriftRate);
                printf("ephimeris: %le,    crs: %le,    deltaN: %le,    m0: %le\n", navDatas[i][j].ephimeris,
                                                                                    navDatas[i][j].crs,
                                                                                    navDatas[i][j].deltaN,
                                                                                    navDatas[i][j].m0);
                printf("cuc: %le,    eccentricity: %le,    cus: %le,    sqrtA: %le\n", navDatas[i][j].cuc,
                                                                                       navDatas[i][j].eccentricity,
                                                                                       navDatas[i][j].cus,
                                                                                       navDatas[i][j].sqrtA);
                printf("toe: %le,    cic: %le,    OMEGA: %le,    cis: %le\n", navDatas[i][j].toe,
                                                                              navDatas[i][j].cic,
                                                                              navDatas[i][j].OMEGA,
                                                                              navDatas[i][j].cis);
                printf("i0: %le,    crc: %le,    omegaLowerCase: %le,    OMEGADOT: %le\n", navDatas[i][j].i0,
                                                                                           navDatas[i][j].crc,
                                                                                           navDatas[i][j].omegaLowerCase,
                                                                                           navDatas[i][j].OMEGADOT);
                printf("iDot: %le,    L2Codes: %le,    gpsWeek: %le,    L2PDataFlag: %le\n", navDatas[i][j].iDot,
                                                                                             navDatas[i][j].L2Codes,
                                                                                             navDatas[i][j].gpsWeek,
                                                                                             navDatas[i][j].L2PDataFlag);
                printf("svAccuracy: %le,    svHealth: %le,    tgd: %le,    iodc: %le\n", navDatas[i][j].svAccuracy,
                                                                                         navDatas[i][j].svHealth,
                                                                                         navDatas[i][j].tgd,
                                                                                         navDatas[i][j].iodc);
                printf("transmissionTime: %le,    fitInterval: %le\n", navDatas[i][j].transmissionTime,
                                                                       navDatas[i][j].fitInterval);
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
int parseGPSNavigationHeader(char* line, GPSNavigationHeader* header, char* commentTime)
{

    char funcName[] = "parseGPSNAvigationHeader()";
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
        if (line[20] != 'N' && line[20] != 'n')
        {
            printf("%s: bad file type, required: 'N' or 'n', got: %c\n", funcName, line[20]);
            return -1;
        }
        header->fileType = NAV_MSG;
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

    else if(strstr(recordName, "ION ALPHA"))
    {
        char tmp[13] = {0};
        strncpy(tmp, &(line[2]), 12);
        sscanf(tmp, "%lf", &(header->ionosphereA0));
        memset(tmp, 0, sizeof(char) * 13);
        strncpy(tmp, &(line[2  + 12]), 12);
        sscanf(tmp, "%lf", &(header->ionosphereA1));
        memset(tmp, 0, sizeof(char) * 13);
        strncpy(tmp, &(line[2 + 24]), 12);
        sscanf(tmp, "%lf", &(header->ionosphereA2));
        memset(tmp, 0, sizeof(char) * 13);
        strncpy(tmp, &(line[2 + 36]), 12);
        sscanf(tmp, "%lf", &(header->ionosphereA3));
    }

    else if(strstr(recordName, "ION BETA"))
    {
        char tmp[13] = {0};
        strncpy(tmp, &(line[2]), 12);
        sscanf(tmp, "%lf", &(header->ionosphereB0));
        memset(tmp, 0, sizeof(char) * 13);
        strncpy(tmp, &(line[2  + 12]), 12);
        sscanf(tmp, "%lf", &(header->ionosphereB1));
        memset(tmp, 0, sizeof(char) * 13);
        strncpy(tmp, &(line[2 + 24]), 12);
        sscanf(tmp, "%lf", &(header->ionosphereB2));
        memset(tmp, 0, sizeof(char) * 13);
        strncpy(tmp, &(line[2 + 36]), 12);
        sscanf(tmp, "%lf", &(header->ionosphereB3));
    }

    else if(strstr(recordName, "DELTA-UTC: A0,A1,T,W"))
    {
        char tmp[19] = {0};
        strncpy(tmp, &(line[3]), 19);
        sscanf(tmp, "%lf", &(header->almanachA0));
        memset(tmp, 0, sizeof(char) * 19);
        strncpy(tmp, &(line[3  + 19]), 19);
        sscanf(tmp, "%lf", &(header->almanachA1));
        memset(tmp, 0, sizeof(char) * 19);
        strncpy(tmp, &(line[3 + 38]), 9);
        sscanf(tmp, "%ld", &(header->referenceTime));
        memset(tmp, 0, sizeof(char) * 19);
        strncpy(tmp, &(line[3 + 47]), 9);
        sscanf(tmp, "%ld", &(header->referenceWeek));
    }

    else if(strstr(recordName, "LEAP SECONDS"))
    {
        sscanf(line, "%d", &(header->leapSeconds));
    }

    else if(strstr(recordName, "END OF HEADER"))
    {
        return 0;
    }

    return 1;
}



void insertGPSNavigationData(GPSNavigationData** navDatas, GPSNavigationData* navData, int* totalNavDataCount, int satId)
{
    GPSNavigationData* tmp = malloc(sizeof(GPSNavigationData) * (totalNavDataCount[satId] + 1));
    GPSNavigationData* ptr = 0;
    int i;
    int posFound = 0;
    for (i = 0; i < totalNavDataCount[satId]; i++)
    {
        if(comparePreciseTime(&(navDatas[satId][i].epoch), &(navData->epoch)) <= 0)
        {
            ptr = &(tmp[i]);
            copyGPSNavigationData(&ptr, &(navDatas[satId][i]), 0);
            //tmp[i] = navDatas[satId][i];
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
    copyGPSNavigationData(&ptr, navData, 0);
    //tmp[i] = *navData;
    for (;i < totalNavDataCount[satId]; i++)
    {
        ptr = &(tmp[i + 1]);
        copyGPSNavigationData(&ptr, &(navDatas[satId][i]), 0);
        //tmp[i] = navDatas[satId][i];
    }
    if(navDatas[satId])
    {
        free(navDatas[satId]);
    }
    navDatas[satId] = tmp;
    (totalNavDataCount[satId])++;
}

/*
 * navDatas must be a satPerType size array of null pointers
 * headers must be the address of a null pointer
 *
 * return value is 1 if parsing was successful and 0 if it was not.
 */
int parseGPSNavigationFile(GPSNavigationData** navDatas, int* totalNavDataCount, GPSNavigationHeader** header, char* navFilePath)
{
    char funcName[] = "parseGPSNavigationFile()";
    int i;
    for (i = 0; i < satPerType; i++)
    {
        if (navDatas[i])
        {
            printf("%s: GPS navigation data pointer with prn %d in navDatas is not null\n", funcName, i);
            return 0;
        }
    }
    if (*header)
    {
         printf("%s: GPS navigation header pointer is not null\n", funcName);
         return 0;
    }
    if (!navFilePath)
    {
        printf("%s: Null pointer passed as navFilePath\n", funcName);
        return 0;
    }
    i = 0;
    while (navFilePath[i])
    {
        i++;
    }
    if (navFilePath[i-1] != 'N' && navFilePath[i-1] != 'n')
    {
        printf("%s: File %s is not a GPS navigation file (does not end with 'N')\n", funcName, navFilePath);
        return 0;
    }

    FILE* navFile = fopen(navFilePath, "r");
    if (!navFile)
    {
        printf("%s: Could not open GPS navigation file %s\n", funcName, navFilePath);
        return 0;
    }

    int headerSection = 1;
    *header = malloc(sizeof(GPSNavigationHeader));
    initGPSNavigationHeader(*header);
    char line[82] = {0};
    //29 char
    char commentTime[] = "Initial comment              ";
    int lineNum = 0;
    while (!feof(navFile))
    {
        //header information is coming
        if (headerSection)
        {
            memset(line, 0, 82);
            fgets(line, 82, navFile);
            lineNum++;
            if (checkEmptyLine(line))
            {
                continue;
            }
            headerSection = parseGPSNavigationHeader(line, *header, commentTime);
            if (headerSection == -1)
            {
                return 0;
            }
        }
        //nav data epoch
        else
        {
            GPSNavigationData currentNavigation;
            initGPSNavigationData(&currentNavigation);
            currentNavigation.header = *header;
            PreciseTime currentEpoch;
            int currentSatelliteNum = 0;
            int currentRecordPos = sizeof(GPSNavigationHeader*) + sizeof(PreciseTime);
            int i;
            int validData = 1;
            for (i = 0; i < 8; i++)
            {
                if (!feof(navFile))
                {
                    memset(line, 0, 82);
                    fgets(line, 82, navFile);
                    lineNum++;
                    if (replaceDoubleExponent(line))
                    {
                        printf("%s: error while replaceing D-s with E-s in line %d\n", funcName, lineNum);
                        return 0;
                    }
                }
                int j = 0;
                int k = 4;
                if (i == 0)
                {
                    if (checkEmptyLine(line))
                    {
                        validData = 0;
                        break;
                    }
                    struct tm time;
                    memset(&time, 0, sizeof(struct tm));
                    int year, month;
                    sscanf(line, "%d%d%d%d%d%d%d", &currentSatelliteNum,
                                                   &year,
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
                    sscanf(&(line[21]), "%d", &(currentEpoch.nanos));
                    currentEpoch.nanos *= 100;
                    memcpy(&(currentNavigation.epoch), &currentEpoch, sizeof(PreciseTime));
                    j = 1;
                    k = 4;
                }
                else if(i == 7)
                {
                    j = 0;
                    k = 1;
                }
                else
                {
                    j = 0;
                    k = 4;
                }

                for (; j < k; j++)
                {
                    sscanf(&(line[3 + j * 19]), "%lf", (double*)(((char*)&currentNavigation) + currentRecordPos));
                    currentRecordPos += sizeof(double);
                }
            }
            if (validData)
            {
                insertGPSNavigationData(navDatas, &currentNavigation, totalNavDataCount, currentSatelliteNum);
            }
        }
    }
    fclose(navFile);
    return 1;
}
