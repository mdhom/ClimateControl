#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "MqttClient.h"
#include "PreferencesManager.h"

char mqttBMETopic[4][128];
char mqttESTopic[128];
char mqttSystemTopic[128];
char mqttErrorTopic[128];
char mqttConfigTopic[128];
char mqttConfigSetTopic[128];
char mqttConfigGetTopic[128];
char mqttSetFanTopic[128];

char systemStateMessageLc[4096];
int lastStatePublished = millis();

MqttClient::MqttClient(PubSubClient *pubsub, PreferencesManager *preferences) 
{
    this->client = pubsub;
    this->preferencesManager = preferences;
}

void MqttClient::begin(IPAddress *broker, const char *mqttTopic, const char *deviceIdentifier) 
{
    this->DeviceIdentifier = deviceIdentifier;
    this->client->setServer(*broker, 1883);

    using std::placeholders::_1;
    using std::placeholders::_2;
    using std::placeholders::_3;
    this->client->setCallback(std::bind(&MqttClient::MessageReceived, this, _1, _2, _3));
        
    for (int i=0; i < 4; i++)
    {
      sprintf(mqttBMETopic[i], "%s/%s/BME/%i", mqttTopic, deviceIdentifier, i);
      Serial.println("mqttBMETopic:       " + String(mqttBMETopic[i]));
    }
        
    sprintf(mqttESTopic, "%s/%s/ES", mqttTopic, deviceIdentifier);
    Serial.println("mqttESTopic:        " + String(mqttESTopic));
        
    sprintf(mqttSystemTopic, "%s/%s", mqttTopic, deviceIdentifier);
    Serial.println("mqttSystemTopic:    " + String(mqttSystemTopic));

    sprintf(mqttErrorTopic, "%s/%s/error", mqttTopic, deviceIdentifier);
    Serial.println("mqttErrorTopic:     " + String(mqttErrorTopic));

    sprintf(mqttConfigTopic, "%s/%s/config", mqttTopic, deviceIdentifier);
    Serial.println("mqttConfigTopic:    " + String(mqttConfigTopic));

    sprintf(mqttConfigSetTopic, "%s/%s/config/set", mqttTopic, deviceIdentifier);
    Serial.println("mqttConfigSetTopic: " + String(mqttConfigSetTopic));

    sprintf(mqttConfigGetTopic, "%s/%s/config/get", mqttTopic, deviceIdentifier);
    Serial.println("mqttConfigGetTopic: " + String(mqttConfigGetTopic));

    sprintf(mqttSetFanTopic, "%s/%s/fan/set", mqttTopic, deviceIdentifier);
    Serial.println("mqttSetFanTopic:    " + String(mqttSetFanTopic));
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
      this->client->subscribe(mqttConfigGetTopic);
      this->client->subscribe(mqttConfigSetTopic);
      this->client->subscribe(mqttSetFanTopic);
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
  Serial.print("Received topic \""); Serial.print(topic); Serial.println("\" via MQTT");
  if (strcmp(topic, mqttConfigSetTopic) == 0)
  {
    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, payload);

    switch (error.code()) {
      case DeserializationError::Ok:
        handleConfigMessage(&doc);
        break;
      case DeserializationError::InvalidInput:
        publishError(F("Invalid input!"));
        break;
      case DeserializationError::NoMemory:
        publishError(F("Not enough memory"));
        break;
      default:
        publishError(F("Deserialization failed"));
        break;
    }
  }
  else if(strcmp(topic, mqttConfigGetTopic) == 0)
  {
    publishConfig();
  }
  else if(strcmp(topic, mqttSetFanTopic) == 0)
  {
    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, payload);

    switch (error.code()) {
      case DeserializationError::Ok:
        handleSetFanMessage(&doc);
        break;
      case DeserializationError::InvalidInput:
        publishError(F("Invalid input!"));
        break;
      case DeserializationError::NoMemory:
        publishError(F("Not enough memory"));
        break;
      default:
        publishError(F("Deserialization failed"));
        break;
    }
  }
}

void MqttClient::handleSetFanMessage(StaticJsonDocument<200> *doc) {
  this->FanInSetValue = max(0, min(100, (*doc)["fanIn"].as<int>()));
  this->FanOutSetValue = max(0, min(100, (*doc)["fanOut"].as<int>()));
}

void MqttClient::handleConfigMessage(StaticJsonDocument<200> *doc) {
  int publishInterval = (*doc)["publishInterval"].as<uint>();
  if (publishInterval <= 0) {
    publishError("Publish interval must be greater than zero");
    return;
  }
  preferencesManager->SetPublishInterval(publishInterval);

  publishConfig();
}

void MqttClient::publishError(String error)
{
  char message[128];
  sprintf(message, "{\"message\": \"%s\" }", error.c_str());
  this->client->publish(mqttErrorTopic, message);
}

void MqttClient::publishConfig()
{
  char message[4096];
  DynamicJsonDocument doc(4096);
  doc["publishInterval"] = preferencesManager->GetPublishInterval();
  serializeJson(doc, message);

  this->client->publish(mqttConfigTopic, message);
}

void MqttClient::publishBMEState(int bmeIndex, BME680_IAQ_Data *data) 
{
  char message[4096];
  DynamicJsonDocument doc(4096);
  doc["index"] = bmeIndex;
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

  this->client->publish(mqttBMETopic[bmeIndex], message);
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

  int numBME = sizeof(BME680Online) / sizeof(BME680Online[0]);
  for (int i=0; i < numBME; i++){
    JsonObject bme = doc.createNestedObject("BME-"+String(i));
    bme["BME680Online"] = BME680Online[i];
    bme["BSECErrorCode"] = BSECErrorCode[i];
    bme["BSECWarningCode"] = BSECWarningCode[i];
    bme["BMEErrorCode"] = BMEErrorCode[i];
    bme["BMEWarningCode"] = BMEWarningCode[i];
  }

  doc["FanInSetValue"] = FanInSetValue;
  doc["FanOutSetValue"] = FanOutSetValue;

  serializeJson(doc, message);

  if (strcmp(message, systemStateMessageLc) != 0 || (millis() - lastStatePublished) > 10000)
  {
    strcpy(systemStateMessageLc, message);

    // now add json properties which should not be diffed
    doc["WiFiRSSI"] = WiFiRSSI;
    

    serializeJson(doc, message);

    lastStatePublished = millis();
    this->client->publish(mqttSystemTopic, message);
  }
}