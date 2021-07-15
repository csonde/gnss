#include "measurement.h"

Measurement::Measurement()
{
    this->gpsTime = 0;
    this->satId = 0;
    this->recId = "";
    this->C1 = 0;
    this->P2 = 0;
}

Measurement::Measurement(long gpsTime, unsigned int satId, char* recId, double C1, double P2)
{
    this->gpsTime = gpsTime;
    this->satId = satId;
    this->recId = recId;
    this->C1 = C1;
    this->P2 = P2;
}

Measurement::~Measurement()
{
}
