#include <Arduino.h>
#include <Fsm.h>

constexpr int operator"" _sec(long double ms) { return 1000 * ms; }

enum GPIO { RED = 5, YELLOW = 4, GREEN = 15 };

// A basic non-blocking version of a traffic light.
// While this seems to be a toy example, the same mechanism can be used
// to create a non-blocking FSM to handle deep-sleep, Wifi connection (with
// attemps).
//
// This will create a traffic light with the following states:
// - red
// - yellow
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
//   digitalWrite(GPIO::RED, HIGH);
//   delay(3.0_sec);
//   digitalWrite(GPIO::YELLOW, HIGH);
//   delay(0.5_sec);
//   digitalWrite(GPIO::RED, LOW);
//   digitalWrite(GPIO::YELLOW, LOW);
//   digitalWrite(GPIO::GREEN, HIGH);
//   delay(2.0_sec);
//   digitalWrite(GPIO::GREEN, LOW);
//   digitalWrite(GPIO::YELLOW, HIGH);
//   delay(0.5_sec);
//   digitalWrite(GPIO::YELLOW, LOW);
// }

// Event that is emitted, when a light has changed.
struct LightChangedEvent : fsm::Event {
  // Specifies whe
  bool progress_to_red = false;
  LightChangedEvent() {}
  LightChangedEvent(bool progress_to_red) : progress_to_red(progress_to_red) {}
};

// This event is triggered from the loop.
struct TickEvent : fsm::Event {};

// Forward declaration of all available states.
struct RedLightState;
struct YellowLightState;
struct GreenLightState;

// A quite generic light state.
class TrafficLightState : public fsm::StateMachine<TrafficLightState> {
 protected:
  // The time when the event has been entered.
  unsigned long entered_time_stamp;

  // Are we longer in this state, than a given max_period?
  bool ActiveLongerThan(unsigned long max_period) {
    return ((millis() - entered_time_stamp) > max_period);
  }

 public:
  // Whenever we enter a state, we want the time-stamp to updated.
  virtual void Enter() { entered_time_stamp = millis(); }
  // Nothing special for the exit method. This is is filled out by the
  // concrete implementation.
  virtual void Exit() = 0;

  // Whenever the loop "ticks".
  virtual void On(const TickEvent&) {}
  // Whenever a light has changed.
  virtual void On(const LightChangedEvent&) {}
};

class RedLightState : public TrafficLightState {
  static constexpr unsigned long max_period = 3.0_sec;

 public:
  RedLightState() {}

  void Enter() override {
    entered_time_stamp = millis();
    digitalWrite(GPIO::RED, HIGH);
  }

  // We do not turn the red light off as some states expect it to stay turned
  // on.
  void Exit() override {}

  void On(const LightChangedEvent& event) override {}
  void On(const TickEvent& event) override {
    if (ActiveLongerThan(max_period)) {
      Emit(GotoEvent<YellowLightState>());
      Emit(LightChangedEvent(false));
    }
  };
};

class YellowLightState : public TrafficLightState {
  static constexpr unsigned long max_period = 0.5_sec;
  bool progress_to_red = false;

 public:
  YellowLightState() {}

  // All we know, we need to turn on yellow.
  void Enter() override {
    entered_time_stamp = millis();
    digitalWrite(GPIO::YELLOW, HIGH);
  }

  // If the previous state communicated to turn red off, we will do so.
  void Exit() override {
    digitalWrite(GPIO::YELLOW, LOW);
    if (!progress_to_red) {
      digitalWrite(GPIO::RED, LOW);
    }
  }

  void On(const LightChangedEvent& event) override {
    progress_to_red = event.progress_to_red;
  }

  void On(const TickEvent& event) override {
    if (ActiveLongerThan(max_period)) {
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
  static constexpr unsigned long max_period = 2.0_sec;

 public:
  GreenLightState() {}

  void Enter() override {
    entered_time_stamp = millis();
    digitalWrite(GPIO::GREEN, HIGH);
  }
  void Exit() override { digitalWrite(GPIO::GREEN, LOW); }

  void On(const LightChangedEvent& event) override {}
  void On(const TickEvent& event) override {
    if (ActiveLongerThan(max_period)) {
      Emit(GotoEvent<YellowLightState>());
      Emit(LightChangedEvent(true));
    }
  }
};

void setup() {
  Serial.begin(115200);

  pinMode(GPIO::RED, OUTPUT);
  pinMode(GPIO::YELLOW, OUTPUT);
  pinMode(GPIO::GREEN, OUTPUT);

  TrafficLightState::Start<RedLightState>();
}
void loop() { TrafficLightState::Emit(TickEvent()); }
