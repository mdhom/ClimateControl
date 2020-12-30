#ifndef ENVIRONMENTALSENSOR_h
#define ENVIRONMENTALSENSOR_h

#include <stdio.h>
#include <Wire.h>
#include <SparkFunBME280.h>
#include <SparkFunCCS811.h>

class EnvironmentalSensor{
public:
    uint16_t CO2;
    uint16_t TVOC;
    float Temperature;
    float Humidity;
    float Pressure;
    double DewPoint;

    EnvironmentalSensor();
    void begin();
    void fetch();
private:
    void initBME280();
    void initCCS811();
    void printData();
};

#endif
