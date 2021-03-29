#ifndef MQTTCLIENT_h
#define MQTTCLIENT_h

#include <stdio.h>
#include <Wifi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "BME680_IAQ.h"
#include "PreferencesManager.h"

class MqttClient{
public:
    bool BME280Equipped, CCS811Equipped, BME680Equipped;
    bool BME280Online, CCS811Online, BME680Online;
    String BSECErrorCode, BSECWarningCode, BMEErrorCode, BMEWarningCode;
    int8_t WiFiRSSI;
    float FanSetValue = 70;
    float FanTachoValue;

    MqttClient(PubSubClient *client, PreferencesManager *preferences);
    void begin(IPAddress *broker, const char *mqttTopic, const char *deviceIdentifier);

    bool isConnected();
    bool reconnect();
    void loop();

    void publishBMEState(float temperature, float pressure, float humidity, float gas);
    void publishBMEState(struct BME680_IAQ_Data *data);

    void publishESState(float temperature, float humidity);

    void publishSystemState();
    void publishError(String error);

    void publishConfig();
    
    void MessageReceived(char* topic, byte* payload, unsigned int length);
private:
    const char *DeviceIdentifier;
    PubSubClient* client;
    PreferencesManager* preferencesManager;

    void handleConfigMessage(StaticJsonDocument<200> *doc);
    void handleSetFanMessage(StaticJsonDocument<200> *doc);
    void handleSetPWMFrequencyMessage(StaticJsonDocument<200> *doc);
};

#endif
