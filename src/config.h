#include "WiFi.h"
#include <PubSubClient.h>
#include <ArduinoJson.h>

const char* roomIdentifier              = "Dachboden";
const char* mqttTopic                   = "airControl";

const char* mqttWriteTopicSuffix        = "write";
const char* mqttStateRequestSuffix      = "get";
const char* mqttSetFlowSuffix           = "set";
const char* mqttErrorSuffix             = "error";

#define PUBLISH_INTERVAL 5000

#define STORE_IN_FLASH

/* MQTT Settings */
IPAddress broker(192,168,188,149);                     // Address of the MQTT broker
//#define USE_MQTT_AUTH
//#define MQTT_USER "USERNAME"
//#define MQTT_PASSWORD "PASSWORD"

/* WiFi Settings */
const char* ssid     = "MMD";
const char* password = "MMD%83139%";

const int NumBME = 4;
/* Sensor Settings */
//#define USE_BME680_IAQ
//#define USE_BME680
//#define USE_ENVIRONMENTALSENSOR
#define USE_MUX