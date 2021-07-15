#ifndef MEASUREMENT_H
#define MEASUREMENT_H

#include <string>
using namespace std;

class Measurement
{
public:
    long gpsTime;
    unsigned int satId;
    string recId;
    double C1;
    double P2;

    Measurement();
    Measurement(long gpsTime, unsigned int satId, char* recId, double C1, double P2);
    ~Measurement();
};

#endif /*MEASUREMENT_H*/