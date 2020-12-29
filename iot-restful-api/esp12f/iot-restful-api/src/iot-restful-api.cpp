#include <ArduinoJson.h>
#include <DNSServer.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <WiFiManager.h>

#include <Api.h>

// This is tested on an ESP12-F board. And the LED is a different GPIO.
const uint8_t ESP21F_LED = 2;

void config_mode_callback(WiFiManager *wifi_manager) {
  Serial.println("Entered configuration mode");
  Serial.println(WiFi.softAPIP());
  Serial.print("Created configuration portal AP ");
  Serial.println(wifi_manager->getConfigPortalSSID());
}

void ping() { Api::Routes::ping(); }

void send_message() {
  Api::Request::Message message;
  message.info = 12;
  strncpy(message.key, "some_key\0", 9);

  Api::Routes::post_message(message);
}

void send_secure_message() {
  Api::Request::Message message;
  message.info = 42;
  strncpy(message.key, "secure_key\0", 11);

  Api::Routes::post_secure_message(message);
}

void blink(const int times) {
  for (int i = 0; i < times; i++) {
    digitalWrite(ESP21F_LED, LOW);
    delay(200);
    digitalWrite(ESP21F_LED, HIGH);
    delay(200);
  }

  delay(1000);
}

void setup() {
  Serial.begin(115200);

  pinMode(ESP21F_LED, OUTPUT);

  // Local initialization. Once its business is done, there is no need to keep
  // it around.
  WiFiManager wifiManager;

  // When connecting to WiFi fails open the configuration portal.
  wifiManager.setAPCallback(config_mode_callback);

  // TODO(patwie): Add logic for deep sleep and fail-safe re-connection. In
  // some cases we want to prevent the configuration portal to pop up if the
  // WiFi has been lost.
}

void loop() {

  ping();
  blink(2);
  delay(2000);

  send_message();
  blink(3);
  delay(2000);

  send_secure_message();
  blink(3);
  delay(2000);
}
