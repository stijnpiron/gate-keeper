#include "Arduino.h"
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ArduinoJson.h>
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

// set wifi settings
#define WIFI_SSID "UnifIOT"
#define WIFI_PASSWORD "VRRfGsK1WM4fDrf-lLorSMsrwm74neR2"

// set LED pins
RGBLed status_led(RED_LED_PIN, GREEN_LED_PIN, BLUE_LED_PIN, RGBLed::COMMON_CATHODE);

// Global variables
int feedback_state = 0;

ESP8266WebServer server(80);

void setup() {
  Serial.begin(115200);
  // set pin modes
  pinMode(FRONT_REED_PIN, INPUT);
  pinMode(REAR_REED_PIN, INPUT);

  // setup wifi
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(WIFI_SSID);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Activate mDNS this is used to be able to connect to the server
  // with local DNS hostmane gatechecker.local
  if (MDNS.begin("gatechecker")) {
    Serial.println("MDNS responder started");
  }

  // Set server routing
  restServerRouting();
  // Set not found response
  server.onNotFound(handleNotFound);
  // Start server
  server.begin();
  Serial.println("HTTP server started");

  status_led.off();
}

void loop() {
  set_status();
  blink_feedback_led();
  server.handleClient();
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

// Define routing
void restServerRouting() {
  server.on("/", HTTP_GET, []() {
    server.send(200, "text/html", "Gate cheker API, check status on: http://" + WiFi.localIP().toString() + "/status");
  });
  server.on(F("/status"), HTTP_GET, getStatus);
}

// serving settings
void getStatus() {
  // Allocate a temporary JsonDocument
  // Don't forget to change the capacity to match your requirements.
  // Use arduinojson.org/v6/assistant to compute the capacity.
  //  StaticJsonDocument<512> doc;
  // You can use DynamicJsonDocument as well
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
  String message = "File Not Found\n\n";
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