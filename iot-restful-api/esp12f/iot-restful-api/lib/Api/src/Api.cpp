#include <Api.h>
#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>

#define USER_AGENT "esp8266"
#define REMOTE_HOST "sub.domain.tld"
#define REMOTE_PING_URL "http://" REMOTE_HOST "/ping"
#define REMOTE_MESSAGE_URL "http://" REMOTE_HOST "/v1/iot/messages"
#define REMOTE_SECURE_MESSAGE_URL                                              \
  "http://" REMOTE_HOST "/v1/iot/secure_messages"
#define JWT_TOKEN                                                              \
  "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJkZXZpY2VfaWQiOjEyM30.cgkvt-"        \
  "XXIl85Bi4NSqGS0YD-u_4gPkZkirRqNggJMCo"

namespace {
bool is_connected(WiFiClient *client) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("wifi is not connected");
    return false;
  }

  if (!client->connect(String(REMOTE_HOST), 80)) {
    Serial.println("remote host is not reachable");
    client->stop();
    return false;
  }

  return true;
}

String render(const Api::Request::Message &message) {
  const int capacity = JSON_OBJECT_SIZE(2);
  StaticJsonDocument<capacity> message_request;
  message_request["key"] = message.key;
  message_request["info"] = message.info;

  String encoded_message_request;
  serializeJson(message_request, encoded_message_request);
  return encoded_message_request;
}

void bind(Stream &stream, Api::Response::Message *message) {
  DynamicJsonDocument doc(2048);
  deserializeJson(doc, stream);

  message->id = doc["id"].as<int>();
  String key = doc["key"].as<String>();
  strncpy(message->key, key.c_str(), key.length());
  message->key[key.length()] = '\0';
  message->info = doc["info"].as<int>();
  message->counter = doc["counter"].as<int>();
  message->device_id = doc["device_id"].as<int>();
}

void print(const Api::Response::Message &message) {
  Serial.print("id: ");
  Serial.println(message.id);
  Serial.print("key: ");
  Serial.println(message.key);
  Serial.print("info: ");
  Serial.println(message.info);
  Serial.print("counter: ");
  Serial.println(message.counter);
  Serial.print("device_id: ");
  Serial.println(message.device_id);
  Serial.println();
}
};

namespace Api {

namespace Routes {
void ping() {

  WiFiClient client;
  if (!is_connected(&client)) {
    return;
  }

  HTTPClient http;
  http.begin(client, String(REMOTE_PING_URL));
  http.setUserAgent(String(USER_AGENT));
  http.GET();
  http.end();

  client.stop();
}

void post_message(const Request::Message &message) {

  WiFiClient client;
  if (!is_connected(&client)) {
    return;
  }

  HTTPClient http;
  http.begin(client, String(REMOTE_MESSAGE_URL));
  http.setUserAgent(String(USER_AGENT));
  http.addHeader("Content-Type", "application/json");
  http.POST(render(message));

  Api::Response::Message response;
  bind(http.getStream(), &response);
  print(response);

  http.end();

  client.stop();
}

void post_secure_message(const Request::Message &message) {

  WiFiClient client;
  if (!is_connected(&client)) {
    return;
  }

  HTTPClient http;
  http.begin(client, String(REMOTE_SECURE_MESSAGE_URL));
  http.setUserAgent(String(USER_AGENT));

  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization", String("Bearer ") + JWT_TOKEN);
  http.POST(render(message));

  Api::Response::Message response;
  bind(http.getStream(), &response);
  print(response);

  http.end();

  client.stop();
}

}; // namespace Routes

}; // namespace Api

#undef USER_AGENT
#undef REMOTE_HOST
#undef REMOTE_PING_URL
#undef REMOTE_MESSAGE_URL
#undef REMOTE_SECURE_MESSAGE_URL
#undef JWT_TOKEN
