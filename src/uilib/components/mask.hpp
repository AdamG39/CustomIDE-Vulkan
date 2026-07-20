#ifndef CUSTOM_COMPONENT_MASK_H
#define CUSTOM_COMPONENT_MASK_H

#include "component.hpp"

namespace CustomIDE::UI::ECS {

class Mask : public IComponent {
private:
  ClipRect m_clipRect;

public:
  static int TypeValue() { return TypeMask; }
  int GetType() override;

  Mask(ClipRect Rect = {});

  ClipRect GetClipArea();
};

} // namespace UI::ECS

#endif

