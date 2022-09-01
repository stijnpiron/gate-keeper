#include <Arduino.h>
#include "RgbLed.h"

int RgbLed::BLUE[3] = { 0, 0, 205 };
int RgbLed::GREEN[3] = { 0, 200, 0 };
int RgbLed::MAGENTA[3] = { 255, 0, 255 };
int RgbLed::ORANGE[3] = { 255, 69, 0 };
int RgbLed::RED[3] = { 255, 0, 0 };
int RgbLed::WHITE[3] = { 255, 255, 255 };
int RgbLed::YELLOW[3] = { 255, 255, 0 };

bool RgbLed::COMMON_ANODE = true;
bool RgbLed::COMMON_CATHODE = false;

RgbLed::RgbLed(int red_pin, int green_pin, int blue_pin, bool common)
  : _red_pin(red_pin), _green_pin(green_pin), _blue_pin(blue_pin), _common(common), _brightness(100), _red(0), _green(0), _blue(0) {
#if defined(ESP32)
  ledcSetup(0, 5000, 8);
  ledcSetup(1, 5000, 8);
  ledcSetup(2, 5000, 8);

  ledcAttachPin(_red_pin, 0);
  ledcAttachPin(_green_pin, 1);
  ledcAttachPin(_blue_pin, 2);

#else
  pinMode(_red_pin, OUTPUT);
  pinMode(_green_pin, OUTPUT);
  pinMode(_blue_pin, OUTPUT);
#endif
}

void RgbLed::off() {
  color(0, 0, 0);
}

void RgbLed::setBrightness(int rgb[3], int brightness) {
  _brightness = brightness;
  intensity(rgb[0], rgb[1], rgb[2], brightness);
}

void RgbLed::setBrightness(int red, int green, int blue, int brightness) {
  intensity(red, green, blue, brightness);
}

void RgbLed::setBrightness(int brightness) {
  _brightness = checkValue(brightness, 0, 100);
}

void RgbLed::intensity(int red, int green, int blue, int brightness) {
  setBrightness(brightness);

  red = (red * _brightness) / 100;
  green = (green * _brightness) / 100;
  blue = (blue * _brightness) / 100;

  color(red, green, blue);
}

void RgbLed::setColor(int rgb[3]) {
  intensity(rgb[0], rgb[1], rgb[2], _brightness);
}

void RgbLed::setColor(int red, int green, int blue) {
  intensity(red, green, blue, _brightness);
}

void RgbLed::setColor(int rgb[3], int brightness) {
  intensity(rgb[0], rgb[1], rgb[2], checkValue(brightness, 0, 100));
}

void RgbLed::setColor(int red, int green, int blue, int brightness) {
  intensity(red, green, blue, checkValue(brightness, 0, 100));
}

void RgbLed::color(int red, int green, int blue) {
  _red = checkValue(red, 0, 255);
  _green = checkValue(green, 0, 255);
  _blue = checkValue(blue, 0, 255);

  if (_common == COMMON_ANODE) {
#if defined(ESP32)
    ledcWrite(0, 255 - red);
    ledcWrite(1, 255 - green);
    ledcWrite(2, 255 - blue);
#else
    analogWrite(_red_pin, 255 - red);
    analogWrite(_green_pin, 255 - green);
    analogWrite(_blue_pin, 255 - blue);
#endif
  } else {
#if defined(ESP32)
    ledcWrite(0, red);
    ledcWrite(1, green);
    ledcWrite(2, blue);
#else
    analogWrite(_red_pin, red);
    analogWrite(_green_pin, green);
    analogWrite(_blue_pin, blue);
#endif
  }
}

int RgbLed::checkValue(int value, int min, int max) {
  if (value < min) {
    return min;
  } else if (value > max) {
    return max;
  } else {
    return value;
  }
}

int RgbLed::checkValue(int value) {
  return checkValue(value, 0, 255);
}