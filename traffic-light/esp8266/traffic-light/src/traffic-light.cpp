
#include <Arduino.h>
#include <Fsm.h>

constexpr int operator"" _sec(long double ms) { return 1000 * ms; }

enum LEDS { RED = 5, YELLOW = 4, GREEN = 15 };

// A basic non-blocking version of a traffic light.
// While this seems to be a toy example, the same mechanism can be used
// to create a non-blocking FSM to handle deep-sleep, Wifi connection (with
// attemps).
//
// This will create a traffic light with the following states:
// - red
// - red+yellow
// - green
//
// and transit like:
//
// red -> red+yellow -> green -> yellow -> red
//
// This  toy example might sound trivial, but is has nice properties as
// the yellow-state depends on the message of the previous state.
//
// A basic blocking(!) implementation would be
//
// void loop() {
//   digitalWrite(LEDS::RED, HIGH);
//   delay(3.0_sec);
//   digitalWrite(LEDS::YELLOW, HIGH);
//   delay(0.5_sec);
//   digitalWrite(LEDS::RED, LOW);
//   digitalWrite(LEDS::YELLOW, LOW);
//   digitalWrite(LEDS::GREEN, HIGH);
//   delay(2.0_sec);
//   digitalWrite(LEDS::GREEN, LOW);
//   digitalWrite(LEDS::YELLOW, HIGH);
//   delay(0.5_sec);
//   digitalWrite(LEDS::YELLOW, LOW);
// }

// Event that is emited, when a light has changed.
struct LightChangedEvent : fsm::Event {
  // Specifies whe
  bool progress_to_red = false;
  LightChangedEvent() {}
  LightChangedEvent(bool progress_to_red) : progress_to_red(progress_to_red) {}
};

// This even is triggered from the loop.
struct TickEvent : fsm::Event {};

// Forward declaration of all available states.
struct RedLightState;
struct YellowLightState;
struct GreenLightState;

// A quite generic light state.
class TrafficLightState : public fsm::StateMachine<TrafficLightState> {
 protected:
  // The time when the event has been entered.
  unsigned long start_time_stamp;

  // Are we longer in this state, than a given period?
  bool ActiveLongerThan(unsigned long period) {
    return ((millis() - start_time_stamp) > period);
  }

 public:
  // Whenever we enter a state, we want the time-stamp to updated.
  virtual void Enter() { start_time_stamp = millis(); }
  // Nothing special for the exit method. This is is filled out by the
  // concrete implementation.
  virtual void Exit() = 0;

  // Whenever the loop "ticks".
  virtual void On(const TickEvent&) {}
  // Whenever a light has changed.
  virtual void On(const LightChangedEvent&) {}
};

class RedLightState : public TrafficLightState {
  static constexpr unsigned long period = 3.0_sec;

 public:
  RedLightState() {}

  void Enter() override {
    start_time_stamp = millis();
    digitalWrite(LEDS::RED, HIGH);
  }

  // We do not turn the red light off.
  void Exit() override {}

  void On(const LightChangedEvent& event) override {}
  void On(const TickEvent& event) override {
    if (ActiveLongerThan(period)) {
      Emit(GotoEvent<YellowLightState>());
      Emit(LightChangedEvent(false));
    }
  };
};

class YellowLightState : public TrafficLightState {
  static constexpr unsigned long period = 0.5_sec;
  bool progress_to_red = false;

 public:
  YellowLightState() {}

  // All we know, we need to turn on yellow.
  void Enter() override {
    start_time_stamp = millis();
    digitalWrite(LEDS::YELLOW, HIGH);
  }

  // If the previous state communicated to turn red off, we will do so.
  void Exit() override {
    digitalWrite(LEDS::YELLOW, LOW);
    if (!progress_to_red) {
      digitalWrite(LEDS::RED, LOW);
    }
  }

  void On(const LightChangedEvent& event) override {
    progress_to_red = event.progress_to_red;
  }

  void On(const TickEvent& event) override {
    if (ActiveLongerThan(period)) {
      if (progress_to_red) {
        Emit(GotoEvent<RedLightState>());
        Emit(LightChangedEvent());
      } else {
        Emit(GotoEvent<GreenLightState>());
        Emit(LightChangedEvent());
      }
    }
  }
};

class GreenLightState : public TrafficLightState {
  static constexpr unsigned long period = 2.0_sec;

 public:
  GreenLightState() {}

  void Enter() override {
    start_time_stamp = millis();
    digitalWrite(LEDS::GREEN, HIGH);
  }
  void Exit() override { digitalWrite(LEDS::GREEN, LOW); }

  void On(const LightChangedEvent& event) override {}
  void On(const TickEvent& event) override {
    if (ActiveLongerThan(period)) {
      Emit(GotoEvent<YellowLightState>());
      Emit(LightChangedEvent(true));
    }
  }
};

void setup() {
  Serial.begin(115200);

  pinMode(LEDS::RED, OUTPUT);
  pinMode(LEDS::YELLOW, OUTPUT);
  pinMode(LEDS::GREEN, OUTPUT);

  TrafficLightState::Start<RedLightState>();
}
void loop() { TrafficLightState::Emit(TickEvent()); }
