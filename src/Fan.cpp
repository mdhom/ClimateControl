#include "Fan.h"

// maximum RPM on Fan = interval 25ms = 40Hz = 100%
// minimum RPM on Fan = interval 62ms = 16Hz = ~40%
const float tachoIntervalMin = 25;
const float tachoIntervalMax = 62;
const float tachoMinimumSpeed = 40; // if no PWM output, fan rotates with 16Hz

int tachoLastHigh = millis();
int tachoInterval = 0;
int tachoState = LOW;
int tachoStateLC = LOW;

Fan::Fan(uint8_t pinPWM, int pwmChannel, double pwmFrequency, uint8_t pwmResolution)
{
    this->pinPWM = pinPWM;
    this->pwmChannel = pwmChannel;
    this->pwmFrequency = pwmFrequency;
    this->pwmResolution = pwmResolution;
}

Fan::Fan(uint8_t pinPWM, int pwmChannel, double pwmFrequency, uint8_t pwmResolution, uint8_t pinTacho)
{
    this->pinPWM = pinPWM;
    this->pwmChannel = pwmChannel;
    this->pwmFrequency = pwmFrequency;
    this->pwmResolution = pwmResolution;
    this->pinTacho = pinTacho;
}

void Fan::begin()
{
  pinMode(pinPWM, OUTPUT);
  ledcSetup(pwmChannel, pwmFrequency, pwmResolution);
  ledcAttachPin(pinPWM, pwmChannel);

  if (pinTacho != 0) 
  {
      pinMode(pinTacho, INPUT_PULLUP);
  }
}

void Fan::loop()
{
  tachoState = digitalRead(pinTacho);
  if (tachoState == HIGH && tachoStateLC == LOW) {
    tachoInterval = millis() - tachoLastHigh;
    tachoLastHigh = millis();
    if (tachoInterval <= tachoIntervalMax && tachoInterval >= tachoIntervalMin)
    {
        TachoSpeed = 100.0 - (tachoInterval - tachoIntervalMin) * (100.0 - tachoMinimumSpeed) / (tachoIntervalMax - tachoIntervalMin);
    }
  }
  tachoStateLC = tachoState;  
}

bool Fan::setSpeed(double percentage)
{
    if (percentage != SetSpeed) {
        SetSpeed = percentage;
        ledcWrite(pwmChannel, ((SetSpeed - tachoMinimumSpeed) / (100.0 - tachoMinimumSpeed)) * 255); 

        return true;
    }

    return false;
}