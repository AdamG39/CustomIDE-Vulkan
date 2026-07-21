#include "mask.hpp"

namespace CustomIDE {

int UI::ECS::Mask::GetType() { return TypeValue(); }

UI::ECS::Mask::Mask(ClipRect Rect) : m_clipRect(Rect) {}

ClipRect UI::ECS::Mask::GetClipArea() {
  return m_clipRect;
}

} // namespace CustomIDE

