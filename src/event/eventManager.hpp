#ifndef CUSTOM_EVENT_MANAGER_H
#define CUSTOM_EVENT_MANAGER_H

#include <queue>
#include "event.hpp"

class EventHandler {
private:
  bool HandleMouseEvent(const EventInfo& Info);
  bool HandleWindowEvent(const EventInfo& Info);
  bool HandleKeyboardEvent(const EventInfo& Info);

public:
   bool HandleEvent(const Event& Event);
};

class EventManager {
private:
  std::queue<Event> m_eventQueue;
  EventHandler m_eventHandler = EventHandler();

public:
  void PushEvent(const Event& Event);

  void PopEvent();

  Event CreateEvent(EventType Type, const EventInfo* Info);

  void AddEvent(EventType Type, const EventInfo* Info);

  void HandleEvents();
};

#endif

