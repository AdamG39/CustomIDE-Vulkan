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
  if (Info.KeyboardInfo.Action == GLFW_RELEASE) return true;
  auto& entityTree = Info.EntityManager->GetEntityTree();

  for (auto entity : entityTree) {
    TextBox* textBox = entity->GetComponent<TextBox>();

    if (textBox != nullptr) {
      switch (Info.KeyboardInfo.Key) {
      case GLFW_KEY_LEFT:
        if (Info.KeyboardInfo.Modifications & GLFW_MOD_CONTROL) {
          if (textBox->GetSelectionState()) { 
            textBox->EndSelection();
            break;
          }
          textBox->MoveBackWord();
        }
        else if (Info.KeyboardInfo.Modifications & GLFW_MOD_SHIFT) {
          textBox->SelectLeft();
        }
        else {
          if (textBox->GetSelectionState()) { 
            textBox->EndSelection();
            break;
          }
          textBox->MoveCursorLeft();
        }
        break;
      case GLFW_KEY_RIGHT:
        if (Info.KeyboardInfo.Modifications & GLFW_MOD_CONTROL) {
          if (textBox->GetSelectionState()) { 
            textBox->EndSelection();
            break;
          }
          textBox->MoveForwardWord();
        }
        else if (Info.KeyboardInfo.Modifications & GLFW_MOD_SHIFT) {
          textBox->SelectRight();
        }
        else {
          if (textBox->GetSelectionState()) { 
            textBox->EndSelection();
            break;
          }
          textBox->MoveCursorRight();
        }
        break;
      case GLFW_KEY_UP:
        textBox->MoveCursorUp();
        break;
      case GLFW_KEY_DOWN:
        textBox->MoveCursorDown();
        break;
      case GLFW_KEY_BACKSPACE:
        if (textBox->GetSelectionState())
          textBox->DeleteSelection();
        else {
          textBox->Delete(textBox->GetCursorPosition() - 1);
          textBox->MoveCursorLeft();
        }
        break;
      case GLFW_KEY_DELETE:
        if (textBox->GetSelectionState())
          textBox->DeleteSelection();
        else
          textBox->Delete(textBox->GetCursorPosition());
        break;
      case GLFW_KEY_TAB:
        textBox->Insert('\t', textBox->GetCursorPosition());
        break;
      case GLFW_KEY_ENTER:
        textBox->Insert('\n', textBox->GetCursorPosition());
        break;
      case GLFW_KEY_ESCAPE:
        textBox->EndSelection();
        break;
      case GLFW_KEY_C:
        if (Info.KeyboardInfo.Modifications & GLFW_MOD_CONTROL)
          textBox->CopySelection(Info.Window);
        break;
      case GLFW_KEY_V:
        if (Info.KeyboardInfo.Modifications & GLFW_MOD_CONTROL)
          textBox->PasteText(Info.Window);
        break;
      case GLFW_KEY_S:
        if (Info.KeyboardInfo.Modifications & GLFW_MOD_CONTROL)
          textBox->SaveFile();
        break;
      }

      return true;
    }
  }

  return false;
}

bool EventHandler::HandleCharacterEvent(const EventInfo& Info) {
  // TODO: add handling of any UFT-8 character

  auto& entityTree = Info.EntityManager->GetEntityTree();

  for (auto entity : entityTree) {
    TextBox* textBox = entity->GetComponent<TextBox>();

    if (textBox == nullptr) continue;

    textBox->Insert(static_cast<char>(Info.CharacterInfo.Codepoint), textBox->GetCursorPosition());
    return true;
  }

  return false;
}

bool EventHandler::HandleEvent(const Event& Event) {
  switch (Event.Type) {
    case Mouse:
      return HandleMouseEvent(Event.Info);
    case Window:
      return HandleWindowEvent(Event.Info);
    case Keyboard:
      return HandleKeyboardEvent(Event.Info);
    case Character:
      return HandleCharacterEvent(Event.Info);
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

