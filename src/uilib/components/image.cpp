#include "image.hpp"
#include "transform.hpp"
#include "../ecs.hpp"

int UIImage::GetType() { return TypeValue(); }

void UIImage::Render(EntityManager& Manager, const Transform* Transform) {
  auto pos = Transform->GetPixelPosition();
  auto size = Transform->GetPixelSize();

  ClipRect clipRect {.clippingEnabled = false};
  if (!Manager.GetClipStack().empty())
    clipRect = Manager.GetClipStack().top();

  Manager.GetRenderer().lock()->DrawTexturedRectEx(
      Rect2D{ static_cast<int32_t>(pos.x), static_cast<int32_t>(pos.y),
              static_cast<uint32_t>(size.x), static_cast<uint32_t>(size.y) },
      m_uvRect, m_drawDepth, m_textureIndex, clipRect, m_colour);
}

void UIImage::SetColour(const Colour& Colour) {
  m_colour = Colour;
}

Colour UIImage::GetColour() const {
  return m_colour;
}

void UIImage::SetUVRect(UVRect2D UVRect) {
  m_uvRect = UVRect;
}

const UVRect2D& UIImage::GetUVRect() const {
  return m_uvRect;
}

void UIImage::SetTextureIndex(int TextureIndex) {
  m_textureIndex = TextureIndex;
}

int UIImage::GetTextureIndex() const {
  return m_textureIndex;
}

