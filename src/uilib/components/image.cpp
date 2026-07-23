#include "image.hpp"
#include "transform.hpp"
#include "../ecs.hpp"

namespace CustomIDE {

int UI::ECS::Image::GetType() { return TypeValue(); }

void UI::ECS::Image::Render(EntityManager& Manager, Transform* Transform) {
  auto pos = Transform->GetGlobalPosition();
  auto size = Transform->GetGlobalSize();

  ClipRect clipRect {.clippingEnabled = false};
  if (!Manager.GetClipStack().empty())
    clipRect = Manager.GetClipStack().top();

  Manager.GetRenderer().lock()->DrawTexturedRectEx(
      Rect2D{ static_cast<int32_t>(pos.x), static_cast<int32_t>(pos.y),
              static_cast<uint32_t>(size.x), static_cast<uint32_t>(size.y) },
      m_uvRect, m_drawDepth, m_textureIndex, clipRect, m_colour);
}

void UI::ECS::Image::SetColour(const Colour& Colour) {
  m_colour = Colour;
}

Colour UI::ECS::Image::GetColour() const {
  return m_colour;
}

void UI::ECS::Image::SetUVRect(UVRect2D UVRect) {
  m_uvRect = UVRect;
}

const UVRect2D& UI::ECS::Image::GetUVRect() const {
  return m_uvRect;
}

void UI::ECS::Image::SetTextureIndex(int TextureIndex) {
  m_textureIndex = TextureIndex;
}

int UI::ECS::Image::GetTextureIndex() const {
  return m_textureIndex;
}

} // namespace CustomIDE

