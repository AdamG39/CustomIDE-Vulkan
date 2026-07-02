#include "button.hpp"

int Button::GetType() { return TypeValue(); }

void Button::OnPress() {
  ExecAction("OnPress");
}

void Button::OnRelease() {
  ExecAction("OnRelease");
}

void Button::OnHover() {
  ExecAction("OnHover");
}

