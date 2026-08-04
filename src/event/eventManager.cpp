#include "../helpers/errors/errors.hpp"
#include "eventManager.hpp"
#include "../uilib/components/transform.hpp"
#include "../uilib/components/textBox.hpp"
#include "GLFW/glfw3.h"
#include <iostream>

namespace CustomIDE {

bool EventSystem::CursorOverlap(const Vector2D& CursorPos, const Vector2D& Size, const Vector2D& Position) {
  Vector2 min = Vector2(Position.x - (Size.x / 2), Position.y - (Size.y / 2));
  Vector2 max = Vector2(Position.x + (Size.x / 2), Position.y + (Size.y / 2));

  if (CursorPos.x >= min.x && CursorPos.x <= max.x &&
      CursorPos.y >= min.y && CursorPos.y <= max.y) return true;

  return false;
}

bool EventSystem::EventListenerHandle::IsValid() {
  return !Expired && Object != nullptr;
}

bool EventSystem::EventHandler::HandleMouseEvent(const EventInfo& Info) {
  if (Info.MouseInfo.Button == GLFW_MOUSE_BUTTON_LEFT) {
    for (auto& handler : m_eventChannels[EventTypeEnumToIndex(EventType::Mouse)]) {
      if (!handler.IsValid()) continue;

      auto entity = handler.Object;

      UI::ECS::Transform* transform = entity->GetComponent<UI::ECS::Transform>();
      UI::ECS::IInteractable* button = entity->GetInteractableComponent();

      if (transform == nullptr || button == nullptr) continue;

      if (CursorOverlap(Info.MouseInfo.Position, transform->GetGlobalSize(), transform->GetGlobalPosition())) {
        if (Info.MouseInfo.Action == GLFW_PRESS && button->HasAction("OnPress")) button->ExecAction("OnPress"); 
        else if (Info.MouseInfo.Action == GLFW_RELEASE && button->HasAction("OnRelease")) button->ExecAction("OnRelease");
        return true;
      }
    }
  }
  else if (Info.MouseInfo.Button < 0) {
    bool foundSuitableEntity = false;
    for (auto& handler : m_eventChannels[EventTypeEnumToIndex(EventType::Mouse)]) {
      if (!handler.IsValid()) continue;

      auto entity = handler.Object;

      UI::ECS::Transform* transform = entity->GetComponent<UI::ECS::Transform>();
      UI::ECS::IInteractable* button = entity->GetInteractableComponent();

      if (transform == nullptr || button == nullptr) continue;

      if (CursorOverlap(Info.MouseInfo.Position, transform->GetGlobalSize(), transform->GetGlobalPosition())) {
        foundSuitableEntity = true;
        if (entity != m_hoveredEntity) {
          if (m_hoveredEntity != nullptr) {
            auto* previousEntityInteractable = m_hoveredEntity->GetInteractableComponent();

            if (previousEntityInteractable && previousEntityInteractable->HasAction("OnHoverExit")) {
              previousEntityInteractable->ExecAction("OnHoverExit");
            }
          }

          if (button->HasAction("OnHoverEnter")) button->ExecAction("OnHoverEnter");

          m_hoveredEntity = entity;
        }
      }
    }

    if (!foundSuitableEntity) {
      if (m_hoveredEntity != nullptr) {
        auto* previousEntityInteractable = m_hoveredEntity->GetInteractableComponent();

        if (previousEntityInteractable && previousEntityInteractable->HasAction("OnHoverExit")) {
          previousEntityInteractable->ExecAction("OnHoverExit");
        }
      }

      m_hoveredEntity = nullptr;
    }
  }

  return true;
}

bool EventSystem::EventHandler::HandleWindowEvent(const EventInfo& Info) {
  return false;
}

bool EventSystem::EventHandler::HandleKeyboardEvent(const EventInfo& Info) {
  if (Info.KeyboardInfo.Action == GLFW_RELEASE) return true;

  for (auto& handler : m_eventChannels[EventTypeEnumToIndex(EventType::Keyboard)]) {
    if (!handler.IsValid()) continue;

    UI::ECS::TextBox* textBox = handler.Object->GetComponent<UI::ECS::TextBox>();

    if (textBox == nullptr) continue;

    int previousPosition = textBox->GetCursorPosition();

    auto cursorPos = textBox->GetCursorPosition();

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

    if (textBox->GetSelectionState() && cursorPos != textBox->GetCursorPosition()) textBox->UpdateSelection(previousPosition);
    return true;
  }

  return false;
}

bool EventSystem::EventHandler::HandleCharacterEvent(const EventInfo& Info) {
  // TODO: add handling of any UFT-8 character



  for (auto handler : m_eventChannels[EventTypeEnumToIndex(EventType::Character)]) {
    if (!handler.IsValid()) continue;

    UI::ECS::TextBox* textBox = handler.Object->GetComponent<UI::ECS::TextBox>();

    if (textBox == nullptr) continue;

    textBox->Insert(static_cast<char>(Info.CharacterInfo.Codepoint), textBox->GetCursorPosition());
    return true;
  }

  return false;
}

bool EventSystem::EventHandler::HandleEvent(const Event& Event) {
  switch (Event.Type) {
    case EventType::Mouse:
      return HandleMouseEvent(Event.Info);
    case EventType::Window:
      return HandleWindowEvent(Event.Info);
    case EventType::Keyboard:
      return HandleKeyboardEvent(Event.Info);
    case EventType::Character:
      return HandleCharacterEvent(Event.Info);
    case EventType::Undefined:
      Errors::ExitWithError("Undefined event was triggered", -20);
      break;
  }

  return false;
}

void EventSystem::EventManager::PushEvent(const Event& Event) {
  m_eventQueue.push(Event);
}

void EventSystem::EventManager::PopEvent() {
  m_eventQueue.pop();
}

EventSystem::Event EventSystem::EventManager::CreateEvent(EventType Type, const EventInfo* Info) {
  Event event{};

  event.Type = Type;
  if (Info != nullptr)
    event.Info = *Info;

  return event;
}

void EventSystem::EventManager::AddEvent(EventType Type, const EventInfo* Info) {
  PushEvent(CreateEvent(Type, Info));
}

void EventSystem::EventManager::HandleEvents() {
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

void EventSystem::EventManager::RegisterEventListener(UI::ECS::Entity* Object, EventType Channels) {
  EventListenerHandle handle {
    .Object = Object,
    .Expired = false
  };

  if ((Channels & EventType::Mouse) != EventType::Undefined) {
    m_eventChannels[EventTypeEnumToIndex(EventType::Mouse)].push_back(handle);
  }

  if ((Channels & EventType::Window) != EventType::Undefined) {
    m_eventChannels[EventTypeEnumToIndex(EventType::Window)].push_back(handle);
  }

  if ((Channels & EventType::Keyboard) != EventType::Undefined) {
    m_eventChannels[EventTypeEnumToIndex(EventType::Keyboard)].push_back(handle);
  }

  if ((Channels & EventType::Character) != EventType::Undefined) {
    m_eventChannels[EventTypeEnumToIndex(EventType::Character)].push_back(handle);
  }
}

void EventSystem::EventManager::ExpireEventListener(UI::ECS::Entity* Object) {
  for (auto& channel : m_eventChannels) {
    for (auto& listener : channel) {
      if (listener.Object == Object)
        listener.Expired = true;
    }
  }
}

void EventSystem::EventManager::ExpireEventListenerOnChannel(UI::ECS::Entity* Object, EventType Channel) {
  int channelIndex = EventTypeEnumToIndex(Channel);
  if (channelIndex >= EventTypeCount) return;

  for (auto& listener : m_eventChannels[channelIndex]) {
    if (listener.Object == Object)
      listener.Expired = true;
  }
}

} // namespace CustomIDE
