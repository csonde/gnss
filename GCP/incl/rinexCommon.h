#ifndef RINEX_COMMON_H
#define RINEX_COMMON_H

#include <sys/types.h>
#include <time.h>


extern int satTypeNum;
extern int satPerType;

extern const double rinexVersion;

int replaceDoubleExponent(char* str);

int checkEmptyLine(char* str);

typedef enum
{
    OBS_DATA,
    NAV_MSG,
    MET_DATA,
    GLONASS_NAV_MSG,
    NOT_SUPPORTED,
    UNKONWN
}FileType;

typedef struct PreciseTime
{
    time_t seconds;
    int nanos;
}PreciseTime;

int comparePreciseTime(const void * t1, const void * t2);

#endif //RINEX_COMMON_H