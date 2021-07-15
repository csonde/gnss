#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <argp.h>
#include <dirent.h>
extern "C"
{
#include <observationParser.h>
#include <rinexCommon.h>
}
#include "almanac.h"
#include "bigNumMatrix.h"
#include "equationSystemSolver.h"
#include "paramStart.h"
#include "pi.h"

#include <iostream>
#include <fstream>
#include <iomanip>


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
    struct arguments *arguments = (struct arguments*)(state->input);
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

    /* 
     * Read measurements to
     */

    //Fetch rinex files from rinex directory
    DIR* rinexDir = opendir(arguments.rinexDir);

    struct dirent* fileEntry = 0;
    measurement_vector_t measurements;

    int i;

    while ((fileEntry = readdir(rinexDir)))
    {
        if (fileEntry->d_type == DT_REG)
        {
            char rinexFileName[1024] = {0};
            strcat(rinexFileName, arguments.rinexDir);
            strcat(rinexFileName, "/");
            strcat(rinexFileName, fileEntry->d_name);

            //parse current rinex file
            GNSSObservation** observations = (GNSSObservation**)(malloc(sizeof(GNSSObservation*) * (satTypeNum * satPerType)));
            memset(observations, 0, sizeof(GNSSObservation*) * satTypeNum * satPerType);
            int* totalObservationCount = (int*)(malloc(sizeof(int) * (satTypeNum * satPerType)));
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
                        /*if ((utcToGPST(observations[i][j].epoch.seconds) % 604800) - 16 == 579510)
                        {
                            assert(0);
                        }*/
                        Measurement meas;
                        meas.gpsTime = utcToGPST(observations[i][j].epoch.seconds);
                        meas.satId = i - 1;
                        meas.recId = fileEntry->d_name;
                        meas.recId.resize(4);
                        int signalStrength = 0;
                        //fetch C1 and P2 measurements
                        int l;
                        for(l = 0; l < observations[i][j].header->obsTypeNumber; l++)
                        {
                            signalStrength = observations[i][j].signalStrength[l];
                            if ((observations[i][j].header->observationCodes)[l] == 'C' &&
                               (observations[i][j].header->frequencyCodes)[l] == 1)
                            {
                                meas.C1 = observations[i][j].observations[l];
                            }
                            else if ((observations[i][j].header->observationCodes)[l] == 'P' &&
                                    (observations[i][j].header->frequencyCodes)[l] == 2)
                            {
                                meas.P2 = observations[i][j].observations[l];
                            }
                        }
                        k++;
                        //error if there was no C1 or P2
                        //satellite does not contain relevant measurements, discarding it
                        if(!meas.C1 || !meas.P2 || signalStrength < 6)
                        {
                            continue;
                        }
                        measurements.push_back(meas);
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

    filebuf fb;
    fb.open ("./measurements", ios::out);
    ostream os(&fb);
    for (measurement_vector_t::iterator it = measurements.begin(); it != measurements.end(); it++)
    {
        os << it->recId << "    " << it->satId << "    " << it->gpsTime << "    "<<it->C1 - it->P2 << endl;
    }
    fb.close();

    printf("Filtering measurements\n");
    measurement_vector_t filteredMeasurements = measurements;
    bool filterNeeded = true;
    unsigned int iterationNumber = 1;

    while (filterNeeded || filteredMeasurements.size() == 0)
    {
        cout << "filtering, iteration " << iterationNumber << endl;
        double expected = 0;
        measurement_vector_t::iterator minIt = filteredMeasurements.begin();
        measurement_vector_t::iterator maxIt = filteredMeasurements.begin();
        for (measurement_vector_t::iterator it = filteredMeasurements.begin(); it != filteredMeasurements.end(); it++)
        {
            if (minIt->C1 - minIt->P2 > it->C1 - it->P2)
            {
                minIt = it;
            }
            else if (maxIt->C1 - maxIt->P2 < it->C1 - it->P2)
            {
                maxIt = it;
            }
            expected += it->C1 - it->P2;
        }
        expected /= filteredMeasurements.size();
        cout << "    expected: " << expected << endl;

        double dispersion = 0;
        for (measurement_vector_t::iterator it = filteredMeasurements.begin(); it != filteredMeasurements.end(); it++)
        {
            dispersion += pow((it->C1 - it->P2 - expected), 2);
        }
        dispersion /= (filteredMeasurements.size() - 1);
        dispersion = sqrt(dispersion);
        cout << "    dispersion: " << dispersion << endl;

        double qmin = (expected - minIt->C1 + minIt->P2) / (dispersion * sqrt(((double)filteredMeasurements.size() - 1) / (double)filteredMeasurements.size()));
        double qmax = (maxIt->C1 - maxIt->P2 - expected) / (dispersion * sqrt(((double)filteredMeasurements.size() - 1) / (double)filteredMeasurements.size()));
        cout << "    qmin: " << qmin << endl;
        cout << "    qmax: " << qmax << endl;
        if (qmax > qmin && qmax > 2.717)
        {
            cout << "    removing " << maxIt->C1 - maxIt->P2 << endl;
            filteredMeasurements.erase(maxIt);
            continue;
        }
        if (qmin > qmax && qmin > 2.717)
        {
            cout << "    removing " << minIt->C1 - minIt->P2 << endl;
            filteredMeasurements.erase(minIt);
            continue;
        }
        filterNeeded = false;
    }

    fb.open ("./filteredMeasurements", ios::out);
    os.flush();
    os.rdbuf(&fb);
    for (measurement_vector_t::iterator it = filteredMeasurements.begin(); it != filteredMeasurements.end(); it++)
    {
        os << it->recId << "    " << it->satId << "    " << it->gpsTime << "    " <<it->C1 - it->P2 << endl;
    }
    fb.close();

    fb.open ("./filteredMeasOnly", ios::out);
    os.flush();
    os.rdbuf(&fb);
    for (measurement_vector_t::iterator it = filteredMeasurements.begin(); it != filteredMeasurements.end(); it++)
    {
        os << it->C1 - it->P2 << endl;
    }
    fb.close();

    //Read station coordinates
    char line[100] = {0};
    FILE* statCoordFile = fopen(arguments.recCoordFile, "r");
    if(!statCoordFile)
    {
        printf("could not open receiver coordinate file %s\n", arguments.recCoordFile);
        exit(-1);
    }

    rec_coord_t stationCoords;

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
        sscanf(line, "%d%s%lf%lf%lf%c", &stationNum, stationID, &stationX, &stationY, &stationZ, &flag);
        bool stationFound = false;
        for (measurement_vector_t::iterator it = filteredMeasurements.begin(); it != filteredMeasurements.end(); it++)
        {
            if (!it->recId.compare(stationID))
            {
                stationFound = true;
                break;
            }
        }
        if (!stationFound)
        {
            memset(line, 0, 100);
            continue;
        }
        GeographicalVector statVector(stationX, stationY, stationZ, Alakmatrix::a, Alakmatrix::e);
        stationCoords[stationID] = statVector;
        memset(line, 0, 100);
    }
    fclose(statCoordFile);


    //Calculate gps sat coords for all relevant epochs
    sat_coord_t gpsSatCoords;
    int insertStep = 1;
    for (measurement_vector_t::iterator it = filteredMeasurements.begin(); it != filteredMeasurements.end(); it++)
    {
        printf("insertGPSSatCoords step %d\n", insertStep);
        insertStep++;
        double t = calculateTimeOfGPSWeek(it->gpsTime);
        calculateGPSSatellitePositions(t, arguments.almanacFile, gpsSatCoords);
    }
    printf("GPS satellite coordinates are calculated.\n");
    //remove unnecessary gps coordinates
    for (sat_coord_t::iterator it1 = gpsSatCoords.begin(); it1 != gpsSatCoords.end();)
    {
        measurement_vector_t::iterator it2;
        for (it2 = filteredMeasurements.begin(); it2 != filteredMeasurements.end(); it2++)
        {
            if (it1->first == it2->satId)
            {
                it1++;
                break;
            }
        }
        if (it2 == filteredMeasurements.end())
        {
            sat_coord_t::iterator tmpIt = it1;
            it1++;
            gpsSatCoords.erase(tmpIt);
        }
    }

    //vertical profiles
    unsigned int verticalGranularity = 20;
    vertical_profile_t verticalProfiles;
    vector<unsigned int> heightData;
    line[0] = 0;
    FILE* heightDataFile = fopen("../../testData/heightData.txt", "r");

    for (i = 0; i < verticalGranularity; i++)
    {
        fgets(line, 100, heightDataFile);
        unsigned int height = 0;
        sscanf(line, "%u", &height);
        heightData.push_back(height);
    }
    fclose(heightDataFile);

    char line2[1024] = {0};
    unsigned int K = 5;
    FILE* profileFile = fopen("../../testData/principal_components.txt", "r");
    for (unsigned int i = 0; i < K; i++)
    {
        fgets(line2, 1024, profileFile);
        verticalProfiles.push_back(map<unsigned int, double>());
        unsigned int j = 0;
        char* token = strtok(line2, " ()\n");
        while (token)
        {
            double profileData = strtod(token, 0);
            verticalProfiles[i][heightData[j]] = profileData;
            //cout << profileData << endl;
            token = strtok(0, " ()\n");
            j++;
        }
    }

    fb.open ("./verticalProfiles", ios::out);
    os.flush();
    os.rdbuf(&fb);
    for (unsigned int k = 0; k < verticalProfiles.size(); k++)
    {
        for (map<unsigned int, double>::iterator it = verticalProfiles[k].begin(); it != verticalProfiles[k].end(); it++)
        {
            os << it->second << " ";
        }
        os << endl;
    }
    fb.close();

    //weights matrix
    BigNumMatrix weights(filteredMeasurements.size(), filteredMeasurements.size());
    for (int i = 0; i < filteredMeasurements.size(); i++)
    {
        Vector3d recSat = gpsSatCoords[filteredMeasurements[i].satId][calculateTimeOfGPSWeek(filteredMeasurements[i].gpsTime)] -
                                    stationCoords[filteredMeasurements[i].recId];
        Vector3d recVec = stationCoords[filteredMeasurements[i].recId];
        double cosZenit = (recSat[0] * recVec[0] + recSat[1] * recVec[1] + recSat[2] * recVec[2]) /
                          sqrt(recSat[0] * recSat[0] + recSat[1] * recSat[1] + recSat[2] * recSat[2]) /
                          sqrt(recVec[0] * recVec[0] + recVec[1] * recVec[1] + recVec[2] * recVec[2]);
        weights(i, i) = mpf_class(cosZenit*cosZenit, 128);
    }

    //Read DCB values from dcb file
    double p1p2dcb[32];
    map<string, double> p1p2recdcb;

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
                int satIdNum = (satId[1]-'0') * 10 + satId[2]-'0' - 1;
                p1p2dcb[satIdNum] = satDCB;
            }
            else
            {
                p1p2recdcb[recId] = satDCB;
            }
        }
        memset(line, 0, 100);
    }
    fclose(p1p2File);

    //Calculate dcb length from time

    for (i = 0; i < 32; i++)
    {
        p1p2dcb[i] = (-p1p2dcb[i]) * 0.299792458;
        /*{
            *dcb = (-p1p2dcb[i]) * 0.299792458;
        }*/
    }

    for (map<string,double>::iterator it = p1p2recdcb.begin(); it != p1p2recdcb.end(); it++)
    {
        it->second = (-(it->second)) * 0.299792458;
            /*{
                *dcb = -p1p2recdcb[i].dcb * 0.299792458;
            }*/
    }

    fb.open ("./dcbmeter", ios::out);
    os.flush();
    os.rdbuf(&fb);
    for (i = 0; i < 32; i++)
    {
        os << setprecision(16) << p1p2dcb[i] << endl;
    }
    for (map<string,double>::iterator it = p1p2recdcb.begin(); it != p1p2recdcb.end(); it++)
    {
        os << setprecision(16) << it->second << endl;
    }
    fb.close();


    unsigned int N = 9;

//======================================================
// Calculate parameter start values with least squares
//======================================================

    //Calculate parameter start values with least squares
    printf("Calculate parameter start values\n");
    BigNumMatrix fis;
    BigNumMatrix lambdas;
    BigNumMatrix edensProfiles = ParamStart::getEdensFromDir("../../testData/eldens_profile", fis, lambdas);
    BigNumMatrix nVector = ParamStart::createNVector(heightData, verticalProfiles, edensProfiles);
    BigNumMatrix paramStartDesignMatrix = ParamStart::createDesignMatrix(heightData,
                                                                         verticalProfiles,
                                                                         fis,
                                                                         lambdas,
                                                                         N,
                                                                         edensProfiles.getRowNum());
    fb.open ("./edensProfiles", ios::out);
    os.flush();
    os.rdbuf(&fb);
    os << edensProfiles;
    fb.close();
    fb.open ("./nVector", ios::out);
    os.flush();
    os.rdbuf(&fb);
    os << nVector;
    fb.close();
    fb.open ("./fis", ios::out);
    os.flush();
    os.rdbuf(&fb);
    os << fis;
    fb.close();
    fb.open ("./lambdas", ios::out);
    os.flush();
    os.rdbuf(&fb);
    os << lambdas;
    fb.close();
    fb.open ("./paramStartDesignMatrix", ios::out);
    os.flush();
    os.rdbuf(&fb);
    os << paramStartDesignMatrix;
    fb.close();

    cout << "Calculating paramstart normal matrix" << endl;
    paramStartDesignMatrix.setPrec(256);
    paramStartDesignMatrix.setThreshold(mpf_class("1e-40", 256));
    BigNumMatrix pNormalMatrix = paramStartDesignMatrix.transpose() * paramStartDesignMatrix;
    cout << "paramstart Normal matrix created" << endl;
    pNormalMatrix.roundToZero();
    fb.open ("./paramStartNormalmat", ios::out);
    os.flush();
    os.rdbuf(&fb);
    os << setprecision(20) << pNormalMatrix << endl;
    fb.close();
    cout << "Calculating paramstart inverse matrix" << endl;
    BigNumMatrix pInverse = pNormalMatrix.inverse();
    fb.open ("./paramStartNormalMatInv", ios::out);
    os.flush();
    os.rdbuf(&fb);
    os << setprecision(20) << pInverse << endl;
    fb.close();
    cout << "paramstart Inverse matrix created" << endl;

    cout << "Calculating paramstart least squares" << endl;
    BigNumMatrix paramStart(paramStartDesignMatrix.getColNum(), 1);
    BigNumMatrix meresJav(paramStartDesignMatrix.getRowNum(), 1);
    for (i = 0; i < 4; i++)
    {
        BigNumMatrix a0 = paramStartDesignMatrix * paramStart;
        BigNumMatrix ttv = nVector - a0;
        BigNumMatrix paramJav = pInverse * paramStartDesignMatrix.transpose() * ttv;
        meresJav = paramStartDesignMatrix * paramJav - ttv;
        fb.open ("./paramStartParamJav", ios::out);
        os.flush();
        os.rdbuf(&fb);
        os << setprecision(20) << paramJav << endl << endl;
        fb.close();
        fb.open ("./paramStartMeresJav", ios::out);
        os.flush();
        os.rdbuf(&fb);
        os << setprecision(20) << meresJav << endl;
        fb.close();
        paramStart = paramStart + paramJav;
    }
    cout << "paramstart least squares calculation done" << endl << endl << endl << endl;
    fb.open ("./psParamStart", ios::out);
    os.flush();
    os.rdbuf(&fb);
    os << setprecision(20) << paramStart << endl;
    fb.close();
    BigNumMatrix pIdentity = pNormalMatrix * pInverse;
    fb.open ("./pIdentity", ios::out);
    os.flush();
    os.rdbuf(&fb);
    os << setprecision(20) << pIdentity << endl;
    fb.close();
    //check result
    BigNumMatrix paramstartCheckMatrix = paramStartDesignMatrix * paramStart - (nVector + meresJav);
    fb.open ("./paramStartCheckMatrix", ios::out);
    os.flush();
    os.rdbuf(&fb);
    os << setprecision(20) << paramstartCheckMatrix << endl;
    fb.close();

//======================================================
//                     Alakmatrix
//======================================================


    /*
    printf("Creating alakmatrix\n");
    Alakmatrix alakmatrix(K, N, verticalProfiles, gpsSatCoords, stationCoords, 1000, filteredMeasurements);
    alakmatrix.calcAlakmatrix();
    BigNumMatrix bnAlakmatrix(alakmatrix.alakmatrix);
    bnAlakmatrix.setPrec(256);
    bnAlakmatrix.setThreshold(mpf_class("1e-50", 256));
    printf("Alakmatrix created\n");

    fb.open ("./gpscoords", ios::out);
    os.flush();
    os.rdbuf(&fb);
    for (sat_coord_t::iterator it = gpsSatCoords.begin(); it != gpsSatCoords.end(); it++)
    {
        os << it->first << endl;
        for (map<long, GeographicalVector>::iterator it2 = it->second.begin(); it2 != it->second.end(); it2++)
        {
            os << "    " << it2->first << "    " << it2->second.x << " " << it2->second.y << " " << it2->second.z << endl;
        }
    }
    fb.close();

    fb.open ("./Alakmatrix", ios::out);
    os.flush();
    os.rdbuf(&fb);
    os << setprecision(16) << alakmatrix.alakmatrix << endl;
    fb.close();
    fb.open ("./Weights", ios::out);
    os.flush();
    os.rdbuf(&fb);
    os << setprecision(16) << weights << endl;
    fb.close();

    //Creating measurements and constants
    BigNumMatrix measVec(filteredMeasurements.size(), 1);
    for (int i = 0; i < filteredMeasurements.size(); i++)
    {
        measVec(i, 0) = (filteredMeasurements[i].C1 - filteredMeasurements[i].P2) /* / alakmatrix.commonCoeff *//*;
    }

    BigNumMatrix constants(filteredMeasurements.size(), 1);
    for (int i = 0; i < filteredMeasurements.size(); i++)
    {
        constants(i, 0) = p1p2dcb[filteredMeasurements[i].satId] + p1p2recdcb[filteredMeasurements[i].recId] /* / alakmatrix.commonCoeff*//*;
    }
    cout << "measurements and constants created" << endl;*/

//======================================================
//                      LSQ method
//======================================================

    /*
    cout << "Calculating normal matrix" << endl;
    BigNumMatrix normalMatrix = bnAlakmatrix.transpose() * weights * bnAlakmatrix;
    cout << "Normal matrix created" << endl;
    normalMatrix.roundToZero();
    fb.open ("./normalmat", ios::out);
    os.flush();
    os.rdbuf(&fb);
    os << setprecision(20) << normalMatrix << endl;
    fb.close();
    cout << "Calculating inverse matrix" << endl;
    BigNumMatrix inverse = normalMatrix.inverse();
    fb.open ("./normalMatInv", ios::out);
    os.flush();
    os.rdbuf(&fb);
    os << setprecision(20) << inverse << endl;
    fb.close();
    cout << "Inverse matrix created" << endl;

    cout << "frequency coefficient: " << alakmatrix.commonCoeff << endl;

    cout << "Calculating least squares" << endl;
    BigNumMatrix paramStart(normalMatrix.getColNum(), 1);
    BigNumMatrix meresJav(bnAlakmatrix.getRowNum(), 1);
    for (i = 0; i < 4; i++)
    {
        BigNumMatrix a0 = bnAlakmatrix * paramStart;
        BigNumMatrix ttv = measVec - a0 - constants;
        BigNumMatrix paramJav = inverse * bnAlakmatrix.transpose() * weights * ttv;
        meresJav = bnAlakmatrix * paramJav - ttv;
        fb.open ("./paramJav", ios::out);
        os.flush();
        os.rdbuf(&fb);
        os << setprecision(20) << paramJav << endl << endl << endl << endl;
        fb.close();
        fb.open ("./meresJav", ios::out);
        os.flush();
        os.rdbuf(&fb);
        os << setprecision(20) << meresJav << endl;
        fb.close();
        paramStart = paramStart + paramJav;
    }
    cout << "Least squares calculation done" << endl << endl << endl << endl;
    fb.open ("./param", ios::out);
    os.flush();
    os.rdbuf(&fb);
    os << setprecision(20) << paramStart << endl;
    fb.close();
    BigNumMatrix identity = normalMatrix * inverse;
    fb.open ("./identity", ios::out);
    os.flush();
    os.rdbuf(&fb);
    os << setprecision(20) << identity << endl;
    fb.close();
    //check result
    BigNumMatrix checkMatrix = bnAlakmatrix * paramStart - (measVec + meresJav -constants);
    fb.open ("./checkMatrix", ios::out);
    os.flush();
    os.rdbuf(&fb);
    os << setprecision(20) << checkMatrix << endl;
    fb.close();*/

//======================================================
//                   Kaczmarz method
//======================================================

    /*
    cout << "Calculating Kaczmarz" << endl;
    BigNumMatrix paramStart = EquationSystemSolver::calculateKaczmarz(bnAlakmatrix, measVec - constants, 1000000);
    cout << "Kaczmarz calculation done" << endl;
    fb.open ("./kaczmarzParam", ios::out);
    os.flush();
    os.rdbuf(&fb);
    os << setprecision(20) << paramStart << endl;
    fb.close();

    BigNumMatrix residual = bnAlakmatrix * paramStart - (measVec - constants);
    fb.open ("./KaczmarzResidual", ios::out);
    os.flush();
    os.rdbuf(&fb);
    os << setprecision(20) << residual << endl;
    fb.close();*/

//======================================================
//                   Extended Kaczmarz method
//======================================================

/*    cout << "Calculating Extended Kaczmarz" << endl;
    paramStart = EquationSystemSolver::calculateKE(bnAlakmatrix, measVec - constants, 1000);
    cout << "Extended Kaczmarz calculation done" << endl;
    fb.open ("./KEParam", ios::out);
    os.flush();
    os.rdbuf(&fb);
    os << setprecision(20) << paramStart << endl;
    fb.close();

    BigNumMatrix residual = bnAlakmatrix * paramStart - (measVec - constants);
    fb.open ("./KEResidual", ios::out);
    os.flush();
    os.rdbuf(&fb);
    os << setprecision(20) << residual << endl;
    fb.close();*/

//======================================================
// Extended Kaczmarz method with relaxation parameters
//======================================================

    /*cout << "Calculating Extended Kaczmarz with relaxation parameters" << endl;
    BigNumMatrix paramStart = EquationSystemSolver::calculateKERP(bnAlakmatrix,
                                                                  measVec - constants,
                                                                  mpf_class(0.15, bnAlakmatrix.getPrec()),
                                                                  mpf_class(1.4, bnAlakmatrix.getPrec()),
                                                                  250);
    cout << "Extended Kaczmarz with relaxation parameters calculation done" << endl;
    fb.open ("./KERPParam", ios::out);
    os.flush();
    os.rdbuf(&fb);
    os << setprecision(20) << paramStart << endl;
    fb.close();

    BigNumMatrix residual = bnAlakmatrix * paramStart - (measVec - constants);
    fb.open ("./KERPResidual", ios::out);
    os.flush();
    os.rdbuf(&fb);
    os << setprecision(20) << residual << endl;
    fb.close();*/

//======================================================
//                    DLSQ method
//======================================================

    /*
    cout << "Calculating DLSQ" << endl;
    BigNumMatrix meresJav(bnAlakmatrix.getRowNum(), 1, bnAlakmatrix.getPrec());
    BigNumMatrix paramStart = EquationSystemSolver::calculateDLSQ(bnAlakmatrix, measVec - constants, meresJav, 10, mpf_class(1e-24, bnAlakmatrix.getPrec()));
    cout << "DLSQ calculation done" << endl;
    fb.open ("./meresJavDLSQ", ios::out);
    os.flush();
    os.rdbuf(&fb);
    os << setprecision(20) << meresJav << endl;
    fb.close();
    fb.open ("./paramDLSQ", ios::out);
    os.flush();
    os.rdbuf(&fb);
    os << setprecision(20) << paramStart << endl;
    fb.close();*/

    //Reconstruction
    double fi = 40;
    double lambda = 10;
    double fiRad = fi / 180.0 * PI;
    double lambdaRad = lambda / 180.0 * PI;
    map<unsigned int, double> verticalEdensProfile;
    for (vector<unsigned int>::iterator it = heightData.begin(); it != heightData.end(); it++)
    {
        double value = 0;
        for (unsigned int k = 0; k < K; k++)
        {
            for (unsigned int n = 0; n < N; n++)
            {
                for (unsigned int m = 0; m <= n; m++)
                {
                    unsigned int colIndexA = k * (N + 1) * N / 2 + (n + 1) * n / 2 + m;
                    unsigned int colIndexB = K * (N + 1) * N / 2 +
                                             k * (N + 1) * N / 2 + (n + 1) * n / 2 + m -
                                             k * N - (n + 1);
                    double Pnm = Alakmatrix::calcLegendrePolynomial(n, m, fiRad);
                    value += paramStart(colIndexA,0).get_d() * cos(m * lambdaRad) * Pnm * verticalProfiles[k][*it];
                    if (m != 0)
                    {
                        value += paramStart(colIndexB,0).get_d() * sin(m * lambdaRad) * Pnm * verticalProfiles[k][*it];
                    }
                }
            }
        }
        verticalEdensProfile[*it] = value;
    }

    mpf_class tec = 0;
    for (i = 1; i < heightData.size(); i++)
    {
        tec += (verticalEdensProfile[heightData[i]] + verticalEdensProfile[heightData[i-1]]) / 2 *
               (heightData[i] - heightData[i-1]);
    }

    fb.open ("./edens_e10n40", ios::out);
    os.flush();
    os.rdbuf(&fb);
    for (map<unsigned int, double>::iterator it = verticalEdensProfile.begin(); it != verticalEdensProfile.end(); it++)
    {
        os << it->first << "    " << it->second << endl;
    }
    os << "TEC: " << tec << endl;
    fb.close();



    fi = 40;
    lambda = 20;
    fiRad = fi / 180.0 * PI;
    lambdaRad = lambda / 180.0 * PI;
    for (vector<unsigned int>::iterator it = heightData.begin(); it != heightData.end(); it++)
    {
        double value = 0;
        for (unsigned int k = 0; k < K; k++)
        {
            for (unsigned int n = 0; n < N; n++)
            {
                for (unsigned int m = 0; m <= n; m++)
                {
                    unsigned int colIndexA = k * (N + 1) * N / 2 + (n + 1) * n / 2 + m;
                    unsigned int colIndexB = K * (N + 1) * N / 2 +
                                             k * (N + 1) * N / 2 + (n + 1) * n / 2 + m -
                                             k * N - (n + 1);
                    double Pnm = Alakmatrix::calcLegendrePolynomial(n, m, fiRad);
                    value += paramStart(colIndexA,0).get_d() * cos(m * lambdaRad) * Pnm * verticalProfiles[k][*it];
                    if (m != 0)
                    {
                        value += paramStart(colIndexB,0).get_d() * sin(m * lambdaRad) * Pnm * verticalProfiles[k][*it];
                    }
                }
            }
        }
        verticalEdensProfile[*it] = value;
    }

    tec = 0;
    for (i = 1; i < heightData.size(); i++)
    {
        tec += (verticalEdensProfile[heightData[i]] + verticalEdensProfile[heightData[i-1]]) / 2 *
               (heightData[i] - heightData[i-1]);
    }

    fb.open ("./edens_e20n40", ios::out);
    os.flush();
    os.rdbuf(&fb);
    for (map<unsigned int, double>::iterator it = verticalEdensProfile.begin(); it != verticalEdensProfile.end(); it++)
    {
        os << it->first << "    " << it->second << endl;
    }
    os << "TEC: " << tec << endl;
    fb.close();



    fi = 45;
    lambda = 10;
    fiRad = fi / 180.0 * PI;
    lambdaRad = lambda / 180.0 * PI;
    for (vector<unsigned int>::iterator it = heightData.begin(); it != heightData.end(); it++)
    {
        double value = 0;
        for (unsigned int k = 0; k < K; k++)
        {
            for (unsigned int n = 0; n < N; n++)
            {
                for (unsigned int m = 0; m <= n; m++)
                {
                    unsigned int colIndexA = k * (N + 1) * N / 2 + (n + 1) * n / 2 + m;
                    unsigned int colIndexB = K * (N + 1) * N / 2 +
                                             k * (N + 1) * N / 2 + (n + 1) * n / 2 + m -
                                             k * N - (n + 1);
                    double Pnm = Alakmatrix::calcLegendrePolynomial(n, m, fiRad);
                    value += paramStart(colIndexA,0).get_d() * cos(m * lambdaRad) * Pnm * verticalProfiles[k][*it];
                    if (m != 0)
                    {
                        value += paramStart(colIndexB,0).get_d() * sin(m * lambdaRad) * Pnm * verticalProfiles[k][*it];
                    }
                }
            }
        }
        verticalEdensProfile[*it] = value;
    }

    tec = 0;
    for (i = 1; i < heightData.size(); i++)
    {
        tec += (verticalEdensProfile[heightData[i]] + verticalEdensProfile[heightData[i-1]]) / 2 *
               (heightData[i] - heightData[i-1]);
    }

    fb.open ("./edens_e10n45", ios::out);
    os.flush();
    os.rdbuf(&fb);
    for (map<unsigned int, double>::iterator it = verticalEdensProfile.begin(); it != verticalEdensProfile.end(); it++)
    {
        os << it->first << "    " << it->second << endl;
    }
    os << "TEC: " << tec << endl;
    fb.close();



    fi = 45;
    lambda = 20;
    fiRad = fi / 180.0 * PI;
    lambdaRad = lambda / 180.0 * PI;
    for (vector<unsigned int>::iterator it = heightData.begin(); it != heightData.end(); it++)
    {
        double value = 0;
        for (unsigned int k = 0; k < K; k++)
        {
            for (unsigned int n = 0; n < N; n++)
            {
                for (unsigned int m = 0; m <= n; m++)
                {
                    unsigned int colIndexA = k * (N + 1) * N / 2 + (n + 1) * n / 2 + m;
                    unsigned int colIndexB = K * (N + 1) * N / 2 +
                                             k * (N + 1) * N / 2 + (n + 1) * n / 2 + m -
                                             k * N - (n + 1);
                    double Pnm = Alakmatrix::calcLegendrePolynomial(n, m, fiRad);
                    value += paramStart(colIndexA,0).get_d() * cos(m * lambdaRad) * Pnm * verticalProfiles[k][*it];
                    if (m != 0)
                    {
                        value += paramStart(colIndexB,0).get_d() * sin(m * lambdaRad) * Pnm * verticalProfiles[k][*it];
                    }
                }
            }
        }
        verticalEdensProfile[*it] = value;
    }

    tec = 0;
    for (i = 1; i < heightData.size(); i++)
    {
        tec += (verticalEdensProfile[heightData[i]] + verticalEdensProfile[heightData[i-1]]) / 2 *
               (heightData[i] - heightData[i-1]);
    }

    fb.open ("./edens_e20n45", ios::out);
    os.flush();
    os.rdbuf(&fb);
    for (map<unsigned int, double>::iterator it = verticalEdensProfile.begin(); it != verticalEdensProfile.end(); it++)
    {
        os << it->first << "    " << it->second << endl;
    }
    os << "TEC: " << tec << endl;
    fb.close();



    fi = 42.5;
    lambda = 15;
    fiRad = fi / 180.0 * PI;
    lambdaRad = lambda / 180.0 * PI;
    for (vector<unsigned int>::iterator it = heightData.begin(); it != heightData.end(); it++)
    {
        double value = 0;
        for (unsigned int k = 0; k < K; k++)
        {
            for (unsigned int n = 0; n < N; n++)
            {
                for (unsigned int m = 0; m <= n; m++)
                {
                    unsigned int colIndexA = k * (N + 1) * N / 2 + (n + 1) * n / 2 + m;
                    unsigned int colIndexB = K * (N + 1) * N / 2 +
                                             k * (N + 1) * N / 2 + (n + 1) * n / 2 + m -
                                             k * N - (n + 1);
                    double Pnm = Alakmatrix::calcLegendrePolynomial(n, m, fiRad);
                    value += paramStart(colIndexA,0).get_d() * cos(m * lambdaRad) * Pnm * verticalProfiles[k][*it];
                    if (m != 0)
                    {
                        value += paramStart(colIndexB,0).get_d() * sin(m * lambdaRad) * Pnm * verticalProfiles[k][*it];
                    }
                }
            }
        }
        verticalEdensProfile[*it] = value;
    }

    tec = 0;
    for (i = 1; i < heightData.size(); i++)
    {
        tec += (verticalEdensProfile[heightData[i]] + verticalEdensProfile[heightData[i-1]]) / 2 *
               (heightData[i] - heightData[i-1]);
    }

    fb.open ("./edens_e15.5n42.5", ios::out);
    os.flush();
    os.rdbuf(&fb);
    for (map<unsigned int, double>::iterator it = verticalEdensProfile.begin(); it != verticalEdensProfile.end(); it++)
    {
        os << it->first << "    " << it->second << endl;
    }
    os << "TEC: " << tec << endl;
    fb.close();

    return 0;
}
