#ifndef CUSTOM_COMPONENT_BUTTON_H
#define CUSTOM_COMPONENT_BUTTON_H

#include "component.hpp"

namespace CustomIDE::UI::ECS {

class Button : public IInteractable {
public:
  static int TypeValue() { return TypeButton; }
  int GetType() override;

  Button() = default;

  template <typename Func, typename... Args>
  void SetOnPress(Func&& Function, Args&&... Arguments) {
    AddAction("OnPress", Function, Arguments...);
  }

  template <typename Func, typename... Args>
  void SetOnRelease(Func&& Function, Args&&... Arguments) {
    AddAction("OnRelease", Function, Arguments...);
  }

  template <typename Func, typename... Args>
  void SetOnHoverEnter(Func&& Function, Args&&... Arguments) {
    AddAction("OnHoverEnter", Function, Arguments...);
  }

  template <typename Func, typename... Args>
  void SetOnHoverExit(Func&& Function, Args&&... Arguments) {
    AddAction("OnHoverExit", Function, Arguments...);
  }

  void OnPress();

  void OnRelease();

  void OnHoverEnter();

  void OnHoverExit();
};

} // namespace UI::ECS

#endif

