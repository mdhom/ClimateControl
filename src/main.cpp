#include <Arduino.h>
#include <WiFi.h>

#include "config.h"
#include "WifiReconnector.h"
#include "MqttClient.h"
#include "MuxControl.h"
#include "PreferencesManager.h"
#include "Fan.h"
#include <OTAUpdateClient.h>

PreferencesManager preferencesManager;
WiFiClient wificlient;
WifiReconnector wifiReconnector;
OTAUpdateClient updateClient(wificlient, "mdwd.org", 80);
PubSubClient client(wificlient);
MqttClient mqtt(&client, &preferencesManager);
unsigned long lastSystemStatePublished = millis();
bool wifiConnectedLc = false;

Fan fanIn(17, 1, 5000, 8);
Fan fanOut(16, 1, 5000, 8);

MuxControl mux;

#include "BME680_IAQ.h"
BME680_IAQ bmes[NumBME];
int muxPortsBME[NumBME];
int lastBMEPublished[NumBME];

void setup() 
{  
  for (int i = 0; i < NumBME; i++) {
    lastBMEPublished[i] = millis();
  }
  muxPortsBME[0] = 0;
  muxPortsBME[1] = 1;
  muxPortsBME[2] = 6;
  muxPortsBME[3] = 7;

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
  
  for (int i = 0; i < NumBME; i++) {
    mux.enableMuxPort(muxPortsBME[i]);
    bmes[i].begin(i);
    mux.disableMuxPort(muxPortsBME[i]);
  }
  
  fanIn.begin();
  fanOut.begin();
}

void loop() 
{
  if (!wifiReconnector.isConnected()) {
    wifiConnectedLc = false;
    digitalWrite(LED_BUILTIN, HIGH);
    delay(500);
    digitalWrite(LED_BUILTIN, LOW);
    return;
  } else if (!wifiConnectedLc) {
    // WiFi connection established!
    wifiConnectedLc = true;
    if (!updateClient.isUpdateAvailable("/aircontrol/update/version.json"))
    {
        Serial.println("No update available");
    } else {
        Serial.println("Updating...");
        updateClient.update("/aircontrol/update/firmware.bin");
    }
  }
  mqtt.WiFiRSSI = wifiReconnector.RSSI;
  
  mqtt.loop();

  for (int i = 0; i < NumBME; i++) {
    mux.enableMuxPort(muxPortsBME[i]);
    bmes[i].loop();
    mux.disableMuxPort(muxPortsBME[i]);

    mqtt.BME680Online[i]    = bmes[i].isOnline;
    mqtt.BSECErrorCode[i]   = bmes[i].BSECErrorCode;
    mqtt.BSECWarningCode[i] = bmes[i].BSECWarningCode;
    mqtt.BMEErrorCode[i]    = bmes[i].BMEErrorCode;
    mqtt.BMEWarningCode[i]  = bmes[i].BMEWarningCode;

    if ((millis() - lastBMEPublished[i]) > preferencesManager.GetPublishInterval()) {
      lastBMEPublished[i] = millis();
      mqtt.publishBMEState(i, &bmes[i].data);
    }
  }

  if ((millis() - lastSystemStatePublished) > 500) {
    lastSystemStatePublished = millis();
    mqtt.publishSystemState();
  }

  bool fanUpdated = false;
  fanUpdated |= fanIn.setSpeed(mqtt.FanInSetValue);
  fanUpdated |= fanOut.setSpeed(mqtt.FanOutSetValue);
  if (fanUpdated) {
    mqtt.publishSystemState();
  }

  fanIn.loop();
  fanOut.loop();
}