#include "Button.h"

#include <Arduino.h>

bool PushButton::Pressed() {
  old_state_ = new_state_;
  new_state_ = digitalRead(pin_);

  return (old_state_ != new_state_) && (new_state_ == HIGH);
}

bool PushButton::Released() {
  old_state_ = new_state_;
  new_state_ = digitalRead(pin_);

  return (old_state_ != new_state_) && (new_state_ == LOW);
}
