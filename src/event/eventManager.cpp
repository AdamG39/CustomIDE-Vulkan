#include "eventManager.hpp"
#include "../uilib/components/transform.hpp"
#include "../uilib/components/textBox.hpp"
#include "GLFW/glfw3.h"
#include <iostream>

bool CursorOverlap(const Vector2<float>& CursorPos, const Vector2<float>& Size, const Vector2<float>& Position) {
  Vector2 min = Vector2(Position.x - (Size.x / 2), Position.y - (Size.y / 2));
  Vector2 max = Vector2(Position.x + (Size.x / 2), Position.y + (Size.y / 2));

  if (CursorPos.x >= min.x && CursorPos.x <= max.x &&
      CursorPos.y >= min.y && CursorPos.y <= max.y) return true;

  return false;
}

bool EventListenerHandle::IsValid() {
  return !Expired && Object != nullptr;
}

bool EventHandler::HandleMouseEvent(const EventInfo& Info) {
  if (Info.MouseInfo.Button == GLFW_MOUSE_BUTTON_LEFT) {
    for (auto& handler : m_eventChannels[EventTypeEnumToIndex(EventType::Mouse)]) {
      if (!handler.IsValid()) continue;

      auto entity = handler.Object;

      Transform* transform = entity->GetComponent<Transform>();
      IInteractable* button = entity->GetInteractableComponent();

      if (transform == nullptr || button == nullptr) continue;

      if (CursorOverlap(Info.MouseInfo.Position, transform->GetPixelSize(), transform->GetPixelPosition())) {
        if (Info.MouseInfo.Action == GLFW_PRESS && button->HasAction("OnPress")) button->ExecAction("OnPress"); 
        else if (Info.MouseInfo.Action == GLFW_RELEASE && button->HasAction("OnRelease")) button->ExecAction("OnRelease");
        return true;
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

  for (auto& handler : m_eventChannels[EventTypeEnumToIndex(EventType::Keyboard)]) {
    if (!handler.IsValid()) continue;

    TextBox* textBox = handler.Object->GetComponent<TextBox>();

    if (textBox == nullptr) continue;

    int previousPosition = textBox->GetCursorPosition();

    switch (Info.KeyboardInfo.Key) {
    case GLFW_KEY_LEFT:
      if (textBox->GetSelectionState() && !(Info.KeyboardInfo.Modifications & GLFW_MOD_SHIFT)) { 
        textBox->CancelSelection();
        break;
      }

      if (Info.KeyboardInfo.Modifications & GLFW_MOD_SHIFT && !textBox->GetSelectionState())
        textBox->StartSelection();

      if (Info.KeyboardInfo.Modifications & GLFW_MOD_CONTROL)
        textBox->MoveBackWord();

      else textBox->MoveCursorLeft();

      break;
    case GLFW_KEY_RIGHT:
      if (textBox->GetSelectionState() && !(Info.KeyboardInfo.Modifications & GLFW_MOD_SHIFT)) { 
        textBox->CancelSelection();
        break;
      }

      if (Info.KeyboardInfo.Modifications & GLFW_MOD_SHIFT && !textBox->GetSelectionState())
        textBox->StartSelection();

      if (Info.KeyboardInfo.Modifications & GLFW_MOD_CONTROL)
        textBox->MoveForwardWord();

      else textBox->MoveCursorRight();

      break;
    case GLFW_KEY_UP:
      if (textBox->GetSelectionState() && !(Info.KeyboardInfo.Modifications & GLFW_MOD_SHIFT)) { 
        textBox->CancelSelection();
        break;
      }

      if (Info.KeyboardInfo.Modifications & GLFW_MOD_SHIFT && !textBox->GetSelectionState())
        textBox->StartSelection();

      textBox->MoveCursorUp();

      break;
    case GLFW_KEY_DOWN:
      if (textBox->GetSelectionState() && !(Info.KeyboardInfo.Modifications & GLFW_MOD_SHIFT)) { 
        textBox->CancelSelection();
        break;
      }

      if (Info.KeyboardInfo.Modifications & GLFW_MOD_SHIFT && !textBox->GetSelectionState())
        textBox->StartSelection();

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
      textBox->CancelSelection();
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

    if (textBox->GetSelectionState()) textBox->UpdateSelection(previousPosition);
    return true;
  }

  return false;
}

bool EventHandler::HandleCharacterEvent(const EventInfo& Info) {
  // TODO: add handling of any UFT-8 character



  for (auto handler : m_eventChannels[EventTypeEnumToIndex(EventType::Character)]) {
    if (!handler.IsValid()) continue;

    TextBox* textBox = handler.Object->GetComponent<TextBox>();

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

  return false;
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

void EventManager::RegisterEventListener(Entity* Object, int Channels) {
  EventListenerHandle handle {
    .Object = Object,
    .Expired = false
  };

  if (Channels & EventType::Mouse) {
    m_eventChannels[EventTypeEnumToIndex(EventType::Mouse)].push_back(handle);
  }

  if (Channels & EventType::Window) {
    m_eventChannels[EventTypeEnumToIndex(EventType::Window)].push_back(handle);
  }

  if (Channels & EventType::Keyboard) {
    m_eventChannels[EventTypeEnumToIndex(EventType::Keyboard)].push_back(handle);
  }

  if (Channels & EventType::Character) {
    m_eventChannels[EventTypeEnumToIndex(EventType::Character)].push_back(handle);
  }
}

void EventManager::ExpireEventListener(Entity* Object) {
  for (auto& channel : m_eventChannels) {
    for (auto& listener : channel) {
      if (listener.Object == Object)
        listener.Expired = true;
    }
  }
}

void EventManager::ExpireEventListenerOnChannel(Entity* Object, EventType Channel) {
  int channelIndex = EventTypeEnumToIndex(Channel);
  if (channelIndex >= EventTypeCount) return;

  for (auto& listener : m_eventChannels[channelIndex]) {
    if (listener.Object == Object)
      listener.Expired = true;
  }
}
