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
    bool BME680Online[4];
    String BSECErrorCode[4], BSECWarningCode[4], BMEErrorCode[4], BMEWarningCode[4];
    int8_t WiFiRSSI;
    float FanInSetValue = 50;
    float FanOutSetValue = 50;

    MqttClient(PubSubClient *client, PreferencesManager *preferences);
    void begin(IPAddress *broker, const char *mqttTopic, const char *deviceIdentifier);
    void loop();

    void publishBMEState(int bmeIndex, struct BME680_IAQ_Data *data);

    void publishESState(float temperature, float humidity);

    void publishSystemState();
    void publishError(String error);

    void publishConfig();
    
    void MessageReceived(char* topic, byte* payload, unsigned int length);
private:
    const char *DeviceIdentifier;
    PubSubClient* client;
    PreferencesManager* preferencesManager;
    void reconnectMqtt();

    void handleConfigMessage(StaticJsonDocument<200> *doc);
    void handleSetFanMessage(StaticJsonDocument<200> *doc);
    void handleSetPWMFrequencyMessage(StaticJsonDocument<200> *doc);
};

#endif
