#include "WiFi.h"
#include <PubSubClient.h>
#include <ArduinoJson.h>

const char* roomIdentifier              = "kitchen";
const char* mqttTopic                   = "airControl";

const char* mqttWriteTopicSuffix        = "write";
const char* mqttStateRequestSuffix      = "get";
const char* mqttSetFlowSuffix           = "set";
const char* mqttErrorSuffix             = "error";

#define PUBLISH_INTERVAL 5000

#define STORE_IN_FLASH

/* MQTT Settings */
//IPAddress broker(192,168,188,149); // MQTT Broker Max
IPAddress broker(192,168,178,78); // MQTT Broker Leo
//#define USE_MQTT_AUTH
//#define MQTT_USER "USERNAME"
//#define MQTT_PASSWORD "PASSWORD"

/* WiFi Settings */
const char* ssid     = "heenalu 2,4";
const char* password = "86%FQcOXI!3L";

const int NumBME = 4;
/* Sensor Settings */
//#define USE_BME680_IAQ
//#define USE_BME680
//#define USE_ENVIRONMENTALSENSOR
#define USE_MUX