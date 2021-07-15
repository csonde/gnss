#include "glonassNavigationParser.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


void initGlonassNavigationHeader(GlonassNavigationHeader* header)
{
    memset(header, 0, sizeof(GlonassNavigationHeader));
}


void copyGlonassNavigationHeaderHeader(GlonassNavigationHeader** dest, GlonassNavigationHeader* header, int allocNewMem)
{
    GlonassNavigationHeader* newHeader = *dest;
    if (allocNewMem || !newHeader)
    {
        newHeader = malloc(sizeof(GlonassNavigationHeader));
    }
    memcpy(newHeader, header, sizeof(GlonassNavigationHeader));
    if(header->comment)
    {
        newHeader->comment = malloc(sizeof(char) * header->commentLength);
        memcpy(newHeader->comment, header->comment, sizeof(char) * header->commentLength);
    }
}


void deleteGlonassNavigationHeader(GlonassNavigationHeader* header)
{
    if (header->comment != 0)
    {
        free(header->comment);
    }
}


void printGlonassNavigationHeader(GlonassNavigationHeader* header)
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
    printf("Correction to system time:\n");
    printf("Time of referece: %d %d %d,    correction: %lf\n", header->refYear, header->refMonth, header->refDay, header->timeCorrection);
    printf("Leap seconds: %d\n", header-> leapSeconds);
}



void initGlonassNavigationData(GlonassNavigationData* navData)
{
    memset(navData, 0, sizeof(GlonassNavigationData));
}


void copyGlonassNavigationData(GlonassNavigationData** dst, GlonassNavigationData* source, int allocNewMem)
{
    GlonassNavigationData* dest = *dst;
    if(allocNewMem || !dest)
    {
        dest = malloc(sizeof(GlonassNavigationData));
    }
    memcpy(dest, source, sizeof(GlonassNavigationData));
}


void printGlonassNavigationDatas(GlonassNavigationData** navDatas, int satNum, int* totalNavDataCount)
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
                printf("svClockBias: %le,    svRelFrqBias: %le,    msgFrameTime: %le\n", navDatas[i][j].svClockBias,
                                                                                         navDatas[i][j].svRelFrqBias,
                                                                                         navDatas[i][j].msgFrameTime);
                printf("satPositionX: %le,    satVelocityX: %le,    satAccelerationX: %le,    health: %le\n", navDatas[i][j].satPositionX,
                                                                                                              navDatas[i][j].satVelocityX,
                                                                                                              navDatas[i][j].satAccelerationX,
                                                                                                              navDatas[i][j].health);
                printf("satPositionY: %le,    satVelocityY: %le,    satAccelerationY: %le,    frequencyNumber: %le\n", navDatas[i][j].satPositionY,
                                                                                                                       navDatas[i][j].satVelocityY,
                                                                                                                       navDatas[i][j].satAccelerationY,
                                                                                                                       navDatas[i][j].frequencyNumber);
                printf("satPositionZ: %le,    satVelocityZ: %le,    satAccelerationZ: %le,    operInfAge: %le\n", navDatas[i][j].satPositionZ,
                                                                                                                  navDatas[i][j].satVelocityZ,
                                                                                                                  navDatas[i][j].satAccelerationZ,
                                                                                                                  navDatas[i][j].operInfAge);
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
int parseGlonassNavigationHeader(char* line, GlonassNavigationHeader* header, char* commentTime)
{

    char funcName[] = "parseGlonassNAvigationHeader()";
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
        if (line[20] != 'G' && line[20] != 'g')
        {
            printf("%s: bad file type, required: 'G' or 'g', got: %c\n", funcName, line[20]);
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
    else if(strstr(recordName, "CORR TO SYSTEM TIME"))
    {
        sscanf(line, "%d%d%d %le", &(header->refYear), &(header->refMonth), &(header->refDay), &(header->timeCorrection));
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



void insertGlonassNavigationData(GlonassNavigationData** navDatas, GlonassNavigationData* navData, int* totalNavDataCount, int satId)
{
    GlonassNavigationData* tmp = malloc(sizeof(GlonassNavigationData) * (totalNavDataCount[satId] + 1));
    GlonassNavigationData* ptr = 0;
    int i;
    int posFound = 0;
    for (i = 0; i < totalNavDataCount[satId]; i++)
    {
        if(comparePreciseTime(&(navDatas[satId][i].
                                epoch), &(navData->epoch)) <= 0)
        {
            ptr = &(tmp[i]);
            copyGlonassNavigationData(&ptr, &(navDatas[satId][i]), 0);
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
    copyGlonassNavigationData(&ptr, navData, 0);
    for (;i < totalNavDataCount[satId]; i++)
    {
        ptr = &(tmp[i + 1]);
        copyGlonassNavigationData(&ptr, &(navDatas[satId][i]), 0);
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
int parseGlonassNavigationFile(GlonassNavigationData** navDatas, int* totalNavDataCount, GlonassNavigationHeader** header, char* navFilePath)
{
    char funcName[] = "parseGlonassNavigationFile()";
    int i;
    for (i = 0; i < satPerType; i++)
    {
        if (navDatas[i])
        {
            printf("%s: Glonass navigation data pointer with prn %d in navDatas is not null\n", funcName, i);
            return 0;
        }
    }
    if (*header)
    {
         printf("%s: Glonass navigation header pointer is not null\n", funcName);
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
    if (navFilePath[i-1] != 'G' && navFilePath[i-1] != 'g')
    {
        printf("%s: File %s is not a Glonass navigation file (does not end with 'G')\n", funcName, navFilePath);
        return 0;
    }

    FILE* navFile = fopen(navFilePath, "r");
    if (!navFile)
    {
        printf("%s: Could not open Glonass navigation file %s\n", funcName, navFilePath);
        return 0;
    }

    int headerSection = 1;
    *header = malloc(sizeof(GlonassNavigationHeader));
    initGlonassNavigationHeader(*header);
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
            headerSection = parseGlonassNavigationHeader(line, *header, commentTime);
            if (headerSection == -1)
            {
                return 0;
            }
        }
        //nav data epoch
        else
        {
            GlonassNavigationData currentNavigation;
            initGlonassNavigationData(&currentNavigation);
            currentNavigation.header = *header;
            PreciseTime currentEpoch;
            int currentSatelliteNum = 0;
            int currentRecordPos = sizeof(GlonassNavigationHeader*) + sizeof(PreciseTime);
            int i;
            int validData = 1;
            for (i = 0; i < 4; i++)
            {
                if (!feof(navFile))
                {
                    memset(line, 0, 82);
                    fgets(line, 82, navFile);
                    lineNum++;
                    if(replaceDoubleExponent(line))
                    {
                        printf("%s: error while replaceing D-s with E-s in line %d\n", funcName, lineNum);
                        return 0;
                    }
                }
                else
                {
                    break;
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
                insertGlonassNavigationData(navDatas, &currentNavigation, totalNavDataCount, currentSatelliteNum);
            }
        }
    }
    fclose(navFile);
    return 1;
}
