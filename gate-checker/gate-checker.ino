#include "Arduino.h"
#include "RgbLed.h"

// set led pins
#define RED_LED_PIN 9    // D8
#define GREEN_LED_PIN 8  // D4
#define BLUE_LED_PIN 7   // D3

// set switch pins
#define FRONT_REED_PIN 6  // A0
#define REAR_REED_PIN 5   // D5

// set timings
const int open_on = 500;
const int open_off = 1000;
const int closed_on = 250;
const int closed_off = 29750;

// keep track of time
unsigned long currentMillis = 0;
unsigned long previousFrontLedMillis = 0;
unsigned long previousRearLedMillis = 0;
unsigned long previousOpenLedMillis = 0;
unsigned long previousClosedLedMillis = 0;

// keep track of LED state
byte frontLedState = LOW;
byte rearLedState = LOW;
byte openLedState = LOW;
byte closedLedState = LOW;

// set LED pins
RgbLed status_led(RED_LED_PIN, GREEN_LED_PIN, BLUE_LED_PIN, RgbLed::COMMON_CATHODE);

// Global variables
int feedback_state = 0;

void setup() {
  // set pin modes
  pinMode(FRONT_REED_PIN, INPUT);
  pinMode(REAR_REED_PIN, INPUT);
}

void loop() {
  // set current time used for timing events
  currentMillis = millis();
  // check gate status
  set_status();
  // blink the led accordingly
  blink_feedback_led();
}

void set_status() {
  /* 
  * variable that stores the feedback state of the switches: 
  * - 0: gates are closed
  * - 1: front gate is opened
  * - 2: rear gate is opened
  * - 3: both gates are open
  */

  // reset state
  feedback_state = 0;

  // check the gates and update the feedback variable
  if (digitalRead(FRONT_REED_PIN) == LOW) {
    feedback_state += 1;
  }
  if (digitalRead(REAR_REED_PIN) == LOW) {
    feedback_state += 2;
  }
}

// let the feedback LED blink by checking the variable
void blink_feedback_led() {
  switch (feedback_state) {
    // no gates are open
    case 0:
      blink_closed();
      break;
    // only the front gate is open
    case 1:
      blink_front();
      break;
    // only the rear gate is open
    case 2:
      blink_rear();
      break;
    // both gates are open
    case 3:
      blink_both();
      break;
    default:
      status_led.off();
  }
}

void blink_front() {
  blink(RgbLed::BLUE, status_led, frontLedState, previousFrontLedMillis, open_on, open_off);
}

void blink_rear() {
  blink(RgbLed::ORANGE, status_led, rearLedState, previousRearLedMillis, open_on, open_off);
}

void blink_both() {
  blink(RgbLed::BLUE, status_led, openLedState, previousOpenLedMillis, open_on, open_on, RgbLed::ORANGE);
}

void blink_closed() {
  blink(RgbLed::GREEN, status_led, closedLedState, previousClosedLedMillis, closed_on, closed_off);
}

void blink(int (&color)[3], RgbLed& led, byte& led_state, unsigned long& ledMillis, int time_on, int time_off) {
  blink(color, led, led_state, ledMillis, time_on, time_off, RgbLed::BLANK);
}

void blink(int (&color)[3], RgbLed& led, byte& led_state, unsigned long& ledMillis, int time_on, int time_off, int (&color2)[3]) {
  if (led_state == LOW) {
    if (currentMillis - ledMillis >= time_off) {
      led.setColor(color);
      led_state = HIGH;
      ledMillis += time_off;
    }
  } else {
    if (currentMillis - ledMillis >= time_on) {
      if (color2 == RgbLed::BLANK) {
        led.off();
      } else {
        led.setColor(color2);
      }
      led_state = LOW;
      ledMillis += time_on;
    }
  }
}