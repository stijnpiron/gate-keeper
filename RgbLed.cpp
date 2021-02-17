/*
   RgbLed.cpp - Library to control RGB LED's
*/

#import <Arduino.h>
#include "RgbLed.h"

 #define DEBUG 1 //comment/uncomment this line to disable/enable debug logging to serial monitor

RgbLed::RgbLed(int red_pin, int green_pin, int blue_pin)
{
  #ifdef DEBUG
  String message = "RgbLed::RgbLed(";
  message += red_pin;
  message += ", ";
  message += green_pin;
  message += ", ";
  message += blue_pin;
  message += ")";
  
  Serial.println(message);
  #endif
  
  RgbLed(red_pin, green_pin, blue_pin, 0, 0, 0);
}

RgbLed::RgbLed(int red_pin, int green_pin, int blue_pin, int red, int green, int blue)
{
  #ifdef DEBUG
  String message = "RgbLed::RgbLed(";
  message += red_pin;
  message += ", ";
  message += green_pin;
  message += ", ";
  message += blue_pin;
  message += ", ";
  message += red;
  message += ", ";
  message += green;
  message += ", ";
  message += blue;
  message += ")";
  
  Serial.println(message);
  #endif
  
  pinMode(red_pin, OUTPUT);
  pinMode(green_pin, OUTPUT);
  pinMode(blue_pin, OUTPUT);
  _red_pin = red_pin;
  _green_pin = green_pin;
  _blue_pin = blue_pin;
  _red = red;
  _green = green;
  _blue = blue;
}

void RgbLed::blinkled(int timeValue) 
{
  #ifdef DEBUG
  String message = "RgbLed::blinkled(";
  message += timeValue;
  message += ")";
  
  Serial.println(message);
  #endif
  
  analogWrite(_red_pin, _red);
  analogWrite(_green_pin, _green);
  analogWrite(_blue_pin, _blue);
  delay(timeValue);
  ledOff();
}

void RgbLed::blinkled(int red, int green, int blue, int timeValue, int timeOffset){
  #ifdef DEBUG
  String message = "RgbLed::blinkled(";
  message += red;
  message += ", ";
  message += green;
  message += ", ";
  message += blue;
  message += ", ";
  message += timeValue;
  message += ", ";
  message += timeOffset;
  message += ")";
  
  Serial.println(message);
  #endif
  
  setColor(red, green, blue);
  blinkled(timeValue);
  ledOff(timeOffset);
}

void RgbLed::ledOff()
{
  #ifdef DEBUG
  String message = "RgbLed::ledOff()";
  
  Serial.println(message);
  #endif
  
  ledOff(0);
}

void RgbLed::ledOff(int timeValue)
{
  #ifdef DEBUG
  String message = "RgbLed::ledOff(";
  message += timeValue;
  message += ")";
  
  Serial.println(message);
  #endif
  
  setColor();
  delay(timeValue);
}

void RgbLed::setColor()
{
  #ifdef DEBUG
  String message = "RgbLed::setColor()";
  
  Serial.println(message);
  #endif
  
  setColor(0, 0, 0);
}

void RgbLed::setColor(int red, int green, int blue)
{
  #ifdef DEBUG
  String message = "RgbLed::setColor(";
  message += red;
  message += ", ";
  message += green;
  message += ", ";
  message += blue;
  message += ")";
  
  Serial.println(message);
  #endif
  
  _red = red;
  _green = green;
  _blue = blue;
}
