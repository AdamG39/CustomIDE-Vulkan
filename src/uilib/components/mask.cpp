#include "mask.hpp"

int Mask::GetType() { return TypeValue(); }

Mask::Mask(ClipRect Rect) : m_clipRect(Rect) {}

ClipRect Mask::GetClipArea() {
  return m_clipRect;
}

