/*
 * RgbLed.h - Library to control RGB LED's
 */

#ifndef RgbLed_h
#define RgbLed_h

#import <Arduino.h>

class RgbLed
{
public:
  RgbLed(int red_pin, int green_pin, int blue_pin);
  RgbLed(int red_pin, int green_pin, int blue_pin, int red, int green, int blue);
  void blinkled(int timeValue);
  void blinkled(int red, int green, int blue, int timeValue, int timeOffset);
  void ledOff();
  void ledOff(int timeValue);
  void setColor();
  void setColor(int red, int green, int blue);
private:
  int _red_pin;
  int _green_pin;
  int _blue_pin;
  int _red;
  int _green;
  int _blue;
  bool _state;
};

#endif
