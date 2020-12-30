#include <Arduino.h>
#include <WiFi.h>

#include "config.h"
#include "WifiReconnector.h"
#include "MqttClient.h"
#include "MuxControl.h"

WiFiClient wificlient;
WifiReconnector wifiReconnector;
PubSubClient client(wificlient);
MqttClient mqtt(&client);

MuxControl mux;

#ifdef USE_ENVIRONMENTALSENSOR
  #include "EnvironmentalSensor.h"
  EnvironmentalSensor environmentalSensor;
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
    mux.enableMuxPort(0);
    while (!bme.begin()) {
      delay(1000);
    }
    mux.disableMuxPort(0);
  #endif
  #ifdef USE_BME680_IAQ
    mux.enableMuxPort(0);
    bmeIAQ.begin();
    mux.disableMuxPort(0);
  #endif
  #ifdef USE_ENVIRONMENTALSENSOR
    mux.enableMuxPort(1);
    environmentalSensor.begin();
    mux.disableMuxPort(1);
  #endif
}

void loop() 
{
  if (!wifiReconnector.isConnected()) {
    delay(80);
    return;
  }
  
  mqtt.loop();
  
  digitalWrite(LED_BUILTIN, HIGH);

  #ifdef USE_ENVIRONMENTALSENSOR
    if ((millis() - lastEnvironmentalSensorPublished) > 1000) {
      mux.enableMuxPort(1);
      environmentalSensor.fetch();
      mux.disableMuxPort(1);

      lastEnvironmentalSensorPublished = millis();
      mqtt.publishESState(environmentalSensor.Temperature, environmentalSensor.Humidity);
    }
  #endif
  #ifdef USE_BME60
    if ((millis() - lastBMEStatePublished) > 1000) {
      mux.enableMuxPort(0);
      bme.startReading();
      // could do some other work here
      bme.endReading();
      mux.disableMuxPort(0);

      lastBMEStatePublished = millis();
      mqtt.publishBMEState(bme.Temperature, bme.Pressure, bme.Humidity, bme.Gas);
    }
  #endif
  #ifdef USE_BME680_IAQ
    mux.enableMuxPort(0);
    bmeIAQ.loop();
    mux.disableMuxPort(0);
    
    if (bmeIAQ.dataUpdated) {
      mqtt.publishBMEState(&bmeIAQ.data);
    }
  #endif

  digitalWrite(LED_BUILTIN, LOW);
}