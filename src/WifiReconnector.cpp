#include <stdio.h>
#include <Arduino.h>
#include <WiFi.h>
#include "WifiReconnector.h" 

int lastStatus = WL_DISCONNECTED;

void WifiReconnector::begin(const char *ssid, const char *password)
{
    this->SSID = ssid;
    this->Password = password;

    Serial.println();
    Serial.print("Connecting to: ");
    Serial.println(ssid);

    WiFi.begin(this->SSID, this->Password);
}

bool WifiReconnector::isConnected()
{
    int status = WiFi.status();
    if (status != lastStatus) 
    {
        Serial.print("WiFi Status changed from ");
        Serial.print(toReadableWifiState(lastStatus));
        Serial.print(" to ");
        Serial.println(toReadableWifiState(status));

        if (lastStatus == WL_CONNECTED && (status == WL_CONNECTION_LOST || status == WL_DISCONNECTED))
        {
            Serial.println("Lost WiFi-Connection, reconnecting...");
            WiFi.begin(this->SSID, this->Password);
        }

        lastStatus = status;
    }

    return status == WL_CONNECTED;
}

String WifiReconnector::toReadableWifiState(int state)
{
    switch (state)
    {
        case WL_NO_SHIELD:
            return "No shield";
        case WL_IDLE_STATUS:
            return "Idle";
        case WL_NO_SSID_AVAIL:
            return "No SSID available";
        case WL_SCAN_COMPLETED:
            return "Scan completed";
        case WL_CONNECTED:
            return "Connected";
        case WL_CONNECT_FAILED:
            return "Connect failed";
        case WL_CONNECTION_LOST:
            return "Connection lost";
        case WL_DISCONNECTED:
            return "Disconnected";
        default:
            return "Unknown";
    }
}