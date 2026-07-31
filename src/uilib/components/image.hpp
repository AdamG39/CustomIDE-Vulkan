#ifndef CUSTOM_COMPONENT_IMAGE_H
#define CUSTOM_COMPONENT_IMAGE_H

#include "component.hpp"

namespace CustomIDE::UI::ECS {

class Image : public IRenderable {
private:
  Colour m_colour;
  UVRect2D m_uvRect = { 0.5f, 0.5f, 1.f, 1.f };
  int m_textureIndex;

public:
  static int TypeValue() { return TypeImage; }
  int GetType() override;

  Image(const Colour& _Colour = Colour(), int TextureIndex = -1)
  : m_colour(_Colour), m_textureIndex(TextureIndex) {}

  void Render(EntityManager& Manager, Transform* Transform) override;

  void SetColour(const Colour& Colour);
  Colour GetColour() const;

  void SetUVRect(UVRect2D UVRect);
  const UVRect2D& GetUVRect() const;

  void SetTextureIndex(int TextureIndex);
  int GetTextureIndex() const;
};

} // namespace UI::ECS

#endif

