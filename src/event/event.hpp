#ifndef CUSTOM_EVENT_H
#define CUSTOM_EVENT_H

#include "../renderer/shapes.hpp"
#include "../uilib/ecs.hpp"

enum EventType {
  Mouse,
  Window,
  Keyboard,
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

struct EventInfo {
  // Application manager pointers
  EntityManager* EntityManager;
  GLFWwindow* Window;
  
  union {
    MouseEventInfo MouseInfo;
    KeyboardEventInfo KeyboardInfo;
    WindowEventAction WindowAction;
  };
};

struct Event {
  EventType Type;
  EventInfo Info;
};

#endif

