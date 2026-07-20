#include "transform.hpp"

namespace CustomIDE {

int UI::ECS::Transform::GetType() { return TypeValue(); }

void UI::ECS::Transform::SetSize(const Vector2<UI::Size<float>>& Size) {
  m_size = Size; 
}

Vector2<UI::Size<float>> UI::ECS::Transform::GetSize() const {
  return m_size;
}

Vector2<float> UI::ECS::Transform::RecalculateEntitySize(float ParentWidth, float ParentHeight) {
  Vector2<float> calculatedSize;

  if (m_size.x.Mode == SizeMode::Proportional) {
    calculatedSize.x = ParentWidth * m_size.x.Value;
  } else { calculatedSize.x = m_size.x.Value; }

  if (m_size.y.Mode == SizeMode::Proportional) {
    calculatedSize.y = ParentHeight * m_size.y.Value;
  } else { calculatedSize.y = m_size.y.Value; }

  m_pixelSize = calculatedSize;
  return calculatedSize;
}

Vector2<float> UI::ECS::Transform::GetPixelSize() const {
  return m_pixelSize;
}

void UI::ECS::Transform::SetPosition(const Vector2<UI::Size<float>>& Position) {
  m_position = Position;
}

Vector2<UI::Size<float>> UI::ECS::Transform::GetPosition() const {
  return m_position;
}

// TODO: can child object can only use center anchor
// change to allow better calculation of objects based on parents and anchors
Vector2<float> UI::ECS::Transform::RecalculateEntityPosition(Vector2<float> ParentSize, 
    Vector2<float> ParentPosition, const UI::AnchorType& Anchor) {
  Vector2<float> calculatedPosition;

  switch (Anchor) {
    case UI::AnchorType::Center:
      calculatedPosition.x = ParentSize.x / 2;
      calculatedPosition.y = ParentSize.y / 2;
      break;
    case UI::AnchorType::Top:
      calculatedPosition.x = ParentSize.x / 2;
      calculatedPosition.y = 0;
      break;
    case UI::AnchorType::Left:
      calculatedPosition.x = 0;
      calculatedPosition.y = ParentSize.y / 2;
      break;
    case UI::AnchorType::Right:
      calculatedPosition.x = ParentSize.x;
      calculatedPosition.y = ParentSize.y / 2;
      break;
    case UI::AnchorType::Bottom:
      calculatedPosition.x = ParentSize.x / 2;
      calculatedPosition.y = ParentSize.y;
      break;
    case UI::AnchorType::TopLeft:
      calculatedPosition.x = 0;
      calculatedPosition.y = 0;
      break;
    case UI::AnchorType::TopRight:
      calculatedPosition.x = ParentSize.x;
      calculatedPosition.y = 0;
      break;
    case UI::AnchorType::BottomLeft:
      calculatedPosition.x = 0;
      calculatedPosition.y = ParentSize.y;
      break;
    case UI::AnchorType::BottomRight:
      calculatedPosition.x = ParentSize.x;
      calculatedPosition.y = ParentSize.y;
      break;
  }

  if (m_position.x.Mode == SizeMode::Proportional) {
    calculatedPosition.x = ParentPosition.x + m_position.x.Value;
  } else { calculatedPosition.x += m_position.x.Value; }


  if (m_position.y.Mode == SizeMode::Proportional) {
    calculatedPosition.y = ParentPosition.y + m_position.y.Value;
  } else { calculatedPosition.y += m_position.y.Value; }


  m_pixelPosition = calculatedPosition;
  return calculatedPosition;
}

Vector2<float> UI::ECS::Transform::GetPixelPosition() const {
  return m_pixelPosition;
}

void UI::ECS::Transform::SetAnchor(const UI::AnchorType& AnchorValue) {
  m_anchor = AnchorValue;
}

UI::AnchorType UI::ECS::Transform::GetAnchor() const {
  return m_anchor;
}

} // namespace CustomIDE

