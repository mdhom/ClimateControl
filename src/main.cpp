#include <Arduino.h>
#include <WiFi.h>

#include "config.h"
#include "WifiReconnector.h"
#include "MqttClient.h"
#include "MuxControl.h"
#include "PreferencesManager.h"

PreferencesManager preferencesManager;
WiFiClient wificlient;
WifiReconnector wifiReconnector;
PubSubClient client(wificlient);
MqttClient mqtt(&client, &preferencesManager);
int lastSystemStatePublished = millis();

MuxControl mux;

#ifdef USE_ENVIRONMENTALSENSOR
  #include "EnvironmentalSensor.h"
  EnvironmentalSensor environmentalSensor;
  int lastEnvironmentalSensorMeasured  = millis();
  int lastEnvironmentalSensorPublished = millis();
#endif
#ifdef USE_BME60
  #include "BME680.h"
  BME680 bme;
  int lastBMEStatePublished = millis();
#endif
#ifdef USE_BME680_IAQ
  #include "BME680_IAQ.h"
  BME680_IAQ bmeIAQ;
  int lastBMEPublished = millis();
#endif

void setup() 
{  
  Serial.begin(115200);
  Serial.println();
  Serial.println();
  Serial.println("AirControl Start Up");
  Serial.println();

  WiFi.mode(WIFI_STA);
  wifiReconnector.begin(ssid, password);

  mqtt.begin(&broker, mqttTopic, roomIdentifier);

  pinMode(LED_BUILTIN, OUTPUT);

  // start I2C
  Wire.begin();
  #ifdef USE_BME60
    mqtt.BME680Equipped = true;

    mux.enableMuxPort(0);
    while (!bme.begin()) {
      delay(1000);
    }
    mux.disableMuxPort(0);
  #endif
  #ifdef USE_BME680_IAQ
    mqtt.BME680Equipped = true;

    mux.enableMuxPort(0);
    bmeIAQ.begin();
    mux.disableMuxPort(0);
  #endif
  #ifdef USE_ENVIRONMENTALSENSOR
    mqtt.BME280Equipped = true;
    mqtt.CCS811Equipped = true;

    mux.enableMuxPort(1);
    environmentalSensor.begin();
    mux.disableMuxPort(1);
  #endif

  Serial.print("PublishInterval: ");
  Serial.println(preferencesManager.GetPublishInterval());
}

void loop() 
{
  if (!wifiReconnector.isConnected()) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(500);
    digitalWrite(LED_BUILTIN, LOW);
    return;
  }
  mqtt.WiFiRSSI = wifiReconnector.RSSI;
  
  mqtt.loop();

  #ifdef USE_ENVIRONMENTALSENSOR
    if ((millis() - lastEnvironmentalSensorMeasured) > 1000) {
      mux.enableMuxPort(1);
      environmentalSensor.fetch();
      mux.disableMuxPort(1);

      mqtt.CCS811Online = environmentalSensor.CCS811Online;
      mqtt.BME280Online = environmentalSensor.BME280Online;

      lastEnvironmentalSensorMeasured = millis();

      if ((millis() - lastEnvironmentalSensorPublished) > preferencesManager.GetPublishInterval()) {
        lastEnvironmentalSensorPublished = millis();

        mqtt.publishESState(
          environmentalSensor.Temperature, 
          environmentalSensor.Humidity);
      }
    }
  #endif
  #ifdef USE_BME60
    if ((millis() - lastBMEStatePublished) > 1000) {
      mux.enableMuxPort(0);
      bme.startReading();
      // could do some other work here
      bme.endReading();
      mux.disableMuxPort(0);

      mqtt.BME680Online = bme.isOnline;

      lastBMEStatePublished = millis();
      mqtt.publishBMEState(bme.Temperature, bme.Pressure, bme.Humidity, bme.Gas);
    }
  #endif
  #ifdef USE_BME680_IAQ
    mux.enableMuxPort(2);
    bmeIAQ.loop();
    mux.disableMuxPort(2);

    mqtt.BME680Online    = bmeIAQ.isOnline;
    mqtt.BSECErrorCode   = bmeIAQ.BSECErrorCode;
    mqtt.BSECWarningCode = bmeIAQ.BSECWarningCode;
    mqtt.BMEErrorCode    = bmeIAQ.BMEErrorCode;
    mqtt.BMEWarningCode  = bmeIAQ.BMEWarningCode;

    if ((millis() - lastBMEPublished) > preferencesManager.GetPublishInterval()) {
      lastBMEPublished = millis();
      mqtt.publishBMEState(&bmeIAQ.data);
    }
  #endif

  if ((millis() - lastSystemStatePublished) > 500) {
    lastSystemStatePublished = millis();
    mqtt.publishSystemState();
  }
}