#ifndef _IOT_STARTER_ESP12F_IOT_STARTER_INCLUDE_API_H_
#define _IOT_STARTER_ESP12F_IOT_STARTER_INCLUDE_API_H_

#include <memory>

#include "Arduino.h"
#include "ESP8266HTTPClient.h"

namespace Api {

namespace Request {
typedef struct {
  int id;
  char key[20];
  int info;
} Message;

String Render(const Message &message);

};  // namespace Request

namespace Response {
typedef struct {
  int id;
  char key[20];
  int info;

  int counter;
  int device_id;
} Message;

void Bind(Stream *stream, Message *message);
void Print(const Message &message);
};  // namespace Response

bool IsConnected(WiFiClient *client);

namespace Routes {
std::unique_ptr<HTTPClient> Ping(WiFiClient *client);
std::unique_ptr<HTTPClient> PostMessage(WiFiClient *client,
                                        const Request::Message &message);
std::unique_ptr<HTTPClient> PostSecureMessage(WiFiClient *client,
                                              const Request::Message &message);
};  // namespace Routes

};  // namespace Api

#endif  // _IOT_STARTER_ESP12F_IOT_STARTER_INCLUDE_API_H_
