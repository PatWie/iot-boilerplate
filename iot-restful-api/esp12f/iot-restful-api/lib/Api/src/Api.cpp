#include "Api.h"

#include "ArduinoJson.h"
#include "ESP8266HTTPClient.h"
#include "ESP8266WiFi.h"
#include "WiFiClient.h"

#define USER_AGENT "esp8266"
#define REMOTE_HOST "sub.domain.tld"
#define REMOTE_PING_URL "http://" REMOTE_HOST "/ping"
#define REMOTE_MESSAGE_URL "http://" REMOTE_HOST "/v1/iot/messages"
#define REMOTE_SECURE_MESSAGE_URL \
  "http://" REMOTE_HOST "/v1/iot/secure_messages"
#define JWT_TOKEN                                                       \
  "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJkZXZpY2VfaWQiOjEyM30.cgkvt-" \
  "XXIl85Bi4NSqGS0YD-u_4gPkZkirRqNggJMCo"

namespace {
template <typename T, typename... Args>
std::unique_ptr<T> make_unique(Args &&...args) {
  return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}
};  // namespace

namespace Api {

namespace Request {
String Render(const Api::Request::Message &message) {
  const int capacity = JSON_OBJECT_SIZE(2);
  StaticJsonDocument<capacity> message_request;
  message_request["key"] = message.key;
  message_request["info"] = message.info;

  String encoded_message_request;
  serializeJson(message_request, encoded_message_request);
  return encoded_message_request;
}
};  // namespace Request

namespace Response {
void Bind(Stream *stream, Api::Response::Message *message) {
  DynamicJsonDocument doc(2048);
  deserializeJson(doc, *stream);

  message->id = doc["id"].as<int>();
  String key = doc["key"].as<String>();
  strncpy(message->key, key.c_str(), key.length());
  message->key[key.length()] = '\0';
  message->info = doc["info"].as<int>();
  message->counter = doc["counter"].as<int>();
  message->device_id = doc["device_id"].as<int>();
}

void Print(const Api::Response::Message &message) {
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
};  // namespace Response

bool IsConnected(WiFiClient *client) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Wifi is not connected");
    return false;
  }

  if (!client->connect(String(REMOTE_HOST), 80)) {
    Serial.println("Remote host is not reachable");
    client->stop();
    return false;
  }

  return true;
}

namespace Routes {

std::unique_ptr<HTTPClient> Ping(WiFiClient *wifi_client) {
  std::unique_ptr<HTTPClient> http_client = make_unique<HTTPClient>();
  http_client->begin(*wifi_client, String(REMOTE_PING_URL));
  http_client->setUserAgent(String(USER_AGENT));
  http_client->GET();
  return http_client;
}

std::unique_ptr<HTTPClient> PostMessage(WiFiClient *wifi_client,
                                        const Request::Message &message) {
  std::unique_ptr<HTTPClient> http_client = make_unique<HTTPClient>();
  http_client->begin(*wifi_client, String(REMOTE_MESSAGE_URL));
  http_client->setUserAgent(String(USER_AGENT));
  http_client->addHeader("Content-Type", "application/json");
  http_client->POST(Render(message));
  return http_client;
}

std::unique_ptr<HTTPClient> PostSecureMessage(WiFiClient *wifi_client,
                                              const Request::Message &message) {
  std::unique_ptr<HTTPClient> http_client = make_unique<HTTPClient>();
  http_client->begin(*wifi_client, String(REMOTE_SECURE_MESSAGE_URL));
  http_client->setUserAgent(String(USER_AGENT));

  http_client->addHeader("Content-Type", "application/json");
  http_client->addHeader("Authorization", String("Bearer ") + JWT_TOKEN);
  http_client->POST(Render(message));
  return http_client;
}

};  // namespace Routes

};  // namespace Api

#undef USER_AGENT
#undef REMOTE_HOST
#undef REMOTE_PING_URL
#undef REMOTE_MESSAGE_URL
#undef REMOTE_SECURE_MESSAGE_URL
#undef JWT_TOKEN
