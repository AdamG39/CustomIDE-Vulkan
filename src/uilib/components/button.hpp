#ifndef CUSTOM_COMPONENT_BUTTON_H
#define CUSTOM_COMPONENT_BUTTON_H

#include "component.hpp"
#include <functional>

class Button : public IInteractable {
public:
  static int TypeValue() { return TypeButton; }
  int GetType() override;

  Button() = default;

  template <typename... FnParams, typename... FnArgs>
  void SetOnPress(void(*Function)(FnParams... Parameters), FnArgs... Arguments) {
    AddAction("OnPress", Function, Arguments...);
  }

  template <typename... FnParams, typename... FnArgs>
  void SetOnRelease(void(*Function)(FnParams... Parameters), FnArgs... Arguments) {
    AddAction("OnRelease", Function, Arguments...);
  }

  template <typename... FnParams, typename... FnArgs>
  void SetOnHover(void(*Function)(FnParams... Parameters), FnArgs... Arguments) {
    AddAction("OnHover", Function, Arguments...);
  }

  void OnPress();

  void OnRelease();

  void OnHover();
};

#endif

