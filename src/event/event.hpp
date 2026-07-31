#ifndef CUSTOM_EVENT_H
#define CUSTOM_EVENT_H

#include "../helpers/generic.hpp"
#include "../renderer/shapes.hpp"
#include "../uilib/ecs.hpp"
#include <cmath>

namespace CustomIDE::EventSystem {

enum class EventType : unsigned {
  Undefined   = 0u,
  Mouse       = 1u << 0,
  Window      = 1u << 1,
  Keyboard    = 1u << 2,
  Character   = 1u << 3
};
constexpr EventType operator|(EventType ET1, EventType ET2) {
  return static_cast<EventType>(to_underlying(ET1) | to_underlying(ET2));
}
constexpr EventType operator&(EventType ET1, EventType ET2) {
  return static_cast<EventType>(to_underlying(ET1) & to_underlying(ET2));
}
constexpr EventType operator^(EventType ET1, EventType ET2) {
  return static_cast<EventType>(to_underlying(ET1) ^ to_underlying(ET2));
}
constexpr EventType operator~(EventType ET) {
  return static_cast<EventType>(~to_underlying(ET));
}
constexpr EventType& operator|=(EventType& LHS, EventType RHS) {
  LHS = LHS | RHS;
  return LHS;
}
constexpr EventType& operator&=(EventType& LHS, EventType RHS) {
  LHS = LHS & RHS;
  return LHS;
}
constexpr EventType& operator^=(EventType& LHS, EventType RHS) {
  LHS = LHS ^ RHS;
  return LHS;
}

static constexpr int EventTypeCount = 4;

constexpr int EventTypeEnumToIndex(EventType Type) {
  return std::log2<int>(to_underlying(Type));
}

struct MouseEventInfo {
  Vector2D Position;
  int Button;
  int Action;
  int Modifications;
};

enum WindowEventInfo {
  Maximise,
  Restore,
  Minimise,
};

struct KeyboardEventInfo {
  int Key;
  int Scancode;
  int Action;
  int Modifications;
};

struct CharacterEventInfo {
  unsigned int Codepoint;
};

struct EventInfo {
  // Application manager pointers
  std::shared_ptr<UI::ECS::EntityManager> Manager;
  GLFWwindow* Window;
  
  union {
    MouseEventInfo MouseInfo;
    WindowEventInfo WindowInfo;
    KeyboardEventInfo KeyboardInfo;
    CharacterEventInfo CharacterInfo;
  };
};

struct Event {
  EventType Type;
  EventInfo Info;
};

} // namespace EventSystem

#endif

