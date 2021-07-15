#include "rinexCommon.h"
#include <string.h>
#include <regex.h>
#include <stdio.h>


int satTypeNum = 2;
int satPerType = 100;

const double rinexVersion = 2.11;

int replaceDoubleExponent(char* str)
{
    char funcName[] = "replaceDoubleExponent()";

    regex_t compiled;
    memset(&compiled, 0, sizeof(compiled));
    char regex[] = "[0-9]+(D)(\\+|-)[0-9][0-9]";
    if (regcomp(&compiled, regex, REG_ICASE | REG_EXTENDED))
    {
        printf("%s: failed to compile regex: %s", funcName, regex);
        return 1;
    }
    size_t nmatch = 2;
    regmatch_t matchptr[2];
    memset(matchptr, 0, sizeof(regmatch_t) * nmatch);

    int start_index = 0;
    while(!regexec(&compiled, &(str[start_index]), nmatch, matchptr, REG_NOTBOL | REG_NOTEOL))
    {
        str[matchptr[1].rm_so + start_index] = 'E';
        start_index += matchptr[0].rm_eo;
        memset(matchptr, 0, sizeof(regmatch_t) * nmatch);
    }
    regfree(&compiled);
    return 0;
}

int checkEmptyLine(char* str)
{
    int i = 0;
    while (str[i])
    {
        if (str[i] != ' ' && str[i] != '\n')
        {
            return 0;
        }
        i++;
    }
    if(i == 0)
    {
        return 1;
    }
    return 1;
}

int comparePreciseTime(const void * t1, const void * t2)
{
    const PreciseTime* time1 = (const PreciseTime*)t1;
    const PreciseTime* time2 = (const PreciseTime*)t2;
    if (time1->seconds < time2->seconds)
    {
        return -1;
    }
    if (time1->seconds == time2->seconds)
    {
        if (time1->nanos < time2->nanos)
        {
            return -1;
        }
        if (time1->nanos == time2->nanos)
        {
            return 0;
        }
    }
    return 1;
}
