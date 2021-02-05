#ifndef BME680_IAQ_h
#define BME680_IAQ_h

#include <stdio.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"

struct BME680_IAQ_Data {
    float iaq, rawTemperature, pressure, rawHumidity, gasResistance, stabStatus, runInStatus, temperature, humidity,
      staticIaq, co2Equivalent, breathVocEquivalent, compGasValue, gasPercentage;
    uint8_t iaqAccuracy, staticIaqAccuracy, co2Accuracy, breathVocAccuracy, compGasAccuracy, gasPercentageAcccuracy;
    uint8_t iaqLevel;
};

class BME680_IAQ{
public:
    int index;
    int lastRead;
    int readInterval;
    bool dataUpdated;
    struct BME680_IAQ_Data data;

    bool isOnline;
    String BSECErrorCode;
    String BSECWarningCode;
    String BMEErrorCode;
    String BMEWarningCode;
    
    void begin(int index);
    void loop();
private:
    void checkIaqSensorStatus(void);
};

#endif
