#include "BME680_IAQ.h"
#include "bsec.h"

void checkIaqSensorStatus(void);

Bsec iaqSensor;

void BME680_IAQ::begin() {

  iaqSensor.begin(BME680_I2C_ADDR_SECONDARY, Wire);
  Serial.println("BSEC library version " + String(iaqSensor.version.major) + "." + String(iaqSensor.version.minor) + "." + String(iaqSensor.version.major_bugfix) + "." + String(iaqSensor.version.minor_bugfix));
  checkIaqSensorStatus();

  bsec_virtual_sensor_t sensorList[10] = {
    BSEC_OUTPUT_RAW_TEMPERATURE,
    BSEC_OUTPUT_RAW_PRESSURE,
    BSEC_OUTPUT_RAW_HUMIDITY,
    BSEC_OUTPUT_RAW_GAS,
    BSEC_OUTPUT_IAQ,
    BSEC_OUTPUT_STATIC_IAQ,
    BSEC_OUTPUT_CO2_EQUIVALENT,
    BSEC_OUTPUT_BREATH_VOC_EQUIVALENT,
    BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_TEMPERATURE,
    BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_HUMIDITY,
  };

  iaqSensor.updateSubscription(sensorList, 10, BSEC_SAMPLE_RATE_LP);
  checkIaqSensorStatus();
}

void BME680_IAQ::loop() 
{
  if (iaqSensor.run()) 
  { 
    // If new data is available
    this->data.breathVocAccuracy      = iaqSensor.breathVocAccuracy;
    this->data.breathVocEquivalent    = iaqSensor.breathVocEquivalent;
    this->data.co2Accuracy            = iaqSensor.co2Accuracy;
    this->data.co2Equivalent          = iaqSensor.co2Equivalent;
    this->data.compGasAccuracy        = iaqSensor.compGasAccuracy;
    this->data.compGasValue           = iaqSensor.compGasValue;
    this->data.gasPercentage          = iaqSensor.gasPercentage;
    this->data.gasPercentageAcccuracy = iaqSensor.gasPercentageAcccuracy;
    this->data.gasResistance          = iaqSensor.gasResistance;
    this->data.humidity               = iaqSensor.humidity;
    this->data.iaq                    = iaqSensor.iaq;
    this->data.iaqAccuracy            = iaqSensor.iaqAccuracy;
    this->data.pressure               = iaqSensor.pressure;
    this->data.rawHumidity            = iaqSensor.rawHumidity;
    this->data.rawTemperature         = iaqSensor.rawTemperature;
    this->data.runInStatus            = iaqSensor.runInStatus;
    this->data.stabStatus             = iaqSensor.stabStatus;
    this->data.staticIaq              = iaqSensor.staticIaq;
    this->data.staticIaqAccuracy      = iaqSensor.staticIaqAccuracy;
    this->data.temperature            = iaqSensor.temperature;

    if (iaqSensor.iaq < 0) {
      this->data.iaqLevel = -1;
    } else if (iaqSensor.iaq <= 50) {
      this->data.iaqLevel = 0;
    } else if(iaqSensor.iaq <= 100) {
      this->data.iaqLevel = 1;
    } else if(iaqSensor.iaq <= 150) {
      this->data.iaqLevel = 2;
    } else if(iaqSensor.iaq <= 200) {
      this->data.iaqLevel = 3;
    } else if(iaqSensor.iaq <= 300) {
      this->data.iaqLevel = 4;
    } else if(iaqSensor.iaq <= 500) {
      this->data.iaqLevel = 5;
    } else {
      this->data.iaqLevel = -2;
    }

    Serial.print("BME680: Temp[");
    Serial.print(iaqSensor.temperature);
    Serial.print("]C Pressure[");
    Serial.print(iaqSensor.pressure);
    Serial.print("]Pa Humidity[");
    Serial.print(iaqSensor.humidity);
    Serial.print("]% IAQ[");
    Serial.print(iaqSensor.iaq);
    Serial.print("]");
    Serial.println();

    dataUpdated = true;
  } 
  else 
  {
    checkIaqSensorStatus();
    
    dataUpdated = false;
  }
}

void checkIaqSensorStatus(void)
{
  if (iaqSensor.status != BSEC_OK) {
    if (iaqSensor.status < BSEC_OK) {
      Serial.println("BSEC error code : " + String(iaqSensor.status));
    } else {
      Serial.println("BSEC warning code : " + String(iaqSensor.status));
    }
  }

  if (iaqSensor.bme680Status != BME680_OK) {
    if (iaqSensor.bme680Status < BME680_OK) {
      Serial.println("BME680 error code : " + String(iaqSensor.bme680Status));
    } else {
      Serial.println("BME680 warning code : " + String(iaqSensor.bme680Status));
    }
  }
}