#ifndef CUSTOM_EVENT_MANAGER_H
#define CUSTOM_EVENT_MANAGER_H

#include <queue>
#include "event.hpp"

namespace CustomIDE::EventSystem {

bool CursorOverlap(const Vector2D& CursorPos, const Vector2D& Size, const Vector2D& Position);

struct EventListenerHandle {
  UI::ECS::Entity* Object;
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

  void RegisterEventListener(UI::ECS::Entity* Object, EventType Channels);

  void ExpireEventListener(UI::ECS::Entity* Object);

  void ExpireEventListenerOnChannel(UI::ECS::Entity* Object, EventType Channel);
};

}

#endif

