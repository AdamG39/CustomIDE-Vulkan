#ifndef CUSTOM_EVENT_H
#define CUSTOM_EVENT_H

#include "../renderer/shapes.hpp"
#include "../uilib/ecs.hpp"

enum EventType {
  Mouse,
  Window,
  Keyboard,
  Character
};

struct MouseEventInfo {
  Vector2<float> Position;
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
  std::shared_ptr<EntityManager> Manager;
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

#endif

