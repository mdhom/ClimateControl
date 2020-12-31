#include "BME680.h"

#define SEALEVELPRESSURE_HPA (1013.25)

BME680::BME680() {
}

bool BME680::begin(){
    if (!bme.begin()) {
        Serial.println(F("Could not find a valid BME680 sensor, check wiring!"));
        isOnline = false;
        return false;
    }

    // Set up oversampling and filter initialization
    bme.setTemperatureOversampling(BME680_OS_8X);
    bme.setHumidityOversampling(BME680_OS_2X);
    bme.setPressureOversampling(BME680_OS_4X);
    bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
    bme.setGasHeater(320, 150); // 320*C for 150 ms

    isOnline = true;
    return true;
}

void BME680::startReading() {
    unsigned long endTime = bme.beginReading();
    if (endTime == 0) {
        Serial.println(F("Failed to begin reading :("));
        isReading = false;
        return;
    } else {
        isReading = true;
    }
}

void BME680::endReading() {
    if (!isReading) {
        return;
    }
    // Obtain measurement results from BME680. Note that this operation isn't
    // instantaneous even if milli() >= endTime due to I2C/SPI latency.
    if (!bme.endReading()) {
        Serial.println(F("Failed to complete reading :("));
        return;
    }
    
    Serial.print(F("Temperature = "));
    Serial.print(bme.temperature);
    Serial.println(F(" °C"));
    this->Temperature = bme.temperature;

    Serial.print(F("Pressure = "));
    Serial.print(bme.pressure / 100.0);
    Serial.println(F(" hPa"));
    this->Pressure = bme.pressure / 100.0;

    Serial.print(F("Humidity = "));
    Serial.print(bme.humidity);
    Serial.println(F(" %"));
    this->Humidity = bme.humidity;

    Serial.print(F("Gas = "));
    Serial.print(bme.gas_resistance / 1000.0);
    Serial.println(F(" KOhms"));
    this->Gas = bme.gas_resistance / 1000.0;

    Serial.print(F("Approx. Altitude = "));
    Serial.print(bme.readAltitude(SEALEVELPRESSURE_HPA));
    Serial.println(F(" m"));
}