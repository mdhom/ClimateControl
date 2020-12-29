#include <Arduino.h>
#include <WiFi.h>

#include "config.h"
#include "WifiReconnector.h"
#include "MqttClient.h"
#include "BME680.h"
#include "BME680_IAQ.h"
#include "MuxControl.h"

WiFiClient wificlient;
WifiReconnector wifiReconnector;
PubSubClient client(wificlient);
MqttClient mqtt(&client);
int lastStatePublished = millis();

MuxControl mux;
#if USE_BME60
  BME680 bme;
#else
  BME680_IAQ bmeIAQ;
#endif

void setup() {
  
  Serial.begin(115200);

  Serial.println();
  Serial.println();
  Serial.println("AirControl Start Up");
  Serial.println();

  WiFi.mode(WIFI_STA);
  wifiReconnector.begin(ssid, password);

  mqtt.begin(&broker, mqttTopic, roomIdentifier);

  // put your setup code here, to run once:
  pinMode(LED_BUILTIN, OUTPUT);

  // start I2C
  Wire.begin();
  mux.enableMuxPort(0);
  #if USE_BME60
    while (!bme.begin()) {
      delay(1000);
    }
  #else
    bmeIAQ.begin();
  #endif
}

void loop() {
  if (!wifiReconnector.isConnected()) {
    delay(80);
    return;
  }
  
  mqtt.loop();
  
  digitalWrite(LED_BUILTIN, HIGH);


  #if USE_BME60
    bme.startReading();
    // could do some other work here
    bme.endReading();
  #else
    bmeIAQ.loop();
  #endif

    #if USE_BME60
      if ((millis() - lastStatePublished) > 1000) {
        lastStatePublished = millis();
        mqtt.publishState(bme.Temperature, bme.Pressure, bme.Humidity, bme.Gas);
      }
    #else
    if (bmeIAQ.dataUpdated) {
      mqtt.publishState(&bmeIAQ.data);
    }
    #endif
  digitalWrite(LED_BUILTIN, LOW);
}