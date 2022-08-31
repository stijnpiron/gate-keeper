#include <RGBLed.h>

// set led pins
#define RED_LED_PIN 5
#define GREEN_LED_PIN 4
#define BLUE_LED_PIN 0

// set switch pins
#define FRONT_REED_PIN 14
#define REAR_REED_PIN 12

// set relay pins
#define FRONT_RELAY_PIN 13
#define REAR_RELAY_PIN 2

// set timings
#define OPEN_ON 500
#define OPEN_OFF 1500
#define CLOSED_ON 250
#define CLOSED_OFF 29750
#define ERROR_ON 250
#define ERROR_OFF 250

// set LED pins
RGBLed status_led(RED_LED_PIN, GREEN_LED_PIN, BLUE_LED_PIN, RGBLed::COMMON_CATHODE);

// Global variables
int feedback_state = 0;

void setup() {
  Serial.begin(115200);
  pinMode(FRONT_REED_PIN, INPUT);
  pinMode(REAR_REED_PIN, INPUT);
  status_led.off();
}

void loop() {
  set_status();
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
    // errorr occurred during gate check or gate control
    case -1:
      blink_error();
      break;
  }
}

void blink_front() {
  status_led.flash(0, 0, 205, OPEN_ON, OPEN_OFF);
}

void blink_rear() {
  status_led.flash(255, 69, 0, OPEN_ON, OPEN_OFF);
}

void blink_both() {
  blink_front();
  blink_rear();
}

void blink_closed() {
  status_led.flash(0, 200, 0, CLOSED_ON, CLOSED_OFF);
}

void blink_error() {
  status_led.flash(255, 0, 0, ERROR_ON, ERROR_OFF);
}