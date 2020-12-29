#ifndef _IOT_STARTER_ESP12F_IOT_STARTER_INCLUDE_API_H_
#define _IOT_STARTER_ESP12F_IOT_STARTER_INCLUDE_API_H_

namespace Api {

namespace Request {
typedef struct {
  int id;
  char key[20];
  int info;
} Message;
}; // namespace Request

namespace Response {
typedef struct {
  int id;
  char key[20];
  int info;

  int counter;
  int device_id;
} Message;
}; // namespace Response

namespace Routes {
void ping();
void post_message(const Request::Message &message);
void post_secure_message(const Request::Message &message);
}; // namespace Routes

}; // namespace Api

#endif // _IOT_STARTER_ESP12F_IOT_STARTER_INCLUDE_API_H_
