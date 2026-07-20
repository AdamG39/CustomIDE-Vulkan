#ifndef CUSTOM_EVENT_H
#define CUSTOM_EVENT_H

#include "../renderer/shapes.hpp"
#include "../uilib/ecs.hpp"

namespace CustomIDE::EventSystem {

enum EventType : int {
  Mouse       = (1u << 0),
  Window      = (1u << 1),
  Keyboard    = (1u << 2),
  Character   = (1u << 3),
};

static constexpr int EventTypeCount = 4;

constexpr int EventTypeEnumToIndex(EventType Type) {
  return log2<int>(Type);
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

