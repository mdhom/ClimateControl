#ifndef OLED_h
#define OLED_h

#include <Arduino.h>

class OLED{
public:
    int firmwareVersion;
    void init();

    void showNoWlan();
    void showNoMqtt();
    void showOTAUpdate();
    void showWorking(int step);
private:
    void lineExample();
    void printTitle(String title, int font);
};

#endif
