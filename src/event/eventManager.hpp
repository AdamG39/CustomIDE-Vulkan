#ifndef CUSTOM_EVENT_MANAGER_H
#define CUSTOM_EVENT_MANAGER_H

#include <queue>
#include "event.hpp"

struct EventListenerHandle {
  Entity* Object;
  bool Expired;

  bool IsValid();
};

using EventChannel = std::list<EventListenerHandle>;

class EventHandler {
private:
  std::array<EventChannel, EventTypeCount>& m_eventChannels;

  bool HandleMouseEvent(const EventInfo& Info);
  bool HandleWindowEvent(const EventInfo& Info);
  bool HandleKeyboardEvent(const EventInfo& Info);
  bool HandleCharacterEvent(const EventInfo& Info);

public:
  bool HandleEvent(const Event& Event);

  EventHandler(std::array<EventChannel, EventTypeCount>& EventChannels)
    : m_eventChannels(EventChannels) {}
};

class EventManager {
private:
  std::queue<Event> m_eventQueue;
  std::array<EventChannel, EventTypeCount> m_eventChannels;
  EventHandler m_eventHandler = EventHandler(m_eventChannels);

public:
  void PushEvent(const Event& Event);

  void PopEvent();

  Event CreateEvent(EventType Type, const EventInfo* Info);

  void AddEvent(EventType Type, const EventInfo* Info);

  void HandleEvents();

  void RegisterEventListener(Entity* Object, int Channels);

  void ExpireEventListener(Entity* Object);

  void ExpireEventListenerOnChannel(Entity* Object, EventType Channel);
};

#endif

