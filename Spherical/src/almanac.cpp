#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <almanac.h>

#include <iostream>
using namespace std;

const long unixUTCMinusGPS = 315964800 - 16;

long utcToGPST(long utc)
{
    return utc - unixUTCMinusGPS;
}

long gpstToUTC(long gpst)
{
    return gpst + unixUTCMinusGPS;
}

void calculateGPSSatellitePositions(double t, char* almanacFile, sat_coord_t& results)
{
    char string[80], name[30], value[30], *pstr;
    FILE *pinfile;
    double e, t0, i, inc, omegadot, sqrta, omega0, kis_omega, m0, nu, omega_e;
    double n, kozep_anom, exc_anom, exc_anom1, valod_anom, r, v, u1, u2, nagy_omega;
    double x, y, z, x_ecef, y_ecef, z_ecef;
    int id, health, week;

    if (!(pinfile = fopen(almanacFile,"rt")))
    {
        printf("Can not find input almanac file\n");
        exit(-1);
    }

    /*Állandók megadása:*/

    nu = 3.986004418E+14;
    omega_e = 7.2921151467E-05;

    /*Almanac adatok beolvasása:*/

    printf("Calculating GPS satellite positon for t = %.0lf from almanac\n", t);
    while (!(feof(pinfile)))
    {
        fgets(string, 80, pinfile);
        pstr = name;
        *pstr = '\0';
        strncat(name,string,27);
        pstr = value;
        *pstr = '\0';
        pstr=string;
        pstr+=27;
        strcat(value,pstr);
        if (!(strncmp(name,"**",2)))
        {
            for (i=0; i<13; i++)
            {
                fgets(string, 80, pinfile);
                pstr = name;
                *pstr = '\0';
                strncat(name,string,27);
                pstr = value;
                *pstr = '\0';
                pstr=string;
                pstr+=27;
                strcat(value,pstr);
                if (!(strncmp(name,"ID",2)))
                {
                    id = atoi(value);
                }
                if (!(strncmp(name,"Hea",3)))
                {
                    health = atoi(value);
                }
                if (!(strncmp(name, "Ec",2)))
                {
                    e = atof(value);
                }
                if(!(strncmp(name,"Ti",2)))
                {
                    t0 = atof(value);
                }
                if(!(strncmp(name,"Or",2)))
                {
                    inc = atof(value);
                }
                if(!(strncmp(name,"Ra",2)))
                {
                    omegadot = atof(value);
                }
                if(!(strncmp(name,"SQ",2)))
                {
                    sqrta = atof(value);
                }
                if(!(strncmp(name,"Ri",2)))
                {
                    omega0 = atof(value);
                }
                if(!(strncmp(name,"Ar",2)))
                {
                    kis_omega = atof(value);
                }
                if(!(strncmp(name,"Mea",3)))
                {
                    m0 = atof(value);
                }
                if(!(strncmp(name,"we",2)))
                {
                    week = atoi(value);
                }
            }

            /* Számítások: */

            n = sqrt(nu/pow(sqrta,6));
            kozep_anom  = m0 + n*(t-t0);
            exc_anom1 = kozep_anom + e*sin(kozep_anom);
            while(exc_anom - exc_anom1 > 0.000001 || exc_anom - exc_anom1 < -0.000001)
            {
                exc_anom = kozep_anom + e*sin(exc_anom1);
                exc_anom1 = kozep_anom + e*sin(exc_anom);
            }

            valod_anom=2*atan(sqrt((1+e)/(1-e))*tan(exc_anom/2));

            r = pow(sqrta,2)*(1-e*cos(exc_anom));
            v = ((n*pow(sqrta,4)))/r*sqrt(1-pow(e*cos(exc_anom),2));

            u1 = r*cos(valod_anom);
            u2 = r*sin(valod_anom);

            /*Forgatás:*/

            nagy_omega = omega0 + omegadot*(t-t0);

            x = u1*(cos(nagy_omega)*cos(kis_omega)-sin(nagy_omega)*sin(kis_omega)*cos(inc))+u2*(-cos(nagy_omega)*sin(kis_omega)-sin(nagy_omega)*cos(kis_omega)*cos(inc));

            y = u1*(sin(nagy_omega)*cos(kis_omega)+cos(nagy_omega)*sin(kis_omega)*cos(inc))+u2*(-sin(nagy_omega)*sin(kis_omega)+cos(nagy_omega)*cos(kis_omega)*cos(inc));

            z = u1*(sin(kis_omega)*sin(inc))+u2*(cos(kis_omega)*sin(inc));

            x_ecef = x*cos(omega_e*t)+y*sin(omega_e*t);
            y_ecef = -x*sin(omega_e*t)+y*cos(omega_e*t);
            z_ecef = z;

            /*Kiírás:*/

            if (health == 0)
            {
                map<long, GeographicalVector>& satmap = results[id-1];
                satmap[t] = GeographicalVector(x_ecef, y_ecef, z_ecef, Alakmatrix::a, Alakmatrix::e);
            }
        }
    }
    fclose(pinfile);
}

double calculateTimeOfGPSWeek(long gpsTime)
{
    //-16 is because of leap seconds
    return gpsTime % 604800;
}