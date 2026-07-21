#include "textBox.hpp"
#include "transform.hpp"
#include "../ecs.hpp"
#include "../../io/io.hpp"

namespace CustomIDE {

int UI::ECS::TextBox::GetType() { return TypeValue(); }

UI::ECS::TextBox::TextBox(Font Font, std::string Filepath, bool WordWrap)
  : m_filepath(Filepath) {
  LoadFile();
  m_font = Font;
  m_wordWrap = WordWrap;
  m_cursor._Colour = Font.colour;
  m_cursor.Type = TextCursorType::DEFAULT;
}

void UI::ECS::TextBox::RenderSelection(EntityManager& Manager, const Vector2D& TextObjPos) {
  size_t start = m_textSelection.start;
  size_t length = m_textSelection.length;
  size_t end = start + length;

  Font font = GetFont();

  // find which line the selection starts on
  int firstLineStart = -1;
  for (size_t i{0ull}; i < m_table.GetStartOfLines().size(); i++) {
    if (m_table.GetStartOfLines()[i] > start) {
      firstLineStart = i - 1;
      break;
    }
  }

  if (firstLineStart < 0) firstLineStart = 0;

  int startOffsetIntoLine = start - m_table.GetStartOfLines()[firstLineStart];

  std::vector<Rect2D> selectionRects;

  int32_t charSpacing{CHAR_SPACING};

  // If not an even number integer division causes spacing to be handled incorrectly
  if (charSpacing & 1) charSpacing += 1; // Round to nearest even number

  // Generate render geometry
  int currentLine = firstLineStart;
  for (auto i{start}; i < end; i++) {
    // Create initial selection rect
    if (selectionRects.empty()) {
      Rect2D first {
        .xOffset = font.size.x * startOffsetIntoLine ,
        .yOffset = font.size.y * currentLine + (currentLine * LINE_SPACING),
        .width = static_cast<uint32_t>(font.size.x),
        .height = static_cast<uint32_t>(font.size.y) + LINE_SPACING
      };

      selectionRects.push_back(first);
    }

    else {
      auto& rect = selectionRects.back();
      // Adjust the width and xOffset to include the next element
      rect.xOffset += (font.size.x / 2) + (charSpacing / 2);
      rect.width += static_cast<uint32_t>(font.size.x) + charSpacing;
    }

    if (m_table.GetContent()[i] == '\n') {
      currentLine++;
      Rect2D newSelectionLine {
        .xOffset = -(font.size.x / 2), // Line could be empty so needs to start before the line
        .yOffset = font.size.y * currentLine + (currentLine * LINE_SPACING),
        .width = 0,
        .height = static_cast<uint32_t>(font.size.y) + LINE_SPACING
      };

      selectionRects.push_back(newSelectionLine);
    }
  }

  // Render all selection rects
  for (auto rect : selectionRects) {
    rect.yOffset += TextObjPos.y;
    rect.xOffset += TextObjPos.x;
    Manager.GetRenderer().lock()->DrawRect(
        rect,
        m_drawDepth, Colour(0x90D5FF, 0.5f));
  }
}

void UI::ECS::TextBox::RenderCursor(EntityManager& Manager, const Vector2D& TextObjPos, const Vector2<int>& CursorPosition) {
  Font font = GetFont();

  float widthMultiplier{};

  switch (m_cursor.Type) {
    case TextCursorType::DEFAULT:
      widthMultiplier = DEFAULT_WIDTH_MULTIPLIER;
      break;

    case TextCursorType::BLOCK:
      widthMultiplier = BLOCK_WIDTH_MULTIPLIER;
      break;
  }

  Vector2D finalCursorSize {
    static_cast<float>(font.size.x) * widthMultiplier,
    static_cast<float>(font.size.y) * CURSOR_HEIGHT_MULTIPLIER
  };

  float xOffset{};

  switch (m_cursor.Type) {
    case TextCursorType::DEFAULT:
      xOffset = DEFAULT_X_OFFSET;
      break;

    case TextCursorType::BLOCK:
      xOffset = BLOCK_X_OFFSET;
      break;
  }

  xOffset *= font.size.x;

  int32_t charSpacing{CHAR_SPACING};

  // If not an even number integer division causes spacing to be handled incorrectly
  if (charSpacing & 1) charSpacing += 1; // Round to nearest even number

  Vector2D finalCursorPosition {
    TextObjPos.x + (font.size.x * CursorPosition.x) + xOffset + (CursorPosition.x * charSpacing),
    TextObjPos.y + (font.size.y * CursorPosition.y) + (CursorPosition.y * LINE_SPACING)
  };

  Manager.GetRenderer().lock()->DrawRect(
      Rect2D{ static_cast<int32_t>(finalCursorPosition.x), static_cast<int32_t>(finalCursorPosition.y),
              static_cast<uint32_t>(finalCursorSize.x), static_cast<uint32_t>(finalCursorSize.y) },
      m_drawDepth, GetCursorColour());
}

void UI::ECS::TextBox::Render(EntityManager& Manager, const Transform* Transform) {
  // calculate size of each character based on font
  Font font = GetFont();
  Vector2D textObjPos = Transform->GetPixelPosition();
  Vector2D fontAtlasSize {64.f, 2.f};
  std::string content = GetContent();

  int charsPerLine = int(Transform->GetPixelSize().x) / font.size.x;
  // create a rect for each character
  int linePosition = 0;
  int lineCount = 0;

  int cursorIndexPosition = GetCursorPosition();
  Vector2<int> cursorPosition;

  for (size_t i = 0; i < content.size(); i++) {
    if (cursorIndexPosition == i)
      cursorPosition = { linePosition, lineCount };

    switch (content[i]) {
    case '\n':
      lineCount++;
      linePosition = 0;
      continue;
    case '\t':
      linePosition += TAB_WIDTH;
      if (GetWordWrap()) {
        if (linePosition > charsPerLine) { 
          lineCount++;
          linePosition = 0;
        }
      }
      continue;
    case ' ':
      linePosition++;
      continue;
    }

    int32_t charSpacing{CHAR_SPACING};

    // If not an even number integer division causes spacing to be handled incorrectly
    if (charSpacing & 1) charSpacing += 1; // Round to nearest even number

    Vector2D charPosition {
      textObjPos.x + (font.size.x * linePosition) + (linePosition * charSpacing),
      textObjPos.y + (lineCount * font.size.y) + (lineCount * LINE_SPACING)
    };

    linePosition++;

    if (GetWordWrap()) {
      if (linePosition > charsPerLine) { 
        lineCount++;
        linePosition = 0;
      }
    }

    Colour textColour = font.colour;
    if (cursorIndexPosition == i && m_cursor.Type == TextCursorType::BLOCK) {
      textColour.r = 1.f - textColour.r;
      textColour.g = 1.f - textColour.g;
      textColour.b = 1.f - textColour.b;
    }
    auto imageIndex = Manager.GetRenderer().lock()->GetImageIndexFromName(font.familyName);
    if (imageIndex < 0) Errors::ExitWithError("No image with that name found", -35);

    ClipRect clipRect {};
    if (!Manager.GetClipStack().empty())
      clipRect = Manager.GetClipStack().top();

    Manager.GetRenderer().lock()->DrawTexturedRectEx(
        Rect2D{ static_cast<int32_t>(charPosition.x), static_cast<int32_t>(charPosition.y),
                static_cast<uint32_t>(font.size.x), static_cast<uint32_t>(font.size.y) },
        CalculateCharUV(fontAtlasSize, content[i]), m_drawDepth + 1, imageIndex, clipRect,
        textColour);
  }

  if (GetSelectionState()) {
    RenderSelection(Manager, textObjPos);
  }

  if (cursorIndexPosition == GetContent().size())
    cursorPosition = { linePosition, lineCount };

  RenderCursor(Manager, textObjPos, cursorPosition);
}

char UI::ECS::TextBox::Index(unsigned Position) {
  return m_table.Index(Position);
}

void UI::ECS::TextBox::Insert(char Character, int Position) {
  CancelSelection();
  size_t moveAmount = m_table.Insert(Character, Position);
  while (moveAmount--)
    MoveCursorRight();
}

void UI::ECS::TextBox::Delete(int Position) {
  m_table.Delete(Position);
}

int UI::ECS::TextBox::GetCursorPosition() const {
  return m_cursor.Position;
}

void UI::ECS::TextBox::MoveCursorLeft() {
  m_cursor.Position = std::max(0, m_cursor.Position - 1);
}

void UI::ECS::TextBox::MoveCursorRight() {
  m_cursor.Position = std::min(static_cast<size_t>(m_cursor.Position + 1), GetContent().size());
}

#include <cassert>

void UI::ECS::TextBox::MoveCursorUp() {
  const std::vector<int>& startOfLines = m_table.GetStartOfLines();

  // if only 1 line just jump to start of line
  if (startOfLines.size() == 1) {
    m_cursor.Position = startOfLines[0];
    return;
  }

  // if at the start of the first line dont move
  if (m_cursor.Position == startOfLines[0]) {
    return;
  }

  // Looks for start of the line that cursor is on if not found if not found then must be on the last line
  int startOfLine = -1;
  for (int i = startOfLines.size() - 1; i >= 0; i--) {
    if (startOfLines[i] <= m_cursor.Position) {
      startOfLine = i;
      break;
    }
  }

  assert(startOfLine != -1);

  int previousLineStart = ((startOfLine - 1) >= 0) ? startOfLines[startOfLine - 1] : 0;
  int cursorLineOffset = m_cursor.Position - startOfLines[startOfLine];
  m_cursor.Position = std::max(std::min(startOfLines[startOfLine] - 1, previousLineStart + cursorLineOffset), 0);
}

void UI::ECS::TextBox::MoveCursorDown() {
  const std::vector<int>& startOfLines = m_table.GetStartOfLines();
  int contentSize = GetContent().size();

  // if only 1 line just jump to end of line
  if (startOfLines.size() == 1) {
    m_cursor.Position = contentSize;
    return;
  }
  // if on the last line just jump to end of line
  if (startOfLines.back() <= m_cursor.Position) {
    m_cursor.Position = contentSize;
    return;
  }

  // Looks for start of line that cursor is on if not found assumes on first line
  int startOfLine = -1;
  for (int i = startOfLines.size() - 1; i >= 0; i--) {
    if (startOfLines[i] <= m_cursor.Position) {
      startOfLine = i;
      break;
    }
  }

  assert(startOfLine != -1);

  int nextLine = startOfLines[startOfLine + 1];
  int cursorLineOffset = m_cursor.Position - startOfLines[startOfLine];
  int lineAfter = ((startOfLine + 2) >= startOfLines.size()) ? contentSize : startOfLines[startOfLine + 2];
  int lineLength = std::max(lineAfter - nextLine - 1, 0); // Sub 1 to account for newline character
  m_cursor.Position = nextLine + std::min(lineLength, cursorLineOffset);
}

void UI::ECS::TextBox::MoveBackWord() {
  MoveCursorLeft();

  // find end of previous word
  while (isspace(GetContent()[m_cursor.Position])) {
    MoveCursorLeft();

    if (m_cursor.Position == 0) return;
  }

  // find start of previous word
  if (isalnum(GetContent()[m_cursor.Position])) {
    MoveCursorLeft();
    while (isalnum(GetContent()[m_cursor.Position])) {
      // while cursor is on a alphanumeric character
      MoveCursorLeft();

      if (m_cursor.Position == 0) return;
    }
  } else if (!isspace(GetContent()[m_cursor.Position])) {
    MoveCursorLeft();

    while (!isspace(GetContent()[m_cursor.Position]) &&
        !isalnum(GetContent()[m_cursor.Position])) {
      // while cursor is on a alphanumeric character
      MoveCursorLeft();

      if (m_cursor.Position == 0) return;
    }
  }

  MoveCursorRight();
}

void UI::ECS::TextBox::MoveForwardWord() {
  if (isalnum(GetContent()[m_cursor.Position])) {
    MoveCursorRight();
    // find end of this word
    while (isalnum(GetContent()[m_cursor.Position])) {
      // while cursor is on a alphanumeric character
      MoveCursorRight();

      if (m_cursor.Position == GetContent().size()) return;
    }
  } else if (!isspace(GetContent()[m_cursor.Position])) {
    MoveCursorRight();

    while (!isspace(GetContent()[m_cursor.Position]) &&
        !isalnum(GetContent()[m_cursor.Position])) {
      // while cursor is on a alphanumeric character
      MoveCursorRight();

      if (m_cursor.Position == GetContent().size()) return;
    }
  } else if (GetContent()[m_cursor.Position] == '\n') {
    MoveCursorRight();
    return;
  }

  // find start of next word
  while (isspace(GetContent()[m_cursor.Position]) && GetContent()[m_cursor.Position] != '\n') {
    MoveCursorRight();

    if (m_cursor.Position == GetContent().size()) return;
  }
}

bool UI::ECS::TextBox::GetSelectionState() const {
  return m_selectionState;
}

void UI::ECS::TextBox::StartSelection() {
  m_textSelection = {
    .start = static_cast<size_t>(m_cursor.Position),
    .length = 0ull
  };

  m_selectionState = true;
  m_selectionDirection = None;
}

void UI::ECS::TextBox::UpdateSelection(int PreviousPosition) {
  // If called with no selection or no need to update just early return
  if (!m_selectionState || m_cursor.Position == PreviousPosition) return;

  switch (m_selectionDirection) {
    case None:
      if (m_cursor.Position > PreviousPosition) { // Moved to the right
        m_selectionDirection = Right;
        m_textSelection.start = PreviousPosition;
      }
      else { // Moved to the left
        m_selectionDirection = Left;
        m_textSelection.start = m_cursor.Position;
      }

      m_textSelection.length = abs(m_cursor.Position - PreviousPosition);
      break;
    case Left:
      if (m_cursor.Position > m_textSelection.start + m_textSelection.length) m_selectionDirection = Right;

      // Still moving to the left
      if (m_selectionDirection == Left) {
        m_textSelection.start = m_cursor.Position;
        m_textSelection.length += PreviousPosition - m_cursor.Position;
      }

      // Now moving to the right
      else {
        m_textSelection.start += m_textSelection.length;
        m_textSelection.length = m_cursor.Position - m_textSelection.start;
      }
      break;
    case Right:
      if (m_cursor.Position < m_textSelection.start) m_selectionDirection = Left;
      int difference = m_cursor.Position - PreviousPosition;

      // Still moving to the right
      if (m_selectionDirection == Right)
        m_textSelection.length += difference;

      // Now moving to the left
      else {
        auto oldStart = m_textSelection.start;
        m_textSelection.start += difference + m_textSelection.length;
        m_textSelection.length = oldStart - m_textSelection.start;
      }
      break;
  }
  if (m_textSelection.length == 0) CancelSelection();
}

void UI::ECS::TextBox::CancelSelection() {
  m_selectionState = false;
  m_selectionDirection = None;
}

void UI::ECS::TextBox::CopySelection(GLFWwindow* Window) {
  if (!m_selectionState) {
    if (m_cursor.Position == GetContent().size()) return;
    std::string currentCharacter;
    currentCharacter.push_back(GetContent()[m_cursor.Position]);
    glfwSetClipboardString(Window, currentCharacter.c_str());
    return;
  }

  std::string selectedText = GetContent().substr(m_textSelection.start, m_textSelection.length);
  glfwSetClipboardString(Window, selectedText.c_str());
}

void UI::ECS::TextBox::PasteText(GLFWwindow* Window) {
  std::string text = glfwGetClipboardString(Window);

  if (text.empty()) return;

  for (char c : text) {
    Insert(c, m_cursor.Position);
  }
}

void UI::ECS::TextBox::DeleteSelection() {
  for (int i = 0; i < m_textSelection.length; i++) {
    Delete(m_textSelection.start);
  }

  m_cursor.Position = m_textSelection.start;

  CancelSelection();
}

void UI::ECS::TextBox::LoadFile() {
  m_table = PieceTable(IO::ReadTextFile(m_filepath));
}

void UI::ECS::TextBox::SaveFile() {
  IO::WriteTextFile(m_filepath, GetContent());

  LoadFile();
}

Colour UI::ECS::TextBox::GetCursorColour() const {
  return m_cursor._Colour;
}

void UI::ECS::TextBox::SetCursorColour(const Colour& NewColour) {
  m_cursor._Colour = NewColour;
}

UI::TextCursorType UI::ECS::TextBox::GetCursorType() const {
  return m_cursor.Type;
}

void UI::ECS::TextBox::SetCursorType(const UI::TextCursorType & NewType) {
  m_cursor.Type = NewType;
}

std::string UI::ECS::TextBox::GetContent() {
  return m_table.GetContent();
}

std::string UI::ECS::TextBox::GetContent() const {
  return m_table.GetContent();
}

#ifdef _DEBUG
  void UI::ECS::TextBox::Print() { m_table.Print(); }

  void UI::ECS::TextBox::DebugPrint() { m_table.DebugPrint(); }
#endif // _DEBUG

} // namespace CustomIDE

