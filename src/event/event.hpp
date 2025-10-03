#ifndef CUSTOM_EVENT_H
#define CUSTOM_EVENT_H

#include "../renderer/shapes.hpp"
#include "../uilib/ecs.hpp"

enum EventType {
  Mouse,
  Window,
};

enum WindowEventAction {
  Maximise,
  Restore,
  Minimise,
};

struct EventInfo {
  // Application manager pointers
  EntityManager* EntityManager;
  GLFWwindow* Window;
  
  // Mouse related info
  Vector2<float> CursorPosition;
  int MouseButton;
  int MouseAction;
  int MouseModifications;

  // Window related info
  WindowEventAction WindowAction;
};

struct Event {
  EventType Type;
  EventInfo Info;
};

#endif

