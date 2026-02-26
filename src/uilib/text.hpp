#ifndef CUSTOM_TEXT_H
#define CUSTOM_TEXT_H

#include <string>
#include <list>
#include <vector>
#include "../renderer/shapes.hpp"


enum TextType {
  Normal,
  Bold,
  Italic
};

struct Font {
  Vector2<int> size;
  std::string familyName;
  Colour<float> colour;
  TextType type = Normal;
};

struct TextCursor {
  int Position;
  Colour<float> Colour;
};

struct TextSelection {
  size_t start;
  size_t length;
};

enum TextCursorMoveDirection : int {
  Up,
  Down,
  Left,
  Right
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
  bool m_recalculateContent;
  std::string m_content;

  std::string m_original;
  std::string m_add;

  std::list<PieceTableEntry> m_entries;
  bool m_recalculateNewLines;
  std::vector<int> m_newLines;

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
    m_recalculateNewLines = true;
  }

  char Index(unsigned Position) const;

  void Insert(char Character, int Position);

  void Delete(int Position);

  std::string GetContent();
  std::string GetContent() const;

  const std::vector<int>& GetNewLines();

#ifdef _DEBUG
  void Print();

  void DebugPrint();
#endif // _DEBUG
};

std::array<Vector2<float>, 4> CalculateCharTextureCoords(Vector2<float> FontAtlasSize, char Character);

#endif

