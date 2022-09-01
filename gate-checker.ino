#include "Arduino.h"
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ArduinoJson.h>

#include "RgbLed.h"

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
#define OPEN_OFF 1000
#define CLOSED_ON 250
#define CLOSED_OFF 29750
#define CONTROL_ON 500
#define CONTROL_OFF 500
#define ERROR_ON 150
#define ERROR_OFF 150

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

void setup() {
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

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(350);
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
  currentMillis = millis();
  set_status();
  blink_feedback_led();
  server.handleClient();
}

// Define routing
void restServerRouting() {
  server.on("/", HTTP_GET, []() {
    server.send(200, "text/html", "Gate cheker API, check status on: http://" + WiFi.localIP().toString() + "/status");
  });
  server.on("/status", HTTP_GET, getStatus);
  server.on("/close", HTTP_POST, closeGate);
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
  if (frontLedState == LOW) {
    if (currentMillis - previousFrontLedMillis >= OPEN_OFF) {
      status_led.setColor(RgbLed::BLUE);
      frontLedState = HIGH;
      previousFrontLedMillis += OPEN_OFF;
    }
  } else {
    if (currentMillis - previousFrontLedMillis >= OPEN_ON) {
      status_led.off();
      frontLedState = LOW;
      previousFrontLedMillis += OPEN_ON;
    }
  }
}

void blink_rear() {
  if (rearLedState == LOW) {
    if (currentMillis - previousRearLedMillis >= OPEN_OFF) {
      status_led.setColor(RgbLed::ORANGE);
      rearLedState = HIGH;
      previousRearLedMillis += OPEN_OFF;
    }
  } else {
    if (currentMillis - previousRearLedMillis >= OPEN_ON) {
      status_led.off();
      rearLedState = LOW;
      previousRearLedMillis += OPEN_ON;
    }
  }
}

void blink_both() {
  if (openLedState == LOW) {
    if (currentMillis - previousOpenLedMillis >= OPEN_ON) {
      status_led.setColor(RgbLed::BLUE);
      openLedState = HIGH;
      previousOpenLedMillis += OPEN_ON;
    }
  } else {
    if (currentMillis - previousOpenLedMillis >= OPEN_ON) {
      status_led.setColor(RgbLed::ORANGE);
      openLedState = LOW;
      previousOpenLedMillis += OPEN_ON;
    }
  }
}

void blink_closed() {
  if (closedLedState == LOW) {
    if (currentMillis - previousClosedLedMillis >= CLOSED_OFF) {
      status_led.setColor(RgbLed::GREEN);
      closedLedState = HIGH;
      previousClosedLedMillis += CLOSED_OFF;
    }
  } else {
    if (currentMillis - previousClosedLedMillis >= CLOSED_ON) {
      status_led.off();
      closedLedState = LOW;
      previousClosedLedMillis += CLOSED_ON;
    }
  }
}

void blink_error() {
  if (errorLedState == LOW) {
    if (currentMillis - previousErrorLedMillis >= ERROR_OFF) {
      status_led.setColor(RgbLed::RED);
      errorLedState = HIGH;
      previousErrorLedMillis += ERROR_OFF;
    }
  } else {
    if (currentMillis - previousErrorLedMillis >= ERROR_ON) {
      status_led.off();
      errorLedState = LOW;
      previousErrorLedMillis += ERROR_ON;
    }
  }
}

void blink_control() {
  status_led.setColor(RgbLed::YELLOW);
}

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
