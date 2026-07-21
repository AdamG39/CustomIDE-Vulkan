#ifndef CUSTOM_COMPONENT_TRANSFORM_H
#define CUSTOM_COMPONENT_TRANSFORM_H

#include "component.hpp"
#include "../../renderer/shapes.hpp"
#include "../ui.hpp"

namespace CustomIDE::UI::ECS {

class Transform : public IComponent {
private:
  Vector2<UI::Size<float>> m_size;
  Vector2<UI::Size<float>> m_position;
  UI::AnchorType m_anchor = UI::AnchorType::Center;

  Vector2D m_pixelSize;
  Vector2D m_pixelPosition;

public:
  static int TypeValue() { return TypeTransform; }
  int GetType() override;

  Transform() = default;
  Transform(Vector2<UI::Size<float>> Size, Vector2<UI::Size<float>> Position)
  : m_size(Size), m_position(Position), m_anchor(UI::AnchorType::Center) {}

  void SetSize(const Vector2<UI::Size<float>>& Size);

  Vector2<UI::Size<float>> GetSize() const;

  Vector2D RecalculateEntitySize(float ParentWidth, float ParentHeight);

  Vector2D GetPixelSize() const;

  void SetPosition(const Vector2<UI::Size<float>>& Position);

  Vector2<UI::Size<float>> GetPosition() const;

  Vector2D RecalculateEntityPosition(Vector2D ParentSize, 
    Vector2D ParentPosition, const UI::AnchorType& Anchor);

  Vector2D GetPixelPosition() const;

  void SetAnchor(const UI::AnchorType& AnchorValue);

  UI::AnchorType GetAnchor() const;
};

} // namespace UI::ECS

#endif

