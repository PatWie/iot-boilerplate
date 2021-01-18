// Copyright https://github.com/PatWie/CppNumericalSolvers, MIT license
#ifndef LEDS_ESP8266_LEDS_LIB_FSM_SRC_FSM_H_
#define LEDS_ESP8266_LEDS_LIB_FSM_SRC_FSM_H_

#include <type_traits>

namespace fsm {

struct Event {};

// Each state belongs to a specific machine. As this is heavily templated,
// we have to prevent a machine transition to a state from a different machine.
template <class StateType, class OtherStateType>
struct is_same_machine : std::is_same<typename StateType::MachineType,
                                      typename OtherStateType::MachineType> {};

// A description/instance of a state.
template <class T>
struct StateInstance {
  using Type = StateInstance<T>;
  using ValueType = T;
  static T value;
};

// A generic+global way to hold all states.
template <class StateType>
typename StateInstance<StateType>::ValueType StateInstance<StateType>::value;

template <class StateType>
class StateMachine {
 public:
  using MachineType = StateMachine<StateType>;
  using StatePtr = StateType *;
  static StatePtr current_state_;

  // This is a special event tied to this machine and indicates a state change.
  // We could have `Goto` implementatin public, but this would allow to mess
  // around with the states.
  template <class TNextStateType>
  struct GotoEvent {
    using NextStateType = StateInstance<TNextStateType>;
  };

  // Start the StateMachine in a specific state.
  template <class StartStateType>
  static void Start() {
    current_state_ = &StateInstance<StartStateType>::value;
    current_state_->Enter();
  }

  // A specialized version of emitting events to transition to another state.
  template <class NextStateType>
  void Emit(const GotoEvent<NextStateType> &event) {
    Goto<NextStateType>();
  }

  // All other events are handled by the state itself.
  template <class EventType>
  static void Emit(EventType const &event) {
    current_state_->On(event);
  }

 private:
  // A helper method to transition to another state.
  template <class NextStateType>
  void Goto(void) {
    static_assert(is_same_machine<StateType, NextStateType>::value,
                  "The state does not belong to the machine type.");
    current_state_->Exit();
    current_state_ = &StateInstance<NextStateType>::value;
    current_state_->Enter();
  }
};

// The current state has to life somewhere. This is here.
template <class StateType>
typename StateMachine<StateType>::StatePtr
    StateMachine<StateType>::current_state_;

} /* namespace fsm */

#endif  // LEDS_ESP8266_LEDS_LIB_FSM_SRC_FSM_H_
