// Copyright 2020, https://github.com/PatWie/iot-boilerplate
#include <memory>

#include "ArduinoJson.h"
#include "DNSServer.h"
#include "ESP8266HTTPClient.h"
#include "ESP8266WebServer.h"
#include "ESP8266WiFi.h"
#include "WiFiManager.h"

// Own header files.
#include <Api.h>

constexpr int operator"" _sec(long double ms) { return 1.0_sec * ms; }

// This is tested on an ESP12-F board. And the LED is a different GPIO.
const uint8_t ESP21F_LED = 2;

void configuration_mode_callback(WiFiManager *wifi_manager) {
  Serial.println("Entered configuration mode");
  Serial.println(WiFi.softAPIP());
  Serial.print("Created configuration portal AP ");
  Serial.println(wifi_manager->getConfigPortalSSID());
}

void Ping(WiFiClient *wifi_client) { Api::Routes::Ping(wifi_client)->end(); }

void SendMessage(WiFiClient *wifi_client) {
  // Create message to send.
  Api::Request::Message message;
  message.info = 12;
  strncpy(message.key, "some_key\0", 9);

  // Send message.
  auto http_client = Api::Routes::PostMessage(wifi_client, message);

  // Handle reponse.
  Api::Response::Message response;
  Api::Response::Bind(&http_client->getStream(), &response);
  Api::Response::Print(response);

  http_client->end();
}

void SendSecureMessage(WiFiClient *wifi_client) {
  // Create message to send.
  Api::Request::Message message;
  message.info = 42;
  strncpy(message.key, "secure_key\0", 11);

  // Send message.
  auto http_client = Api::Routes::PostSecureMessage(wifi_client, message);

  // Handle reponse.
  Api::Response::Message response;
  Api::Response::Bind(&http_client->getStream(), &response);
  Api::Response::Print(response);

  http_client->end();
}

void Blink(const int times) {
  for (int i = 0; i < times; i++) {
    digitalWrite(ESP21F_LED, LOW);
    delay(0.2_sec);
    digitalWrite(ESP21F_LED, HIGH);
    delay(0.2_sec);
  }

  delay(1.0_sec);
}

void setup() {
  Serial.begin(115200);

  pinMode(ESP21F_LED, OUTPUT);

  // Local initialization. Once its business is done, there is no need to keep
  // it around.
  WiFiManager wifi_manager;

  // When connecting to WiFi fails open the configuration portal.
  wifi_manager.setAPCallback(configuration_mode_callback);

  if (!wifi_manager.autoConnect()) {
    Serial.println("failed to connect and hit timeout");
    // reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(1.0_sec);
  }

  // TODO(patwie): Add logic for deep sleep and fail-safe re-connection. In
  // some cases we want to prevent the configuration portal to pop up if the
  // WiFi has been lost.
}

void loop() {
  // Connect to network.
  WiFiClient wifi_client;
  if (!Api::IsConnected(&wifi_client)) {
    wifi_client.stop();
    delay(5.0_sec);
    return;
  }

  Ping(&wifi_client);
  Blink(2);
  delay(2.0_sec);

  SendMessage(&wifi_client);
  Blink(3);
  delay(2.0_sec);

  SendSecureMessage(&wifi_client);
  Blink(3);
  delay(2.0_sec);

  wifi_client.stop();
}
