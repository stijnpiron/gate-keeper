#include "Arduino.h"
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ArduinoJson.h>

#include "RgbLed.h"

#pragma region PARAMSETUP

// set led pins
#define RED_LED_PIN 15 // D8
#define GREEN_LED_PIN 2 // D4
#define BLUE_LED_PIN 0 // D3

// set switch pins
#define FRONT_REED_PIN A0 // A0
#define REAR_REED_PIN 14 // D5
#define FRONT_REED_PIN_OPEN 12 // D6
#define REAR_REED_PIN_OPEN 13 // D7

// set relay pins
#define FRONT_RELAY_PIN 5 // D1
#define REAR_RELAY_PIN 4 // D2

// set timings
const int open_on = 500;
const int open_off = 1000;
const int closed_on = 250;
const int closed_off = 29750;
const int control_on = 500;
const int control_off = 500;
const int error_on = 150;
const int error_off = 150;

// set wifi settings
#define WIFI_SSID "UnifIOT"
#define WIFI_PASSWORD "VRRfGsK1WM4fDrf-lLorSMsrwm74neR2"

// keep track of time
unsigned long currentMillis = 0;
unsigned long previousFrontLedMillis = 0;
unsigned long previousRearLedMillis = 0;
unsigned long previousOpenLedMillis = 0;
unsigned long previousClosedLedMillis = 0;
unsigned long previousControlLedMillis = 0;
unsigned long previousErrorLedMillis = 0;

// keep track of LED state
byte frontLedState = LOW;
byte rearLedState = LOW;
byte openLedState = LOW;
byte closedLedState = LOW;
byte controlLedState = LOW;
byte errorLedState = LOW;

// set LED pins
RgbLed status_led(RED_LED_PIN, GREEN_LED_PIN, BLUE_LED_PIN, RgbLed::COMMON_CATHODE);

// Global variables
int feedback_state = 0;

ESP8266WebServer server(80);

#pragma endregion

void setup() {
  // LED red dduring boot process
  status_led.setColor(RgbLed::RED);
  Serial.begin(115200);
  // set pin modes
  pinMode(FRONT_REED_PIN, INPUT);
  pinMode(REAR_REED_PIN, INPUT);
  pinMode(FRONT_RELAY_PIN, OUTPUT);
  pinMode(REAR_RELAY_PIN, OUTPUT);

  // setup wifi
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  // Wait for WiFi connection
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(350);
  }
  
  // print WiFi properties
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(WIFI_SSID);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Activate mDNS this is used to be able to connect to the server with the DNS name
  if (MDNS.begin("gatechecker")) {
    Serial.println("MDNS responder started");
  }

  // Set webserver routing
  restServerRouting();
  // Set not found response
  server.onNotFound(handleNotFound);
  // Start webserver
  server.begin();
  Serial.println("Webserver started");

  status_led.off();
}

void loop() {
  // set current time used for timing events
  currentMillis = millis();
  // check gate status
  set_status();
  // blink the led accordingly
  blink_feedback_led();
  // handle any web requests
  server.handleClient();
}

#pragma region WEBSERVER

// Define webserver routing
void restServerRouting() {
  server.on("/", HTTP_GET, []() {
    server.send(200, "text/html", "Gate cheker API, check status on: http://" + WiFi.localIP().toString() + "/status");
  });
  server.on("/status", HTTP_GET, getStatus);
  server.on("/close", HTTP_POST, closeGate);
}

// serving settings
void getStatus() {
  DynamicJsonDocument doc(512);

  doc["ip"] = WiFi.localIP().toString();
  doc["gw"] = WiFi.gatewayIP().toString();
  doc["nm"] = WiFi.subnetMask().toString();
  doc["signalStrengh"] = WiFi.RSSI();
  doc["chipId"] = ESP.getChipId();
  doc["flashChipId"] = ESP.getFlashChipId();
  doc["flashChipSize"] = ESP.getFlashChipSize();
  doc["flashChipRealSize"] = ESP.getFlashChipRealSize();
  doc["freeHeap"] = ESP.getFreeHeap();

  Serial.print(F("Stream..."));
  String buf;
  serializeJson(doc, buf);
  server.send(200, F("application/json"), buf);
  Serial.println(F("done."));
}

// Manage not found URL
void handleNotFound() {
  String message = "Location Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

#pragma endregion

#pragma region STATUSLED

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
  // as we need to read from an analog pin, we check if the reading is less or more than 1000, 
  // digital HIGH would result in +/-1024, digital low in +/-18
  if (analogRead(FRONT_REED_PIN) < 1000) {
    feedback_state += 1;
  }
  if (analogRead(REAR_REED_PIN) < 1000) {
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
    // error occurred during gate check or gate control
    case -1:
      blink_error();
      break;
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

void blink_error() {
  blink(RgbLed::RED, status_led, errorLedState, previousErrorLedMillis, error_on, error_off);
}

void blink_control() {
  blink(RgbLed::YELLOW, status_led, controlLedState, previousControlLedMillis, control_on, control_off);
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

#pragma endregion

#pragma region GATECONTROL

/**
 * Controlling the gate involves triggering a relay switch for the gate. 
 * Wether the comnmand is OPEN or CLOSE, the same action is performed, 
 * monitorring the gate status is different:
 * 
 * OPEN:
 * 1. trgger relay
 * 2. wait x seconds
 * 3. check gate status
 * 4. if status == OPEN, stop
 * 5. if status == CLOSED, repeat from step 1
 * max 2 repetitions, if not OPEN after 2 repetitions, engage ERROR state
 *
 * CLOSE:
 * 1. check gate status
 * 2. if status == CLOSED, stop
 * 3. if status == OPEN, trigger relay
 * 4. wait x seconds
 * 5. repeat from  step 1
 * max 3 repetitions, if not OPEN after 3 repetitions, engage ERROR state
 */
void closeGate() {
  String gate = server.arg("gate") == "" ? "all" : server.arg("gate");
  Serial.println("close gate..." + gate);
  if (gate == "front" || gate == "1") {
    closeFront();
    server.send(201, F("application/json"), "{\"status\":201,\"message\":\"Gate closed with identifier: " + gate + "\"}");
  } else if (gate == "rear" || gate == "2") {
    closeRear();
    server.send(201, F("application/json"), "{\"status\":201,\"message\":\"Gate closed with identifier: " + gate + "\"}");
  } else if (gate == "all") {
    closeFront();
    closeRear();
    server.send(201, F("application/json"), "{\"status\":201,\"message\":\"All gates closed\"}");
  } else {
    server.send(400, F("application/json"), "{\"status\":400,\"message\":\"No gate found with identifier: " + gate + "\"}");
    Serial.println("gate not found: " + gate);
  }
}

void closeFront() {
  digitalWrite(FRONT_RELAY_PIN, HIGH);
  delay(250);
  digitalWrite(FRONT_RELAY_PIN, LOW);
}

void closeRear() {
  digitalWrite(REAR_RELAY_PIN, HIGH);
  delay(250);
  digitalWrite(REAR_RELAY_PIN, LOW);
}

#pragma endregion