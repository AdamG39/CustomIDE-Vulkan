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
  if (Info.MouseInfo.Button == GLFW_MOUSE_BUTTON_LEFT) {
    auto& entityTree = Info.EntityManager->GetEntityTree();

    for (auto entity : entityTree) {
      Transform* transform = entity->GetComponent<Transform>();
      Button* button = entity->GetComponent<Button>();

      if (transform != nullptr && button != nullptr) {
        if (CursorOverlap(Info.MouseInfo.Position, transform->GetPixelSize(), transform->GetPixelPosition())) {
          (Info.MouseInfo.Action == GLFW_PRESS) ? button->OnPress() : button->OnRelease();
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

bool EventHandler::HandleKeyboardEvent(const EventInfo& Info) {
  if (Info.KeyboardInfo.Action != GLFW_PRESS) return true;
  auto& entityTree = Info.EntityManager->GetEntityTree();

  bool foundTextBox = false;

  for (auto entity : entityTree) {
    Transform* transform = entity->GetComponent<Transform>();
    TextBox* textBox = entity->GetComponent<TextBox>();

    if (transform != nullptr && textBox != nullptr) {
      foundTextBox = true;
      const char* temp = glfwGetKeyName(Info.KeyboardInfo.Key, 0);
      char keyChar = (temp != nullptr) ? temp[0] : NULL;

      if (Info.KeyboardInfo.Key == GLFW_KEY_BACKSPACE) {
        textBox->Delete(textBox->GetContent().size() - 1);
      } else if (Info.KeyboardInfo.Key == GLFW_KEY_SPACE) {
        textBox->Insert(' ', textBox->GetContent().size());
      } else if (keyChar != NULL) {
        textBox->Insert(keyChar, textBox->GetContent().size());
      }
    }
  }

  return foundTextBox;
}

bool EventHandler::HandleEvent(const Event& Event) {
  switch (Event.Type) {
    case Mouse:
      return HandleMouseEvent(Event.Info);
    case Window:
      return HandleWindowEvent(Event.Info);
    case Keyboard:
      return HandleKeyboardEvent(Event.Info);
  }
}

void EventManager::PushEvent(const Event& Event) {
  m_eventQueue.push(Event);
}

void EventManager::PopEvent() {
  m_eventQueue.pop();
}

Event EventManager::CreateEvent(EventType Type, const EventInfo* Info) {
  Event event{};

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

