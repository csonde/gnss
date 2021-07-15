#include "observationParser.h"
#include "gpsNavigationParser.h"
#include "glonassNavigationParser.h"
#include "meteorologicalParser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int main()
{
    setbuf(stdout, 0);
    GNSSObservation** observations = malloc(sizeof(GNSSObservation*) * (satTypeNum * satPerType));
    memset(observations, 0, sizeof(GNSSObservation*) * satTypeNum * satPerType);
    int* totalObservationCount = malloc(sizeof(int) * (satTypeNum * satPerType));
    memset(totalObservationCount, 0, sizeof(int) * (satTypeNum * satPerType));
    GNSSObservationHeader* headers = 0;
    int headerCount = 0;
    //char path[] = "./bolg1810.13o";
    //char path[] = "./aubg200a.13o";
    char path[] = "./ACOR1030.13O";
    parseGNSSObservationFile(observations, totalObservationCount, &headers, path, &headerCount);
    GNSSObservationHeader* tmp = headers;
    while(tmp)
    {
        printGNSSObservationHeader(tmp);
        tmp = tmp->nextHeader;
        printf("\n");
    }
    printGNSSObservations(observations, satTypeNum  * satPerType, totalObservationCount);
    int i, j;
    for (i = 0; i < satTypeNum * satPerType; i++)
    {
        for (j = 0; j < totalObservationCount[i]; j++)
        {
            deleteGNSSObservation(&(observations[i][j]));
        }
        if(observations[i])
        {
            free(observations[i]);
        }
    }
    free(observations);
    free(totalObservationCount);
    if(headers)
    {
        deleteGNSSObservationHeader(headers);
        free(headers);
    }
/*
    printf("\n\n\n\n\nGPS Navigation data\n\n\n\n\n");

    GPSNavigationData* navDatas[satPerType];
    int totalNavDataCount[satPerType] = {0};
    memset(navDatas, 0, sizeof(GPSNavigationData*) * satPerType);
    GPSNavigationHeader* navHeader = 0;
    char navPath[] = "./acor2130.13n";
    parseGPSNavigationFile(navDatas, totalNavDataCount, &navHeader, navPath);
    printGPSNavigationHeader(navHeader);
    printGPSNavigationDatas(navDatas, satPerType, totalNavDataCount);
    for (i = 0; i < satPerType; i++)
    {
        if(navDatas[i])
        {
            free(navDatas[i]);
        }
    }
    if (navHeader)
    {
        deleteGPSNavigationHeader(navHeader);
        free(navHeader);
    }

    printf("\n\n\n\n\nGlonass Navigation data\n\n\n\n\n");

    GlonassNavigationData* glonassNavDatas[satPerType];
    int totalGlonassNavDataCount[satPerType] = {0};
    memset(glonassNavDatas, 0, sizeof(GlonassNavigationData*) * satPerType);
    GlonassNavigationHeader* glonassNavHeader = 0;
    char glonassNavPath[] = "./caen2140.13g";
    parseGlonassNavigationFile(glonassNavDatas, totalGlonassNavDataCount, &glonassNavHeader, glonassNavPath);
    printGlonassNavigationHeader(glonassNavHeader);
    printGlonassNavigationDatas(glonassNavDatas, satPerType, totalGlonassNavDataCount);
    for (i = 0; i < satPerType; i++)
    {
        if(glonassNavDatas[i])
        {
            free(glonassNavDatas[i]);
        }
    }
    if (glonassNavHeader)
    {
        deleteGlonassNavigationHeader(glonassNavHeader);
        free(glonassNavHeader);
    }

    printf("\n\n\n\n\nMeteorological data\n\n\n\n\n");

    MeteorologicalData* metData = 0;
    int totalMetDataCount = 0;
    MeteorologicalHeader* metHeader = 0;
    char metPath[] = "./nvsk2210.13m";
    parseMeteorologicalFile(&metData, &totalMetDataCount, &metHeader, metPath);
    printf("%p\n", metHeader);
    printMeteorologicalHeader(metHeader);
    printMeteorologicalData(&metData, 1, &totalMetDataCount);
    i;
    if (metData)
    {
        for (i = 0; i < totalMetDataCount; i++)
        {
            deleteMeteorologicalData(&(metData[i]));
        }
        free(metData);
    }
    if (metHeader)
    {
        deleteMeteorologicalHeader(metHeader);
        free(metHeader);
    }*/
    return 1;
}