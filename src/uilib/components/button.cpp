#include "button.hpp"

namespace CustomIDE {

int UI::ECS::Button::GetType() { return TypeValue(); }

void UI::ECS::Button::OnPress() {
  ExecAction("OnPress");
}

void UI::ECS::Button::OnRelease() {
  ExecAction("OnRelease");
}

void UI::ECS::Button::OnHoverEnter() {
  ExecAction("OnHoverEnter");
}

void UI::ECS::Button::OnHoverExit() {
  ExecAction("OnHoverExit");
}

} // namespace CustomIDE

