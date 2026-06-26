#ifndef CUSTOM_COMPONENT_BUTTON_H
#define CUSTOM_COMPONENT_BUTTON_H

#include "component.hpp"
#include <functional>

class Button : public IComponent {
private:
  std::function<void()> m_onPress;
  std::function<void()> m_onRelease;
  std::function<void()> m_onHover;

public:
  static int TypeValue() { return TypeButton; }
  int GetType() override;

  Button() = default;

  template <typename Func, typename... Args>
  void SetOnPress(Func&& Function, Args&&... Arguments) {
    m_onPress = std::bind(std::forward<Func>(Function), std::decay_t<Args>(Arguments)...);
  }

  template <typename Func, typename... Args>
  void SetOnRelease(Func&& Function, Args&&... Arguments) {
    m_onRelease = std::bind(std::forward<Func>(Function), std::decay_t<Args>(Arguments)...);
  }

  template <typename Func, typename... Args>
  void SetOnHover(Func&& Function, Args&&... Arguments) {
    m_onHover = std::bind(std::forward<Func>(Function), std::decay_t<Args>(Arguments)...);
  }

  void OnPress();

  void OnRelease();

  void OnHover();
};

#endif

