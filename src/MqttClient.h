#ifndef MQTTCLIENT_h
#define MQTTCLIENT_h

#include <stdio.h>
#include <Wifi.h>
#include <PubSubClient.h>
#include "BME680_IAQ.h"

class MqttClient{
public:
    MqttClient(PubSubClient *client);
    void begin(IPAddress *broker, const char *mqttTopic, const char *deviceIdentifier);
    void loop();
    
    void publishBMEState(float temperature, float pressure, float humidity, float gas);
    void publishBMEState(struct BME680_IAQ_Data *data);

    void publishESState(float temperature, float humidity);
    
    static void MessageReceived(char* topic, byte* payload, unsigned int length);
private:
    const char *DeviceIdentifier;
    PubSubClient* client;
    void reconnectMqtt();
};

#endif
