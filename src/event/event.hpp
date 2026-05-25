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

enum WindowEventAction {
  Maximise,
  Restore,
  Minimise,
};

struct MouseEventInfo {
  Vector2<float> Position;
  int Button;
  int Action;
  int Modifications;
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
  EntityManager* Manager;
  GLFWwindow* Window;
  
  union {
    MouseEventInfo MouseInfo;
    KeyboardEventInfo KeyboardInfo;
    CharacterEventInfo CharacterInfo;
    WindowEventAction WindowAction;
  };
};

struct Event {
  EventType Type;
  EventInfo Info;
};

#endif

