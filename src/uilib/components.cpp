#include "components.hpp"

Vector2<float> Transform::CalculateEntitySize(float ParentWidth, float ParentHeight) const {
  Vector2<float> calculatedSize;

  if (m_size.x.Mode == SizeMode::Proportional) {
    calculatedSize.x = ParentWidth * m_size.x.Value;
  } else { calculatedSize.x = m_size.x.Value; }

  if (m_size.y.Mode == SizeMode::Proportional) {
    calculatedSize.y = ParentHeight * m_size.y.Value;
  } else { calculatedSize.y = m_size.y.Value; }

  return calculatedSize;
}

Vector2<float> Transform::CalculateEntityPosition(float ParentWidth, float ParentHeight, const UIAnchorType& Anchor) const {
  Vector2<float> calculatedPosition;

  switch (Anchor) {
    case UIAnchorType::Center:
      calculatedPosition.x = ParentWidth / 2;
      calculatedPosition.y = ParentHeight / 2;
      break;
    case UIAnchorType::Top:
      calculatedPosition.x = ParentWidth / 2;
      break;
    case UIAnchorType::Left:
      calculatedPosition.y = ParentHeight / 2;
      break;
    case UIAnchorType::Right:
      calculatedPosition.x = ParentWidth;
      calculatedPosition.y = ParentHeight / 2;
      break;
    case UIAnchorType::Bottom:
      calculatedPosition.x = ParentWidth / 2;
      calculatedPosition.y = ParentHeight;
      break;
    case UIAnchorType::TopLeft:         
      // Already relative to 0, 0 so no changes needed
      break;
    case UIAnchorType::TopRight:
      calculatedPosition.x = ParentWidth;
      break;
    case UIAnchorType::BottomLeft:
      calculatedPosition.y = ParentHeight;
      break;
    case UIAnchorType::BottomRight:
      calculatedPosition.x = ParentWidth;
      calculatedPosition.y = ParentHeight;
      break;
  }

  if (m_position.x.Mode == SizeMode::Proportional) {
    calculatedPosition.x += ParentWidth * m_position.x.Value;
  } else { calculatedPosition.x += m_position.x.Value; }

  if (m_position.y.Mode == SizeMode::Proportional) {
    calculatedPosition.y += ParentHeight * m_position.y.Value;
  } else { calculatedPosition.y += m_position.y.Value; }

  return calculatedPosition;
}

void TextBox::Insert(char Character, int Position) {
   m_table.Insert(Character, Position);
   MoveCursorBy(1, Right);
}

void TextBox::Insert(std::string Content, int Position) {

}

void TextBox::Delete(int Position, int Count) {

}

void TextBox::MoveCursorBy(int Amount, TextCursorMoveDirection Direction) {
  switch (Direction) {
    case Left:
      m_cursor.Position = std::max(0, m_cursor.Position - Amount);
      break;
    case Right:
      m_cursor.Position = std::min(static_cast<size_t>(m_cursor.Position + Amount), GetContent().size());
      break;
    case Up:
      puts("unimplemented");
      break;
    case Down:
      puts("unimplemented");
      break;
  }
}

