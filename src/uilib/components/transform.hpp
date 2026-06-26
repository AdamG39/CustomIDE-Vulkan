#ifndef CUSTOM_COMPONENT_TRANSFORM_H
#define CUSTOM_COMPONENT_TRANSFORM_H

#include "component.hpp"
#include "../../renderer/shapes.hpp"
#include "../ui.hpp"

class Transform : public IComponent {
private:
  Vector2<UISize<float>> m_size;
  Vector2<UISize<float>> m_position;
  UIAnchorType m_anchor = UIAnchorType::Center;

  Vector2<float> m_pixelSize;
  Vector2<float> m_pixelPosition;

public:
  static int TypeValue() { return TypeTransform; }
  int GetType() override;

  Transform() = default;
  Transform(Vector2<UISize<float>> Size, Vector2<UISize<float>> Position)
  : m_size(Size), m_position(Position), m_anchor(UIAnchorType::Center) {}

  void SetSize(const Vector2<UISize<float>>& Size);

  Vector2<UISize<float>> GetSize() const;

  Vector2<float> RecalculateEntitySize(float ParentWidth, float ParentHeight);

  Vector2<float> GetPixelSize() const;

  void SetPosition(const Vector2<UISize<float>>& Position);

  Vector2<UISize<float>> GetPosition() const;

  Vector2<float> RecalculateEntityPosition(Vector2<float> ParentSize, 
    Vector2<float> ParentPosition, const UIAnchorType& Anchor);

  Vector2<float> GetPixelPosition() const;

  void SetAnchor(const UIAnchorType& AnchorValue);

  UIAnchorType GetAnchor() const;
};

#endif

