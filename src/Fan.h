#ifndef FAN_h
#define FAN_h

#include <Arduino.h>
#include <stdio.h>

class Fan{
public:
    double SetSpeed;
    double TachoSpeed;

    Fan(uint8_t pinPWM, int pwmChannel, double pwmFrequency, uint8_t pwmResolution);
    Fan(uint8_t pinPWM, int pwmChannel, double pwmFrequency, uint8_t pwmResolution, uint8_t pinTacho);

    void begin();
    void loop();
    bool setSpeed(double percentage);
private:
    uint8_t pinPWM;
    int pwmChannel;
    double pwmFrequency;
    uint8_t pwmResolution;

    uint8_t pinTacho;
};

#endif
