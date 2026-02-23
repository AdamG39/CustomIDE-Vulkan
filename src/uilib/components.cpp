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
   MoveCursorRight();
}

void TextBox::Insert(std::string Content, int Position) {

}

void TextBox::Delete(int Position, int Count) {

}

void TextBox::MoveCursorLeft() {
  m_cursor.Position = std::max(0, m_cursor.Position - 1);
}

void TextBox::MoveCursorRight() {
  m_cursor.Position = std::min(static_cast<size_t>(m_cursor.Position + 1), GetContent().size());
}

void TextBox::MoveCursorUp() {
  const std::vector<int>& newLines = m_table.GetNewLines();

  // if only 1 line (aka no new lines) just jump to start of line
  if (newLines.empty()) {
    m_cursor.Position = 0;
    return;
  }
  // if on the first line just jump to start of line
  if (newLines.front() >= m_cursor.Position) {
    m_cursor.Position = 0;
    return;
  }

  // Looks for start of line that cursor is on if not found assumes on first line
  int startOfLine = -1;
  for (int i = newLines.size() - 1; i >= 0; i--) {
    if (newLines[i] < m_cursor.Position) {
      startOfLine = i;
      break;
    }
  }

  int previousLine = ((startOfLine - 1) >= 0) ? newLines[startOfLine - 1] + 1 : 0; // Add one to account for newline character
  int cursorLineOffset = m_cursor.Position - ((startOfLine < 0) ? 0 : newLines[startOfLine] + 1);
  m_cursor.Position = std::min(newLines[startOfLine], previousLine + cursorLineOffset);
}

void TextBox::MoveCursorDown() {
  const std::vector<int>& newLines = m_table.GetNewLines();
  int contentSize = GetContent().size();

  // if only 1 line (aka no new lines) just jump to end of line
  if (newLines.empty()) {
    m_cursor.Position = contentSize;
    return;
  }
  // if on the last line just jump to end of line
  if (newLines.back() < m_cursor.Position) {
    m_cursor.Position = contentSize;
    return;
  }

  // Looks for start of line that cursor is on if not found assumes on first line
  int startOfLine = -1;
  for (int i = newLines.size() - 1; i >= 0; i--) {
    if (newLines[i] < m_cursor.Position) {
      startOfLine = i;
      break;
    }
  }

  int nextLine = newLines[startOfLine + 1];
  int cursorLineOffset = m_cursor.Position - ((startOfLine < 0) ? 0 : newLines[startOfLine]);
  int lineAfter = ((startOfLine + 2) >= newLines.size()) ? contentSize : newLines[startOfLine + 2];
  int lineLength = lineAfter - nextLine + 1; // Add 1 to account for newline character
  if (m_cursor.Position == 0)
    m_cursor.Position = nextLine + 1;
  else
    m_cursor.Position = nextLine + std::min(lineLength, cursorLineOffset);
}

