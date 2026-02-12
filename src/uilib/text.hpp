#ifndef CUSTOM_TEXT_H
#define CUSTOM_TEXT_H

#include <string>
#include <list>
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

class IText {
private:
  Font font;
  bool m_wordWrap;

public:
  IText(Font Font, bool WordWrap = false) : font(Font), m_wordWrap(WordWrap) {}

  Font GetFont() const {
    return font;
  }

  void SetWordWrap(bool Value) { m_wordWrap = Value; }
  bool GetWordWrap() { return m_wordWrap; }

  virtual std::string GetContent() const = 0;

  std::array<Vector2<float>, 4> CalculateCharTextureCoords(Vector2<float> FontAtlasSize, char Character);
};

struct TextCursor {
  int Position;
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
  const std::string m_original;
  std::string m_add;

  std::list<PieceTableEntry> m_entries;

public:
  PieceTable(std::string FileContents) : m_original(std::move(FileContents)) {
    PieceTableEntry initialEntry {
      .Type = Original,
      .Start = 0,
      .Length = m_original.size()
    };

    m_entries.push_back(initialEntry);
  }

  char Index(unsigned Position) const;

  void Insert(char Character, int Position);

  void Delete(int Position);

  std::string GetContent() const;

#ifdef _DEBUG
  void Print();

  void DebugPrint();
#endif // _DEBUG
};

#endif

