/*
 * Gate Checker v2 uses an RGB LED for status feedback from
 * reed switches. The switches are installed on a gate, 
 * when the gate opens, the state of the sensor switches
 * and the feedback_state variable triggers the blinking
 * of the RGB LED in the appropriate color. 
 * 
 * OTA updates are supported to update the device without 
 * the need to dismount or dismantle it from the installation.
 * A working WiFi connection is required, make sure to set 
 * the WiFi settngs to the correct values for the network
 * used.
 * 
 * v2 also provides the ability to read the status of the 
 * gates through REST services, working on this.
 * REST paths available:
 * - GET - /api: OpenAPI json
 * - GET - /api/status: status of both gates
 * - GET - /api/status?gate=[:gate]: status of requested gate
 * - POST - /api/closeGate: close both gates
 * - POST - /api/closeGate/?gate=[:gate]: close given gate
 * 
 * TODO: 
 * - add logging instead of serial output
 * - make loggng requestable through REST service
 * - add possibility to send open/close commands to the gates
 */

#include <ArduinoOTA.h>
#include "basicOTA.h"
#include "RgbLed.h"

#define DEBUG 1 //comment/uncomment this line to disable/enable debug logging to serial monitor

/* Global variables */

// RGB LED configuration
const int red_light_pin= 1; // RED wire
const int green_light_pin = 2; // GREEN wire
const int blue_light_pin = 3; // BLUE wire

RgbLed* led;

// Reed switch pin configuration
const int front_switch_pin = 5; // BROWN wire
const int rear_switch_pin = 4; // ORANGE wire

// Time configuration
const int blink_time = 1000;
const int blink_offset = 1000;
const int closed_offset = 1000;
const int closed_blink_time = 1000;

// Set WiFi credentials
String WIFI_SSID = "UnifIOT";
String WIFI_PASS = "VRRfGsK1WM4fDrf-lLorSMsrwm74neR2";

/* 
 * variable that stores the feedback state of the switches: 
 * - 0: gates are closed
 * - 1: front gate is opened
 * - 2: rear gate is opened
 * - 3: both gates are open
 */

int feedback_state = 0;

// initialize application
void setup()
{
  #ifdef DEBUG
  Serial.begin(115200);
  #endif
  
  // initialize pin configuration
  pinMode(front_switch_pin, INPUT);
  pinMode(rear_switch_pin, INPUT);

  // Begin WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  #ifdef DEBUG
  Serial.println("booting up...");
  #endif
  
  // Loop continuously while WiFi is not connected
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    #ifdef DEBUG
    Serial.println("error connecting to WiFi");
    #endif
    delay(1000);
    ESP.restart();
  }

  // Connected to WiFi
  #ifdef DEBUG
  Serial.print("Connected. IP address: ");
  Serial.println(WiFi.localIP());
  #endif

  // Setup Firmware update over the air (OTA)
  setup_OTA();

  // setup RGB LED
  led = new RgbLed(red_light_pin, green_light_pin, blue_light_pin);
}

// run the program
void loop()
{
  // check for OTA updates
  ArduinoOTA.handle();

  // perform Gate Checker actions
  set_feedback_state();
  blink_feedback_led();
  delay(30000);
}

/*
 * Check the state of the reed switches and update the 
 * variable accordingly
 */
void set_feedback_state(){
  // reset on the beginning of every check
  feedback_state = 0; 

  // check the gates and update the feedback variable
  if(digitalRead(front_switch_pin) == LOW){
    feedback_state += 1;
  }
  if(digitalRead(rear_switch_pin) == LOW){
    feedback_state += 2;
  }
}

// let the feedback LED blink by checking the variable
void blink_feedback_led(){
  #ifdef DEBUG
  String message = "feedback_state: ";
  message += feedback_state;
  
  Serial.println(message);
  #endif
  
  switch(feedback_state){
    // no gates are open
    case 0:
      blink_closed();
      break;
    // only the front gate is open
    case 1:
      blink_blue();
      break;
    // only the rear gate eis open
    case 2:
      blink_orange();
      break;
    // both gates are open
    case 3:
      blink_blue();
      blink_orange();
      break;
  }
}

//// switch the LED off
//void led_off(int delayTime=blink_offset){
//  // "black"as color so the LED does not produce any light
//  led->blink(0, 0, 0, closed_blink_time, closed_offset);
//}

// the LED is on in the color BLUE
void blink_blue(){
  #ifdef DEBUG
  String message = "blink_blue();";
  message += "\n";
  message += "led->blinkled(0, 0, 255, blink_time, blink_offset);";
  
  Serial.println(message);
  #endif
  
  led->blinkled(0, 0, 255, blink_time, blink_offset);
}

// the LED is on in the color ORANGE
void blink_orange(){
  #ifdef DEBUG
  String message = "blink_orange();";
  message += "\n";
  message += "led->blinkled(255, 165, 0, blink_time, blink_offset);";
  
  Serial.println(message);
  #endif
  
  led->blinkled(255, 165, 0, blink_time, blink_offset);
}

// when the gates are both closed, blink GREEN
void blink_closed(){
  #ifdef DEBUG
  String message = "blink_closed();";
  message += "\n";
  message += "led->blinkled(0, 255, 0, closed_blink_time, closed_offset);";
  
  Serial.println(message);
  #endif
  led->blinkled(0, 255, 0, closed_blink_time, closed_offset);
}
