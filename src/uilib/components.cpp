#include "components.hpp"
#include "../helpers/errors/errors.hpp"
#include "ecs.hpp"

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

void UIImage::Render(EntityManager& Manager,
                     const Transform* Transform,
                     Vector2<float> DrawArea) {
  Rect<float, float> result = Rect<float, float>(
      Transform->GetPixelSize(DrawArea.x, DrawArea.y),
      Transform->GetPixelPosition(DrawArea.x, DrawArea.y),
      m_colour,
      m_drawDepth
    );
  result.SetTextureCoords(m_textureCoords);
  result.SetTextureIndex(m_textureIndex);
  Manager.AddGeometry(result);
}

void IText::Render(EntityManager& Manager,
                   const Transform* Transform,
                   Vector2<float> DrawArea) {
  // calculate size of each character based on font
  Font font = GetFont();
  Vector2 textObjPos = Transform->GetPixelPosition();
  Vector2 fontAtlasSize {64.f, 2.f};
  std::string content = GetContent();

  int charsPerLine = int(Transform->GetPixelSize().x) / font.size.x;
  // create a rect for each character
  int linePosition = 0;
  int lineCount = 0;

  for (size_t i = 0; i < content.size(); i++) {
    switch (content[i]) {
    case '\n':
      lineCount++;
      linePosition = 0;
      continue;
    case '\t':
      linePosition += 4;
      if (GetWordWrap()) {
        if (linePosition > charsPerLine) { 
          lineCount++;
          linePosition = 0;
        }
      }
      continue;
    }

    Vector2<float> charPosition {
      textObjPos.x + (font.size.x * linePosition),
      textObjPos.y + (lineCount * font.size.y)
    };

    linePosition++;

    if (GetWordWrap()) {
      if (linePosition > charsPerLine) { 
        lineCount++;
        linePosition = 0;
      }
    }

    Colour<float> textColour = font.colour;
    Rect<float, float> result = Rect<float, float>(
        font.size,
        charPosition,
        textColour,
        m_drawDepth
      );
    auto imageIndex = Manager.GetRenderer().GetImageIndexFromName(font.familyName);
    if (imageIndex < 0) ExitWithError("No image with that name found", -35);
    result.SetTextureIndex(imageIndex);
    result.SetTextureCoords(CalculateCharTextureCoords(fontAtlasSize, content[i]));
    Manager.AddGeometry(result);
  }
}

void TextBox::Render(EntityManager& Manager,
                     const Transform* Transform,
                     Vector2<float> DrawArea) {
  // calculate size of each character based on font
  Font font = GetFont();
  Vector2 textObjPos = Transform->GetPixelPosition();
  Vector2 fontAtlasSize {64.f, 2.f};
  std::string content = GetContent();

  int charsPerLine = int(Transform->GetPixelSize().x) / font.size.x;
  // create a rect for each character
  int linePosition = 0;
  int lineCount = 0;

  int cursorIndexPosition = GetCursorPosition();
  Vector2<int> cursorPosition;

  for (size_t i = 0; i < content.size(); i++) {
    switch (content[i]) {
    case '\n':
      if (cursorIndexPosition == i)
        cursorPosition = { linePosition, lineCount };
      lineCount++;
      linePosition = 0;
      continue;
    case '\t':
      if (cursorIndexPosition == i)
        cursorPosition = { linePosition, lineCount };
      linePosition += 4;
      if (GetWordWrap()) {
        if (linePosition > charsPerLine) { 
          lineCount++;
          linePosition = 0;
        }
      }
      continue;
    }

    Vector2<float> charPosition {
      textObjPos.x + (font.size.x * linePosition),
      textObjPos.y + (lineCount * font.size.y)
    };

    if (cursorIndexPosition == i)
      cursorPosition = { linePosition, lineCount };

    linePosition++;

    if (GetWordWrap()) {
      if (linePosition > charsPerLine) { 
        lineCount++;
        linePosition = 0;
      }
    }

    Colour<float> textColour = font.colour;
    if (cursorIndexPosition == i) {
      textColour.r = 1.f - textColour.r;
      textColour.g = 1.f - textColour.g;
      textColour.b = 1.f - textColour.b;
    }
    Rect<float, float> result = Rect<float, float>(
      font.size,
      charPosition,
      textColour,
      m_drawDepth
    );
    auto imageIndex = Manager.GetRenderer().GetImageIndexFromName(font.familyName);
    if (imageIndex < 0) ExitWithError("No image with that name found", -35);
    result.SetTextureIndex(imageIndex);
    result.SetTextureCoords(CalculateCharTextureCoords(fontAtlasSize, content[i]));
    Manager.AddGeometry(result);
  }
  if (cursorIndexPosition == content.size())
    cursorPosition = { linePosition, lineCount };

  Vector2<float> finalCursorSize {
    static_cast<float>(font.size.x),
    static_cast<float>(font.size.y) * 1.25f
  };

  Vector2<float> finalCursorPosition {
    textObjPos.x + (font.size.x * cursorPosition.x),
    textObjPos.y + (font.size.y * cursorPosition.y)
  };

  Rect<float, float> result = Rect<float, float>(
    finalCursorSize,
    finalCursorPosition,
    GetCursorColour(),
    m_drawDepth
  );

  Manager.AddGeometry(result);
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

