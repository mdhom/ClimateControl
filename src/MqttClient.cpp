#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "MqttClient.h"

char mqttBMETopic[128];
char mqttESTopic[128];
char mqttSystemTopic[128];

char systemStateMessageLc[4096];
int lastStatePublished = millis();

MqttClient::MqttClient(PubSubClient *pubsub) 
{
    this->client = pubsub;
}

void MqttClient::begin(IPAddress *broker, const char *mqttTopic, const char *deviceIdentifier) 
{
    this->DeviceIdentifier = deviceIdentifier;
    this->client->setServer(*broker, 1883);
    this->client->setCallback(MessageReceived);
        
    sprintf(mqttBMETopic, "%s/%s/BME", mqttTopic, deviceIdentifier);
    Serial.println("mqttBMETopic:    " + String(mqttBMETopic));
        
    sprintf(mqttESTopic, "%s/%s/ES", mqttTopic, deviceIdentifier);
    Serial.println("mqttESTopic:     " + String(mqttESTopic));
        
    sprintf(mqttSystemTopic, "%s/%s", mqttTopic, deviceIdentifier);
    Serial.println("mqttSystemTopic: " + String(mqttSystemTopic));
}

void MqttClient::loop()
{
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

void MqttClient::MessageReceived(char* topic, byte* payload, unsigned int length) 
{
}

void MqttClient::publishBMEState(float temperature, float pressure, float humidity, float gas) 
{
    char message[128];
    sprintf(message, "{\"message\":\"%s\",\"data\":{\"temperature\":%.2f,\"pressure\":%.2f,\"humidity\":%.1f,\"gas\":%.1f }}", "Hello World", temperature, pressure, humidity, gas);
    this->client->publish(mqttBMETopic, message);
}

void MqttClient::publishBMEState(BME680_IAQ_Data *data) 
{
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
  doc["iaqLevel"] = data->iaqLevel;
  doc["pressure"] = data->pressure;
  doc["rawHumidity"] = data->rawHumidity;
  doc["rawTemperature"] = data->rawTemperature;
  doc["runInStatus"] = data->runInStatus;
  doc["stabStatus"] = data->stabStatus;
  doc["staticIaq"] = data->staticIaq;
  doc["staticIaqAccuracy"] = data->staticIaqAccuracy;
  doc["temperature"] = data->temperature;
  serializeJson(doc, message);

  this->client->publish(mqttBMETopic, message);
}

void MqttClient::publishESState(float temperature, float humidity)
{
  char message[4096];
  DynamicJsonDocument doc(4096);
  doc["temperature"] = temperature;
  doc["humidity"] = humidity;
  serializeJson(doc, message);

  this->client->publish(mqttESTopic, message);
}

void MqttClient::publishSystemState()
{
  char message[4096];
  DynamicJsonDocument doc(4096);

  doc["BME280Equipped"] = BME280Equipped;
  if (BME280Equipped) {
    doc["BME280Online"] = BME280Online;
  }

  doc["CCS811Equipped"] = CCS811Equipped;
  if (CCS811Equipped) {
    doc["CCS811Online"] = CCS811Online;
  }

  doc["BME680Equipped"] = BME680Equipped;
  if (BME680Equipped) {
    doc["BME680Online"] = BME680Online;
    doc["BSECErrorCode"] = BSECErrorCode;
    doc["BSECWarningCode"] = BSECWarningCode;
    doc["BMEErrorCode"] = BMEErrorCode;
    doc["BMEWarningCode"] = BMEWarningCode;
  }

  serializeJson(doc, message);

  if (strcmp(message, systemStateMessageLc) != 0 || (millis() - lastStatePublished) > 30000)
  {
    strcpy(systemStateMessageLc, message);

    // now add json properties which should not be diffed
    doc["WiFiRSSI"] = WiFiRSSI;
    serializeJson(doc, message);

    lastStatePublished = millis();
    this->client->publish(mqttSystemTopic, message);
  }
}