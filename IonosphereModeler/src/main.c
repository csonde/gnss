#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <argp.h>
#include <dirent.h>
#include <gsl/gsl_linalg.h>
#include <gsl/gsl_blas.h>
#include <almanac.h>
#include <containers.h>
#include <observationParser.h>
#include <rinexCommon.h>
#include <ionosphereGrid.h>
#include <matrixOperation.h>

static char doc[] = "Ionosphere modeler program";

static char args_doc[] = "-r RINEXDIR -c RECCOORDFILE -d DCBDIR -a ALMANAC -s STARTTIME -e ENDTIME -i INTERVAL";

static struct argp_option options[] =
{
    {"rinexdir",  'r', "RINEXDIR",     0, "The directory containing the rinex files."},
    {"coord",     'c', "RECCOORDFILE", 0, "The file containing the receiver coordinates."},
    {"dcb",       'd', "DCBDIR",       0, "The directory containing the dcb data."},
    {"almanac",   'a', "ALMANAC",      0, "The file containing the almanac data."},
    {"starttime", 's', "STARTTIME",    0, "The time from when the measurement will be processed in gps seconds."},
    {"endtime",   'e', "ENDTIME",      0, "The time until the measurement will be processed in gps seconds."},
    {"interval",  'i', "INTERVAL",     0, "The interval of the sampling of the measurements."}
};

struct arguments
{
    long startTime;
    long endTime;
    long interval;
    char* rinexDir;
    char* recCoordFile;
    char* dcbDir;
    char* almanacFile;
};

static error_t parse_opt (int key, char *arg, struct argp_state *state)
{
    struct arguments *arguments = state->input;
    switch (key)
    {
        case 'r':
            arguments->rinexDir = arg;
            break;
        case 'c':
            arguments->recCoordFile = arg;
            break;
        case 'd':
            arguments->dcbDir = arg;
            break;
        case 'a':
            arguments->almanacFile = arg;
        case 's':
            arguments->startTime = atol(arg);
            break;
        case 'e':
            arguments->endTime = atol(arg);
            break;
        case 'i':
            arguments->interval = atol(arg);
            break;

        case ARGP_KEY_ARG:
            argp_usage (state);
            break;

        default:
           return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

static struct argp argp = { options, parse_opt, args_doc, doc };

typedef struct recDCB
{
    char recId[5];
    double dcb;
}recDCB;

int main(int argc, char *argv[])
{
    satTypeNum = 1;
    satPerType = 33; //1-32, there is no sat with prn 0
    //parse arguments
    struct arguments arguments;

    arguments.startTime = 0;
    arguments.endTime = 0;
    arguments.interval = 0;
    arguments.rinexDir = "-";
    arguments.recCoordFile = "-";
    arguments.dcbDir = "-";
    arguments.almanacFile = "-";

    argp_parse (&argp, argc, argv, 0, 0, &arguments);

    if(arguments.endTime < arguments.startTime)
    {
        printf("startime cannot be larger than endtime, exiting...\n");
        exit(-1);
    }
    //Read station coordinates into a binary tree keyed by 4 letter station ID
    char line[100] = {0};
    FILE* statCoordFile = fopen(arguments.recCoordFile, "r");
    if(!statCoordFile)
    {
        printf("could not open receiver coordinate file %s\n", arguments.recCoordFile);
        exit(-1);
    }
    StationCoord* stationCoordsRoot = 0;
    int i;
    for (i = 0; i < 7; i++)
    {
        fgets(line, 100, statCoordFile);
    }
    while (!feof(statCoordFile))
    {
        fgets(line, 100, statCoordFile);
        if(!strcmp(line, "\n"))
        {
            continue;
        }
        int stationNum;
        char stationID[5] = {0};
        double stationX;
        double stationY;
        double stationZ;
        char flag;
        sscanf(line, "%d%s%lf%lf%lf", &stationNum, stationID, &stationX, &stationY, &stationZ, &flag);
        Vector statVector = createVector(stationX, stationY, stationZ);
        insertStationCoord(stationID, statVector, &stationCoordsRoot);
        memset(line, 0, 100);
    }
    fclose(statCoordFile);

    /* 
     * Read measurements to
     */

    //Fetch rinex files from rinex directory
    DIR* rinexDir = opendir(arguments.rinexDir);

    struct dirent* fileEntry = 0;
    Measurement* measListRoot = 0;
    int measCount = 0;
    while (fileEntry = readdir(rinexDir))
    {
        if (fileEntry->d_type == DT_REG)
        {
            char rinexFileName[1024] = {0};
            strcat(rinexFileName, arguments.rinexDir);
            strcat(rinexFileName, "/");
            strcat(rinexFileName, fileEntry->d_name);

            //parse current rinex file
            GNSSObservation** observations = malloc(sizeof(GNSSObservation*) * (satTypeNum * satPerType));
            memset(observations, 0, sizeof(GNSSObservation*) * satTypeNum * satPerType);
            int* totalObservationCount = malloc(sizeof(int) * (satTypeNum * satPerType));
            memset(totalObservationCount, 0, sizeof(int) * (satTypeNum * satPerType));
            GNSSObservationHeader* headers = 0;
            int headerCount = 0;
            printf("Start parsing file %s\n", rinexFileName);
            if (!parseGNSSObservationFile(observations, totalObservationCount, &headers, rinexFileName, &headerCount))
            {
                continue;
            }

            //read the actual measurements
            //satellites are from 1 to 32, because of parseGNSSObservationFile implementation
            for (i = 1; i < satTypeNum * satPerType; i++)
            {
                //Converting command args to UTC (or perhaps it should be required in utc)
                long startTimeUTC = gpstToUTC(arguments.startTime);
                long endTimeUTC = gpstToUTC(arguments.endTime);
                int j;
                int k = 0;
                //set proper start time and end time
                if(totalObservationCount[i] > 0)
                {
                    if(observations[i][0].epoch.seconds >= startTimeUTC && observations[i][0].epoch.seconds <= endTimeUTC)
                    {
                        startTimeUTC = observations[i][0].epoch.seconds;
                    }
                    else if (observations[i][0].epoch.seconds > endTimeUTC)
                    {
                        continue;
                    }
                    if(observations[i][totalObservationCount[i] - 1].epoch.seconds <= endTimeUTC &&
                       observations[i][totalObservationCount[i] - 1].epoch.seconds >= startTimeUTC)
                    {
                        endTimeUTC = observations[i][totalObservationCount[i] - 1].epoch.seconds;
                    }
                    else if (observations[i][totalObservationCount[i] - 1].epoch.seconds < startTimeUTC)
                    {
                        continue;
                    }
                }
                else
                {
                    continue;
                }
                //cycle through measurements, if time stamp is before the next interval, we simply skip it
                for (j = 0; j < totalObservationCount[i] && startTimeUTC + k * arguments.interval <= endTimeUTC; j++)
                {
                    if(observations[i][j].epoch.seconds >= startTimeUTC + k * arguments.interval &&
                       observations[i][j].epoch.seconds <= endTimeUTC)
                    {
                        if ((utcToGPST(observations[i][j].epoch.seconds) % 604800) - 16 == 579510)
                        {
                            assert(0);
                        }
                        Measurement* meas = malloc(sizeof(Measurement));
                        initMeasurement(meas);
                        meas->gpsTime = utcToGPST(observations[i][j].epoch.seconds);
                        meas->satId = i - 1;
                        strncpy(meas->recId, fileEntry->d_name, 4);
                        //fetch C1 and P2 measurements
                        int l;
                        for(l = 0; l < observations[i][j].header->obsTypeNumber; l++)
                        {
                            if ((observations[i][j].header->observationCodes)[l] == 'C' &&
                               (observations[i][j].header->frequencyCodes)[l] == 1)
                            {
                                meas->C1 = observations[i][j].observations[l];
                            }
                            else if ((observations[i][j].header->observationCodes)[l] == 'P' &&
                                    (observations[i][j].header->frequencyCodes)[l] == 2)
                            {
                                meas->P2 = observations[i][j].observations[l];
                            }
                        }
                        k++;
                        //error if there was no C1 or P2
                        //satellite does not contain relevant measurements, discarding it
                        if(!meas->C1 || !meas->P2)
                        {
                            free(meas);
                            continue;
                        }
                        insertMeasurementToListEnd(meas, &measListRoot);
                        measCount++;
                    }
                }
            }
            for (i = 0; i < satTypeNum * satPerType; i++)
            {
                int j;
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
            memset(rinexFileName, 0, 1024);
        }
    }
    closedir(rinexDir);

    Measurement* tmp = measListRoot;
    GPSSatCoords* gpsSatCoordsTreeRoot = 0;

    //Calculate gps sat coords for all relevant epochs
    int insertStep = 1;
    while(tmp)
    {
        printf("insertGPSSatCoords step %d\n", insertStep);
        insertStep++;
        insertGPSSatCoords(tmp->gpsTime, arguments.almanacFile, &gpsSatCoordsTreeRoot);
        tmp = tmp->next;
    }
    printf("GPS satellite coordinates are calculated.\n");

    //Create the grid
    GeoCoord center = createGeoCoord(50, 15, 1);
    QuadraticGrid* grid = malloc(sizeof(QuadraticGrid));
    /* center, Lat size, Lon size, Num Layers, Lat number, Lon number */
    *grid = createQuadraticGrid(center, 5, 5, 2, 3, 3);
    printf("Ionospehere grid created.\n");

    //Calculate line sectors
    tmp = measListRoot;
    while(tmp)
    {
        printf("calculating linesectors for sat: %d,    rec: %s,    at gps time: %ld\n", tmp->satId+1, tmp->recId, tmp->gpsTime);
        calculateLineSectors(tmp, gpsSatCoordsTreeRoot, stationCoordsRoot, grid);
        if(!tmp->lineSectors)
        {
            printf(" calculation was unsuccesful\n");
        }
        tmp = tmp->next;
    }

    //Remove measurements without valid crossing over the model
    tmp = measListRoot;
    Measurement* prev = 0;
    while (tmp)
    {
        if (tmp == measListRoot)
        {
            if (!tmp->lineSectors)
            {
                Measurement* next = tmp->next;
                free(tmp);
                tmp = measListRoot = next;
            }
            else
            {
                prev = tmp;
                tmp = tmp->next;
            }
            continue;
        }
        else
        {
            if (!tmp->lineSectors)
            {
                Measurement* next = tmp->next;
                free(tmp);
                prev->next = next;
                tmp = next;
            }
            else
            {
                prev = tmp;
                tmp = tmp->next;
            }
        }
    }


    //Alakmatrix
    printf("Creating alakmatrix\n");
    IonoVariables* vars = 0;
    gsl_matrix* alakMatrix = createAlakMatrix(&vars, measListRoot, grid);
    printf("Alakmatrix created\n");

    printf("cellCount: %d,    stationCount: %d,    satCount: %d\n", vars->cellCount, vars->stationCount, vars->satCount);

    printCellParameterPreorder(vars->cellParameters);
    printStationDCBPreorder(vars->stationDCBRoot);

    int paramCount = vars->cellCount;// + vars->stationCount;
    int corrCount = vars->corrCount;

    printf("Number of measurements: %d\n", corrCount);
    printf("Number of paramaeters: %d\n", paramCount);
    if(paramCount > corrCount)
    {
        printf("There are more parameters than measurements, cannot solve problem.");
        exit(-1);
    }

    //Read DCB values from dcb file
    double p1p2dcb[32];
    recDCB p1p2recdcb[1024] = {0};

    char p1p2Path[1024] = {0};
    strcpy(p1p2Path, arguments.dcbDir);
    strcat(p1p2Path, "/BMC17356.DCB");

    memset(line, 0, 100);
    FILE* p1p2File = fopen(p1p2Path, "r");
    if(!p1p2File)
    {
        printf("could not open p1p2 dcb file %s\n", p1p2Path);
        exit(-1);
    }

    for (i = 0; i < 7; i++)
    {
        fgets(line, 100, p1p2File);
        memset(line, 0, 100);
    }
    while (!feof(p1p2File))
    {
        fgets(line, 100, p1p2File);
        if(!strcmp(line, "\n"))
        {
            continue;
        }
        char satId[4] = {0};
        char recId[5] = {0};
        char lineEnd[100] = {0};
        double satDCB;
        double rms;
        sscanf(line, "%s", satId);
        strcpy(lineEnd, &(line[6]));
        sscanf(lineEnd, "%s", recId);
        strcpy(lineEnd, &(line[26]));
        sscanf(lineEnd, "%lf%lf", &satDCB, &rms);
        if (satId[0] == 'G')
        {
            if(satId[1] >= '0' && satId[1] <= '9' && satId[2] >= '0' && satId[2] <= '9')
            {
                int satIdNum = (satId[1]-'0') * 10 + satId[2]-'0';
                p1p2dcb[satIdNum] = satDCB;
            }
            else
            {
                int j;
                for(j = 0; j < 1024; j++)
                {
                    if(p1p2recdcb[j].recId[0] == 0)
                    {
                        strcpy(p1p2recdcb[j].recId, recId);
                        p1p2recdcb[j].dcb = satDCB;
                        break;
                    }
                }
            }
        }
        memset(line, 0, 100);
    }
    fclose(p1p2File);

    //Calculate dcb length from time
    for (i = 0; i < 32; i++)
    {
        double* dcb = getSatDCB(i, vars->satDCBRoot);
        if(dcb)
        {
            *dcb = (-p1p2dcb[i]) * 0.299792458;
        }
    }

    for (i = 0; i < 1024; i++)
    {
        if(p1p2recdcb[i].recId[0] != 0)
        {
            double* dcb = getStationDCB(p1p2recdcb[i].recId, vars->stationDCBRoot);
            if(dcb)
            {
                *dcb = -p1p2recdcb[i].dcb * 0.299792458;
            }
        }
    }

    gsl_matrix* sulyMatrix = gsl_matrix_alloc(corrCount, corrCount);
    gsl_matrix_set_identity(sulyMatrix);

    gsl_vector* meresVektor = gsl_vector_alloc(corrCount);
    gsl_vector* meresJavVektor = gsl_vector_alloc(corrCount);
    gsl_vector* allandok = gsl_vector_alloc(corrCount);

    gsl_vector* paramStartVektor = gsl_vector_alloc(paramCount);
    gsl_vector* paramJavVektor = gsl_vector_alloc(paramCount);

    for(i = 0; i < corrCount; i++)
    {
        gsl_vector_set(meresJavVektor, i, 0);
        gsl_vector_set(allandok, i, 0);
    }

    tmp = measListRoot;
    i = 0;
    while(tmp)
    {
        gsl_vector_set(meresVektor, i, tmp->C1 - tmp->P2);
        gsl_vector_set(allandok, i, *(getSatDCB(tmp->satId, vars->satDCBRoot)) + *(getStationDCB(tmp->recId, vars->stationDCBRoot)));
        i++;
        tmp = tmp->next;
    }

    for(i = 0; i < paramCount; i++)
    {
        gsl_vector_set(paramStartVektor, i, 0);
        gsl_vector_set(paramJavVektor, i, 0);
    }

    FILE* matrixFile1 = fopen("./alakMatrix", "w");
    for(i = 0; i < corrCount; i++)
    {
        int j;
        for(j = 0; j < paramCount; j++)
        {
            printf("A(%d,%d): %le\n", i, j, gsl_matrix_get(alakMatrix, i, j));
            fprintf(matrixFile1, "%le", gsl_matrix_get(alakMatrix, i, j));
            if(j == paramCount-1)
            {
                fprintf(matrixFile1, "\n");
            }
            else
            {
                fprintf(matrixFile1, " ");
            }
        }
    }
    fclose(matrixFile1);

    FILE* matrixFile2 = fopen("./paramStartVektor", "w");
    FILE* matrixFile3 = fopen("./paramJavVektor", "w");
    for(i = 0; i < paramCount; i++)
    {
        fprintf(matrixFile2, "%le\n", gsl_vector_get(paramStartVektor, i));
        fprintf(matrixFile3, "%le\n", gsl_vector_get(paramJavVektor, i));
        if(i != paramCount-1)
        {
            fprintf(matrixFile2, "\n");
            fprintf(matrixFile3, "\n");
        }
    }
    fclose(matrixFile2);
    fclose(matrixFile3);

    FILE* matrixFile4 = fopen("./meresVektor", "w");
    FILE* matrixFile5 = fopen("./meresJavVektor", "w");
    FILE* matrixFile6 = fopen("./allandok", "w");
    for(i = 0; i < corrCount; i++)
    {
        fprintf(matrixFile4, "%le\n", gsl_vector_get(meresVektor, i));
        fprintf(matrixFile5, "%le\n", gsl_vector_get(meresJavVektor, i));
        fprintf(matrixFile6, "%le\n", gsl_vector_get(allandok, i));
        if(i != corrCount-1)
        {
            fprintf(matrixFile4, "\n");
            fprintf(matrixFile5, "\n");
            fprintf(matrixFile6, "\n");
        }
    }
    fclose(matrixFile4);
    fclose(matrixFile5);
    fclose(matrixFile6);
/*
    kiegyenlites(alakMatrix, sulyMatrix,
                 paramStartVektor, paramJavVektor,
                 meresVektor, meresJavVektor,
                 allandok, 10);

    for(i = 0; i < paramCount; i++)
    {
        printf("Parameter %d: %le\n", i, gsl_vector_get(paramStartVektor, i));
    }

    for(i = 0; i < corrCount; i++)
    {
        printf("meres Javitas %d: %lf\n", i, gsl_vector_get(meresJavVektor, i));
    }*/


    tmp = measListRoot;
    while(tmp)
    {
        printf("gpstime: %ld,    satID: %d,    stationID: %s,    C1: %lf,    P2: %lf\n", tmp->gpsTime,
                                                                                         tmp->satId,
                                                                                         tmp->recId,
                                                                                         tmp->C1,
                                                                                         tmp->P2);
        LineSectorList *lsl = tmp->lineSectors;
        int dzs = 0;
        while(lsl)
        {
            printf("    Linesector %d:    cell: (%d, %d, %d)    length: %lf\n", dzs,
                                                                                lsl->layerId,
                                                                                lsl->lateralId,
                                                                                lsl->longitudinalId,
                                                                                lsl->length);
            lsl = lsl->next;
            dzs++;
        }
        tmp = tmp->next;
    }

//    insertGPSSatCoords(1049847916, arguments.almanacFile, &gpsSatCoordsTreeRoot);

/*    Measurement testmeas;
    testmeas.gpsTime = 1049847916;
    testmeas.satId = 2;
    memcpy(&(testmeas.recId), "GENO", 5);
    testmeas.C1 = 21155244.152;
    testmeas.P2 = 21155249.400;
    testmeas.lineSectors = 0;
    testmeas.next = 0;
    calculateLineSectors(&testmeas, gpsSatCoordsTreeRoot, stationCoordsRoot, grid);

    printf("Test meas line sectors:\n");
    LineSectorList *testlsl = testmeas.lineSectors;
    int dzs = 0;
    while(testlsl)
    {
        printf("    Linesector %d:    cell: (%d, %d, %d)    length: %lf\n", dzs,
                                                                            testlsl->layerId,
                                                                            testlsl->lateralId,
                                                                            testlsl->longitudinalId,
                                                                            testlsl->length);
        printf("    single cell id: %d\n", getSingleCellIDByIndexes(testlsl->layerId, testlsl->lateralId, testlsl->longitudinalId, grid));
        testlsl = testlsl->next;
        dzs++;
    }*/

    //TODO:
    //delete statCoords
    //delete gpsSatCoordsTreeRoot
    //delete measListRoot
    //delete grid
    //delete matrixes and vectors
    return 0;
}
