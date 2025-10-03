#include "eventManager.hpp"
#include "GLFW/glfw3.h"

bool CursorOverlap(const Vector2<float>& CursorPos, const Vector2<float>& Size, const Vector2<float>& Position) {
  Vector2 min = Vector2(Position.x - (Size.x / 2), Position.y - (Size.y / 2));
  Vector2 max = Vector2(Position.x + (Size.x / 2), Position.y + (Size.y / 2));

  if (CursorPos.x >= min.x && CursorPos.x <= max.x &&
      CursorPos.y >= min.y && CursorPos.y <= max.y) return true;

  return false;
}

bool EventHandler::HandleMouseEvent(const EventInfo& Info) {
  if (Info.MouseButton == GLFW_MOUSE_BUTTON_LEFT) {
    auto& entityTree = Info.EntityManager->GetEntityTree();

    for (auto entity : entityTree) {
      Transform* transform = entity->GetComponent<Transform>();
      Button* button = entity->GetComponent<Button>();

      if (transform != nullptr && button != nullptr) {
        if (CursorOverlap(Info.CursorPosition, transform->GetPixelSize(), transform->GetPixelPosition())) {
          (Info.MouseAction == GLFW_PRESS) ? button->OnPress() : button->OnRelease();
          return true;
        }
      }
    }
  }

  return true;
}

bool EventHandler::HandleWindowEvent(const EventInfo& Info) {
  return false;
}

bool EventHandler::HandleEvent(const Event& Event) {
  switch (Event.Type) {
    case Mouse:
      return HandleMouseEvent(Event.Info);
    case Window:
      return HandleWindowEvent(Event.Info);
  }
}

void EventManager::PushEvent(const Event& Event) {
  m_eventQueue.push(Event);
}

void EventManager::PopEvent() {
  m_eventQueue.pop();
}

Event EventManager::CreateEvent(EventType Type, const EventInfo* Info) {
  Event event = Event();

  event.Type = Type;
  if (Info != nullptr)
    event.Info = *Info;

  return event;
}

void EventManager::AddEvent(EventType Type, const EventInfo* Info) {
  PushEvent(CreateEvent(Type, Info));
}

void EventManager::HandleEvents() {
  // Loop through all current events ignoring repeats to avoid possible
  // infinite loop due to something required by next frame
  if (m_eventQueue.size() == 0) return;

  for (size_t i = m_eventQueue.size(); i > 0; i--) {
    Event& event = m_eventQueue.front();
    bool result = m_eventHandler.HandleEvent(event);

    PopEvent();

    if (!result) PushEvent(event);
  }
}

