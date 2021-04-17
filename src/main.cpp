#include <Arduino.h>
#include <WiFi.h>
#include <OTAUpdateClient.h>

#include "config.h"
#include "WifiReconnector.h"
#include "MqttClient.h"
#include "MuxControl.h"
#include "PreferencesManager.h"
#include "Fan.h"

PreferencesManager preferencesManager;
WiFiClient wificlient;
WifiReconnector wifiReconnector;
OTAUpdateClient updateClient(wificlient, "mdwd.org", 80);
PubSubClient client(wificlient);
MqttClient mqtt(&client, &preferencesManager);
unsigned long lastSystemStatePublished = millis();
bool wifiConnectedLc = false;

Fan fan(21, 1, 25000, 8, 17);

MuxControl mux;

#ifdef USE_ENVIRONMENTALSENSOR
  #include "EnvironmentalSensor.h"
  EnvironmentalSensor environmentalSensor;
  unsigned long lastEnvironmentalSensorMeasured  = millis();
  unsigned long lastEnvironmentalSensorPublished = millis();
#endif
#ifdef USE_BME60
  #include "BME680.h"
  BME680 bme;
  unsigned long lastBMEStatePublished = millis();
#endif
#ifdef USE_BME680_IAQ
  #include "BME680_IAQ.h"
  BME680_IAQ bmeIAQ;
  unsigned long lastBMEPublished = millis();
#endif
#ifdef USE_OLED
  #include "OLED.h"
  OLED display;
#endif

void setup() 
{  
  Serial.begin(115200);
  Serial.println();
  Serial.println();
  Serial.println("AirControl Start Up");
  Serial.println();
  
  preferencesManager.PrintCurrentConfig();

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

  #ifdef USE_OLED
    display.init();
  #endif
  
  fan.begin();
}

bool isTimeouted(unsigned long lastTimestamp, unsigned long timeoutInterval)
{
  return (millis() - lastTimestamp) >= timeoutInterval || (millis() - lastTimestamp) < 0;
}

void loop() 
{
  #ifdef USE_OLED
  display.showWorking(0);
  #endif

  if (!wifiReconnector.isConnected()) {
    wifiConnectedLc = false;
    #ifdef USE_OLED
    display.showNoWlan();
    #endif

    digitalWrite(LED_BUILTIN, HIGH);
    delay(500);
    digitalWrite(LED_BUILTIN, LOW);
    return;
  } else if (!wifiConnectedLc) {
      // WiFi connection established!
      wifiConnectedLc = true;
      if (!updateClient.isUpdateAvailable("/climatecontrol/update/version.json"))
      {
          Serial.println("No update available");
      } else {
          Serial.println("Updating...");
          updateClient.update("/climatecontrol/update/firmware.bin");
      }
      display.firmwareVersion = updateClient.getLocalVersion();
  }
  mqtt.WiFiRSSI = wifiReconnector.RSSI;
  
  #ifdef USE_OLED
  display.showWorking(1);
  #endif
  
  if (!mqtt.isConnected())
  {
    #ifdef USE_OLED
    display.showNoMqtt();
    #endif

    Serial.println("Attempting to connect to MQTT broker...");
    if (!mqtt.reconnect()) {
      delay(3000);
      return;
    }
  }
  mqtt.loop();
  
  #ifdef USE_OLED
  display.showWorking(2);
  #endif

  #ifdef USE_ENVIRONMENTALSENSOR
    if (isTimeouted(lastEnvironmentalSensorMeasured, 1000UL)) {
      mux.enableMuxPort(1);
      environmentalSensor.fetch();
      mux.disableMuxPort(1);

      mqtt.CCS811Online = environmentalSensor.CCS811Online;
      mqtt.BME280Online = environmentalSensor.BME280Online;

      lastEnvironmentalSensorMeasured = millis();

      if (isTimeouted(lastEnvironmentalSensorPublished, preferencesManager.GetPublishInterval())) {
        lastEnvironmentalSensorPublished = millis();

        mqtt.publishESState(
          environmentalSensor.Temperature, 
          environmentalSensor.Humidity);
      }
    }
  #endif

  #ifdef USE_BME60
    if (isTimeouted(lastBMEStatePublished, 1000UL)) {
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

    #ifdef USE_OLED
    display.showWorking(3);
    #endif

    mqtt.BME680Online    = bmeIAQ.isOnline;
    mqtt.BSECErrorCode   = bmeIAQ.BSECErrorCode;
    mqtt.BSECWarningCode = bmeIAQ.BSECWarningCode;
    mqtt.BMEErrorCode    = bmeIAQ.BMEErrorCode;
    mqtt.BMEWarningCode  = bmeIAQ.BMEWarningCode;

    if (isTimeouted(lastBMEPublished, preferencesManager.GetPublishInterval())) {
      lastBMEPublished = millis();
      mqtt.publishBMEState(&bmeIAQ.data);
    }

    #ifdef USE_OLED
    display.showWorking(4);
    #endif
  #endif

  // SYSTEM STATE
  if (isTimeouted(lastSystemStatePublished, 500UL)) {
    lastSystemStatePublished = millis();
    mqtt.publishSystemState();
  }
    #ifdef USE_OLED
    display.showWorking(5);
    #endif

  // FAN HANDLING
  if (fan.setSpeed(mqtt.FanSetValue)) {
    mqtt.publishSystemState();
  }
  #ifdef USE_OLED
  display.showWorking(6);
  #endif

  fan.loop();
  mqtt.FanTachoValue = fan.TachoSpeed;
  #ifdef USE_OLED
  display.showWorking(7);
  #endif
}