#ifndef TRAFFIC_LIGHT_EMBEDDED_TRAFFIC_LIGHT_LIB_BUTTON_SRC_BUTTON_H_
#define TRAFFIC_LIGHT_EMBEDDED_TRAFFIC_LIGHT_LIB_BUTTON_SRC_BUTTON_H_

#include <Arduino.h>

class PushButton {
 public:
  explicit PushButton(int pin)
      : old_state_(LOW), new_state_(old_state_), pin_(pin) {}

  bool Pressed();

  bool Released();

 private:
  int old_state_ = LOW;
  int new_state_ = LOW;
  int pin_;
};

#endif  // TRAFFIC_LIGHT_EMBEDDED_TRAFFIC_LIGHT_LIB_BUTTON_SRC_BUTTON_H_
