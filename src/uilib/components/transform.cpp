#include "transform.hpp"

int Transform::GetType() { return TypeValue(); }

void Transform::SetSize(const Vector2<UISize<float>>& Size) {
  m_size = Size; 
}

Vector2<UISize<float>> Transform::GetSize() const {
  return m_size;
}

Vector2<float> Transform::RecalculateEntitySize(float ParentWidth, float ParentHeight) {
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

Vector2<float> Transform::GetPixelSize() const {
  return m_pixelSize;
}

void Transform::SetPosition(const Vector2<UISize<float>>& Position) {
  m_position = Position;
}

Vector2<UISize<float>> Transform::GetPosition() const {
  return m_position;
}

// TODO: can child object can only use center anchor
// change to allow better calculation of objects based on parents and anchors
Vector2<float> Transform::RecalculateEntityPosition(Vector2<float> ParentSize, 
    Vector2<float> ParentPosition, const UIAnchorType& Anchor) {
  Vector2<float> calculatedPosition;

  switch (Anchor) {
    case UIAnchorType::Center:
      calculatedPosition.x = ParentSize.x / 2;
      calculatedPosition.y = ParentSize.y / 2;
      break;
    case UIAnchorType::Top:
      calculatedPosition.x = ParentSize.x / 2;
      calculatedPosition.y = 0;
      break;
    case UIAnchorType::Left:
      calculatedPosition.x = 0;
      calculatedPosition.y = ParentSize.y / 2;
      break;
    case UIAnchorType::Right:
      calculatedPosition.x = ParentSize.x;
      calculatedPosition.y = ParentSize.y / 2;
      break;
    case UIAnchorType::Bottom:
      calculatedPosition.x = ParentSize.x / 2;
      calculatedPosition.y = ParentSize.y;
      break;
    case UIAnchorType::TopLeft:
      calculatedPosition.x = 0;
      calculatedPosition.y = 0;
      break;
    case UIAnchorType::TopRight:
      calculatedPosition.x = ParentSize.x;
      calculatedPosition.y = 0;
      break;
    case UIAnchorType::BottomLeft:
      calculatedPosition.x = 0;
      calculatedPosition.y = ParentSize.y;
      break;
    case UIAnchorType::BottomRight:
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

Vector2<float> Transform::GetPixelPosition() const {
  return m_pixelPosition;
}

void Transform::SetAnchor(const UIAnchorType& AnchorValue) {
  m_anchor = AnchorValue;
}

UIAnchorType Transform::GetAnchor() const {
  return m_anchor;
}

