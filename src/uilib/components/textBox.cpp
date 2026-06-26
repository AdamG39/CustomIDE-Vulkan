#include "textBox.hpp"
#include "transform.hpp"
#include "../ecs.hpp"
#include "../../io/io.hpp"

int TextBox::GetType() { return TypeValue(); }

TextBox::TextBox(Font Font, std::string Filepath, bool WordWrap)
  : m_filepath(Filepath) {
  LoadFile();
  m_font = Font;
  m_wordWrap = WordWrap;
  m_cursor._Colour = Font.colour;
}

void TextBox::Render(EntityManager& Manager, const Transform* Transform) {
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

    Colour textColour = font.colour;
    if (cursorIndexPosition == i) {
      textColour.r = 1.f - textColour.r;
      textColour.g = 1.f - textColour.g;
      textColour.b = 1.f - textColour.b;
    }
    auto imageIndex = Manager.GetRenderer().lock()->GetImageIndexFromName(font.familyName);
    if (imageIndex < 0) ExitWithError("No image with that name found", -35);

    ClipRect clipRect {.clippingEnabled = false};
    if (!Manager.GetClipStack().empty())
      clipRect = Manager.GetClipStack().top();

    Manager.GetRenderer().lock()->DrawTexturedRectEx(
        Rect2D{ static_cast<int32_t>(charPosition.x), static_cast<int32_t>(charPosition.y),
                static_cast<uint32_t>(font.size.x), static_cast<uint32_t>(font.size.y) },
        CalculateCharUV(fontAtlasSize, content[i]), m_drawDepth + 1, imageIndex, clipRect,
        textColour);
  }
  if (GetSelectionState()) {
    int start = m_textSelection.start;
    int length = m_textSelection.length;

    // find which line the selection starts on
    int firstLineStart = -1;
    for (int i = 0; i < m_table.GetStartOfLines().size(); i++) {
      if (m_table.GetStartOfLines()[i] > start) {
        firstLineStart = i - 1;
        break;
      }
    }

    if (firstLineStart < 0) firstLineStart = 0;

    int startOffsetIntoLine = start - m_table.GetStartOfLines()[firstLineStart];

    std::vector<Rect2D> selectionRects;

    int currentLine = firstLineStart;
    for (int i = start; i < (start + length); i++) {
      if (selectionRects.empty()) {
        Rect2D first {
          .xOffset = font.size.x * startOffsetIntoLine,
          .yOffset = font.size.y * firstLineStart,
          .width = static_cast<uint32_t>(font.size.x),
          .height = static_cast<uint32_t>(font.size.y)
        };

        selectionRects.push_back(first);
      }
      if (m_table.GetContent()[i] != '\n') {
        auto rectIt = selectionRects.rbegin();
        // Adjust the width and xOffset to include the next element
        rectIt->xOffset += font.size.x / 2;
        rectIt->width += static_cast<uint32_t>(font.size.x);
      }
      else {
        currentLine++;
        Rect2D newSelectionLine {
          .xOffset = 0, // Already at start of line so simplify calculation
          .yOffset = font.size.y * currentLine,
          .width = static_cast<uint32_t>(font.size.x),
          .height = static_cast<uint32_t>(font.size.y)
        };

        selectionRects.push_back(newSelectionLine);
      }
    }

    for (auto rect : selectionRects) {
      rect.xOffset += textObjPos.x;
      rect.yOffset += textObjPos.y;
      Manager.GetRenderer().lock()->DrawRect(
          rect,
          m_drawDepth, Colour(0x90D5FF, 0.5f));
    }
  }

  if (cursorIndexPosition == content.size())
    cursorPosition = { linePosition, lineCount };

  Vector2<float> finalCursorSize {
    static_cast<float>(font.size.x),
    static_cast<float>(font.size.y) * 1.5f
  };

  Vector2<float> finalCursorPosition {
    textObjPos.x + (font.size.x * cursorPosition.x),
    textObjPos.y + (font.size.y * cursorPosition.y)
  };

  Manager.GetRenderer().lock()->DrawRect(
      Rect2D{ static_cast<int32_t>(finalCursorPosition.x), static_cast<int32_t>(finalCursorPosition.y),
              static_cast<uint32_t>(finalCursorSize.x), static_cast<uint32_t>(finalCursorSize.y) },
      m_drawDepth, GetCursorColour());
}

char TextBox::Index(unsigned Position) {
  return m_table.Index(Position);
}

void TextBox::Insert(char Character, int Position) {
  CancelSelection();
  m_table.Insert(Character, Position);
  MoveCursorRight();
}

void TextBox::Delete(int Position) {
  m_table.Delete(Position);
}

int TextBox::GetCursorPosition() const {
  return m_cursor.Position;
}

void TextBox::MoveCursorLeft() {
  m_cursor.Position = std::max(0, m_cursor.Position - 1);
}

void TextBox::MoveCursorRight() {
  m_cursor.Position = std::min(static_cast<size_t>(m_cursor.Position + 1), GetContent().size());
}

#include <cassert>

void TextBox::MoveCursorUp() {
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

void TextBox::MoveCursorDown() {
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

void TextBox::MoveBackWord() {
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

void TextBox::MoveForwardWord() {
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

bool TextBox::GetSelectionState() const {
  return m_selectionState;
}

void TextBox::StartSelection() {
  m_textSelection = {
    .start = static_cast<size_t>(m_cursor.Position),
    .length = 0ull
  };

  m_selectionState = true;
  m_selectionDirection = None;
}

void TextBox::UpdateSelection(int PreviousPosition) {
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
      m_textSelection.start = m_cursor.Position;
      m_textSelection.length += PreviousPosition - m_cursor.Position;
      break;
    case Right:
      if (m_cursor.Position < m_textSelection.start) m_selectionDirection = Left;
      m_textSelection.length += m_cursor.Position - PreviousPosition;
      break;
  }
  if (m_textSelection.length == 0) CancelSelection();
}

void TextBox::CancelSelection() {
  m_selectionState = false;
  m_selectionDirection = None;
}

void TextBox::CopySelection(GLFWwindow* Window) {
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

void TextBox::PasteText(GLFWwindow* Window) {
  std::string text = glfwGetClipboardString(Window);

  if (text.empty()) return;

  for (char c : text) {
    Insert(c, m_cursor.Position);
  }
}

void TextBox::DeleteSelection() {
  for (int i = 0; i < m_textSelection.length; i++) {
    Delete(m_textSelection.start);
  }

  m_cursor.Position = m_textSelection.start;

  CancelSelection();
}

void TextBox::LoadFile() {
  m_table = PieceTable(ReadTextFile(m_filepath));
}

void TextBox::SaveFile() {
  WriteTextFile(m_filepath, GetContent());

  LoadFile();
}

Colour TextBox::GetCursorColour() const {
  return m_cursor._Colour;
}

void TextBox::SetCursorColour(const Colour& NewColour) {
  m_cursor._Colour = NewColour;
}

std::string TextBox::GetContent() {
  return m_table.GetContent();
}

std::string TextBox::GetContent() const {
  return m_table.GetContent();
}

#ifdef _DEBUG
  void TextBox::Print() { m_table.Print(); }

  void TextBox::DebugPrint() { m_table.DebugPrint(); }
#endif // _DEBUG

