#ifndef CUSTOM_TEXT_H
#define CUSTOM_TEXT_H

#include <string>
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

class IText {
  Font font;

public:
  IText(Font Font) : font(Font) {}

  Font GetFont() const {
    return font;
  }

  virtual std::string GetContent() const = 0;

  std::array<Vector2<float>, 4> CalculateCharTextureCoords(Vector2<float> FontAtlasSize, char Character);
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
  std::string m_original;
  std::string m_add;

  std::vector<PieceTableEntry> m_entries;

public:
  PieceTable(std::string FileContents) : m_original(std::move(FileContents)) {
    PieceTableEntry initialEntry {
      .Type = Original,
      .Start = 0,
      .Length = m_original.size()
    };

    m_entries.push_back(initialEntry);
  }

  char Index(unsigned Position);

  void Insert(char Character, int Position);

  void Delete(int Position);

  std::string GetContent() const;

  void Print();

  void DebugPrint();
};

#endif

