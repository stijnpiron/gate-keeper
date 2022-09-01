/*
 * RgbLed.h - Library to control RGB LED's
 */

#ifndef RgbLed_h
#define RgbLed_h

#include <Arduino.h>

class RgbLed {
public:
  RgbLed(int red_pin, int green_pin, int blue_pin, bool common);

  static int RED[3];
  static int BLUE[3];
  static int GREEN[3];
  static int MAGENTA[3];
  static int ORANGE[3];
  static int WHITE[3];
  static int YELLOW[3];

  static bool COMMON_ANODE;
  static bool COMMON_CATHODE;

  void off();
  void setColor(int rgb[3]);
  void setColor(int red, int green, int blue);
  void setColor(int rgb[3], int brightness);
  void setColor(int red, int green, int blue, int brightness);

private:
  int _red_pin;
  int _green_pin;
  int _blue_pin;
  int _common;
  int _brightness;
  int _red;
  int _green;
  int _blue;

  int checkValue(int value);
  int checkValue(int value, int min, int max);
  void color(int red, int green, int blue);
  void intensity(int red, int green, int blue, int brightness);
  void setBrightness(int rgb[3], int brightness);
  void setBrightness(int red, int green, int blue, int brightness);
  void setBrightness(int brightness);
};

#endif