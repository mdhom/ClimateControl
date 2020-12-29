#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "MqttClient.h"

char mqttStateTopic[128];

MqttClient::MqttClient(PubSubClient *pubsub) {
    this->client = pubsub;
}

void MqttClient::begin(IPAddress *broker, const char *mqttTopic, const char *deviceIdentifier) {
    this->DeviceIdentifier = deviceIdentifier;
    this->client->setServer(*broker, 1883);
    this->client->setCallback(MessageReceived);
        
    sprintf(mqttStateTopic, "%s/%s", mqttTopic, deviceIdentifier);
    Serial.println("mqttStateTopic:        " + String(mqttStateTopic));
}

void MqttClient::loop(){
  if (!this->client->connected()) {
    this->reconnectMqtt();
  } else {
    this->client->loop();
  }
}

void MqttClient::reconnectMqtt() 
{
  // Loop until we're reconnected
  while (!this->client->connected())
  {
    Serial.println("Attempting to connect to MQTT broker...");
    // Attempt to connect
#ifdef USE_MQTT_AUTH
    if (client.connect(CLIENT_ID, MQTT_USER, MQTT_PASSWORD))
#else
    if(this->client->connect(this->DeviceIdentifier ))
#endif
    {
      Serial.println("Connected to MQTT broker!");
      //this->client->subscribe(mqttStateRequestTopic);
      //this->client->subscribe(mqttSetFlowTopic);
    } 
    else 
    {
      Serial.print("Reconnect failed! State=");
      Serial.println(this->client->state());
      Serial.println("Retry in 3 seconds...");
      delay(3000);
      return;
    }
  }
} 

void MqttClient::MessageReceived(char* topic, byte* payload, unsigned int length) {

}

void MqttClient::publishState(float temperature, float pressure, float humidity, float gas) {
    char message[128];
    sprintf(message, "{\"message\":\"%s\",\"data\":{\"temperature\":%.2f,\"pressure\":%.2f,\"humidity\":%.1f,\"gas\":%.1f }}", "Hello World", temperature, pressure, humidity, gas);
    this->client->publish(mqttStateTopic, message);
}

void MqttClient::publishState(BME680_IAQ_Data *data) {

    char message[4096];
    DynamicJsonDocument doc(4096);
    doc["breathVocAccuracy"] = data->breathVocAccuracy;
    doc["breathVocEquivalent"] = data->breathVocEquivalent;
    doc["co2Accuracy"] = data->co2Accuracy;
    doc["co2Equivalent"] = data->co2Equivalent;
    doc["compGasAccuracy"] = data->compGasAccuracy;
    doc["compGasValue"] = data->compGasValue;
    doc["gasPercentage"] = data->gasPercentage;
    doc["gasPercentageAcccuracy"] = data->gasPercentageAcccuracy;
    doc["gasResistance"] = data->gasResistance;
    doc["humidity"] = data->humidity;
    doc["iaq"] = data->iaq;
    doc["iaqAccuracy"] = data->iaqAccuracy;
    doc["pressure"] = data->pressure;
    doc["rawHumidity"] = data->rawHumidity;
    doc["rawTemperature"] = data->rawTemperature;
    doc["runInStatus"] = data->runInStatus;
    doc["stabStatus"] = data->stabStatus;
    doc["staticIaq"] = data->staticIaq;
    doc["staticIaqAccuracy"] = data->staticIaqAccuracy;
    doc["temperature"] = data->temperature;
    serializeJson(doc, message);
    
    this->client->publish(mqttStateTopic, message);
}