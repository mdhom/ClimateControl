#ifndef WIFIRECONNECTOR_h
#define WIFIRECONNECTOR_h

#include <stdio.h>

class WifiReconnector{
public:
    const char *SSID;
    const char *Password;
    int8_t RSSI;
    
    void begin(const char *ssid, const char *password);
    bool isConnected();
private:
    String toReadableWifiState(int state);
};

#endif
