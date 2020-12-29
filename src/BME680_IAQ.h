#ifndef BME680_IAQ_h
#define BME680_IAQ_h

#include <stdio.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"

struct BME680_IAQ_Data {
    float iaq, rawTemperature, pressure, rawHumidity, gasResistance, stabStatus, runInStatus, temperature, humidity,
      staticIaq, co2Equivalent, breathVocEquivalent, compGasValue, gasPercentage;
    uint8_t iaqAccuracy, staticIaqAccuracy, co2Accuracy, breathVocAccuracy, compGasAccuracy, gasPercentageAcccuracy;
};

class BME680_IAQ{
public:
    struct BME680_IAQ_Data data;
    bool dataUpdated;
    
    void begin();
    void loop();
private:
};

#endif
