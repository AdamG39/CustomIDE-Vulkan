#include "button.hpp"

int Button::GetType() { return TypeValue(); }

void Button::OnPress() {
  if (m_onPress) {
    m_onPress();
  } else {
    printf("[Warning]: No OnPress() function assigned for object: %p", this);
  }
}

void Button::OnRelease() {
  if (m_onRelease) {
    m_onRelease();
  } else
    printf("[Warning]: No OnRelease() function assigned for object: %p", this);
}

void Button::OnHover() {
  if (m_onHover) {
    m_onHover();
  } else
    printf("[Warning]: No OnHover() function assigned for object: %p", this);
}

