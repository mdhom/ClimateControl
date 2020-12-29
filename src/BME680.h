#ifndef BME680_h
#define BME680_h

#include <stdio.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"

class BME680{
public:
    float Temperature;  // °C
    float Pressure;     // hPa
    float Humidity;     // %
    float Gas;          // KOhms

    BME680();
    bool begin();
    void startReading();
    void endReading();
private:
    Adafruit_BME680 bme; // I2C
    bool isReading;
};

#endif
