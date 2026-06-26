#ifndef CUSTOM_COMPONENT_MASK_H
#define CUSTOM_COMPONENT_MASK_H

#include "component.hpp"

class Mask : public IComponent {
private:
  ClipRect m_clipRect;

public:
  static int TypeValue() { return TypeMask; }
  int GetType() override;

  Mask(ClipRect Rect = {});

  ClipRect GetClipArea();
};

#endif

