#ifndef CUSTOM_TEXT_H
#define CUSTOM_TEXT_H

#include <string>
#include <list>
#include <vector>
#include "../renderer/shapes.hpp"

namespace CustomIDE::UI {

enum TextType {
  Normal,
  Bold,
  Italic
};

struct Font {
  Vector2<int> size;
  std::string familyName;
  Colour colour;
  TextType type = Normal;
};

enum class TextCursorType {
  DEFAULT,
  BLOCK,
};

struct TextCursor {
  int Position;
  Colour _Colour;
  TextCursorType Type;
};

struct TextSelection {
  size_t start;
  size_t length;
};

enum PieceTableBufferType {
  Original,
  Add
};

struct PieceTableEntry {
  PieceTableBufferType Type;
  size_t Start;
  size_t Length;
};

class PieceTable {
private:
  static constexpr int    TAB_CHAR_LITERAL    = '\t';  
  static constexpr int    SPACE_CHAR_LITERAL  = ' ';  

  static constexpr size_t DEFAULT_CHAR_WIDTH  = 1;
  static constexpr size_t TAB_SPACE_WIDTH     = 4; // FIXME: Should be set using a configuration file instead
  static constexpr bool   TABS_ARE_SPACES     = false; // FIXME: Should be set using a configuration file instead

  bool m_recalculateContent;
  std::string m_content;

  std::string m_original;
  std::string m_add;

  std::list<PieceTableEntry> m_entries;
  bool m_recalculateStartOfLines;
  std::vector<int> m_startOfLines;

public:
  PieceTable() = default;
  PieceTable(std::string FileContents) : m_original(std::move(FileContents)) {
    PieceTableEntry initialEntry {
      .Type = Original,
      .Start = 0,
      .Length = m_original.size()
    };

    m_entries.push_back(initialEntry);

    m_recalculateContent = true;
    m_recalculateStartOfLines = true;
  }

  char Index(unsigned Position) const;

  size_t Insert(char Character, unsigned Position);

  void Delete(unsigned Position);

  std::string GetContent();
  std::string GetContent() const;

  const std::vector<int>& GetStartOfLines();

#ifdef _DEBUG
  void Print();

  void DebugPrint();
#endif // _DEBUG
};

UVRect2D CalculateCharUV(Vector2D FontAtlasSize, char Character);

} // namespace UI

#endif

