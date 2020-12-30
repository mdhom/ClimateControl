#include <stdio.h>
#include <Arduino.h>
#include "EnvironmentalSensor.h" 
#include <SparkFunBME280.h>
#include <SparkFunCCS811.h>
#include <Wire.h>

float initialBMETemperature = -1;
float heatedBMETemperature = -1;

CCS811 myCCS811(0x5B);  //Default I2C Address
BME280 myBME280;

EnvironmentalSensor::EnvironmentalSensor()
{
}

void EnvironmentalSensor::begin()
{
  initCCS811();
  initBME280();
}

void EnvironmentalSensor::initBME280()
{
  //Initialize BME280
  //For I2C, enable the following and disable the SPI section
  myBME280.settings.commInterface = I2C_MODE;
  myBME280.settings.I2CAddress = 0x77;
  myBME280.settings.runMode = 3; //Normal mode
  myBME280.settings.tStandby = 0;
  myBME280.settings.filter = 4;
  myBME280.settings.tempOverSample = 5;
  myBME280.settings.pressOverSample = 5;
  myBME280.settings.humidOverSample = 5;
  //Calling .begin() causes the settings to be loaded
  delay(10);  //Make sure sensor had enough time to turn on. BME280 requires 2ms to start up.
  byte id = myBME280.begin(); //Returns ID of 0x60 if successful
  if (id != 0x60)
  {
    Serial.println("Problem with BME280");
    while (1)
      ;
  }
  
  Serial.println("BME280 online");
}

void EnvironmentalSensor::initCCS811()
{
  //This begins the CCS811 sensor and prints error status of .begin()
  CCS811Core::CCS811_Status_e returnCode = myCCS811.beginWithStatus();
  Serial.print("beginCore exited with: ");
  Serial.println(myCCS811.statusString(returnCode));

  if (myCCS811.begin() == false) {
    Serial.println("ERROR WITH CCS811");
    while (1)
      ;
  }

  Serial.println("CCS811 online");
}

void EnvironmentalSensor::fetch()
{
  if (myCCS811.dataAvailable())
  {
    myCCS811.readAlgorithmResults();

    CO2 = myCCS811.getCO2();
    TVOC = myCCS811.getTVOC();
    Temperature = myBME280.readTempC();
    Pressure = myBME280.readFloatPressure();
    Humidity = myBME280.readFloatHumidity();
    DewPoint = myBME280.dewPointC();

    if (initialBMETemperature == -1) {
      initialBMETemperature = Temperature;
    } else if (millis() > 1000 * 60 * 20 && heatedBMETemperature == -1) {
      heatedBMETemperature = Temperature;
    }

    if (initialBMETemperature != -1 && heatedBMETemperature != -1) {
      float offset = (heatedBMETemperature - initialBMETemperature);
      Temperature = Temperature - offset;
      Serial.print("Corrected temperature by ");
      Serial.print(offset);
      Serial.println("°");
    } else {
      Serial.println("No temperature correction available");
    }
    
    printData();

    //This sends the temperature data to the CCS811
    myCCS811.setEnvironmentalData(Humidity, Temperature);
  }
  else if (myCCS811.checkForStatusError())
  {
    Serial.print("CCS811 ERROR: ");
    Serial.println(myCCS811.getErrorRegister()); //Prints whatever CSS811 error flags are detected
  }
}

void EnvironmentalSensor::printData()
{
    Serial.print("CCS811: CO2[");
    Serial.print(CO2);
    Serial.print("]ppm tVOC[");
    Serial.print(TVOC);
    Serial.println("]ppb");

    Serial.print("BME280: Temp[");
    Serial.print(Temperature);
    Serial.print("]C Pressure[");
    Serial.print(Pressure);
    Serial.print("]Pa Humidity[");
    Serial.print(Humidity);
    Serial.print("]% DewPoint[");
    Serial.print(DewPoint);
    Serial.println("]C");
}