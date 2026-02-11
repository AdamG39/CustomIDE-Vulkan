#include "../helpers/errors/errors.hpp"
#include "text.hpp"

std::array<Vector2<float>, 4> IText::CalculateCharTextureCoords(Vector2<float> FontAtlasSize, char Character) {
  Vector2 textureCoordsMax { 1.f / FontAtlasSize.x, 1.f / FontAtlasSize.y };

  return {
    Vector2<float>(textureCoordsMax.x * (Character % static_cast<int>(FontAtlasSize.x)),
                   textureCoordsMax.y * int(Character / FontAtlasSize.x)),
    Vector2<float>((textureCoordsMax.x * (Character % static_cast<int>(FontAtlasSize.x))) + textureCoordsMax.x,
                   textureCoordsMax.y * int(Character / FontAtlasSize.x)),
    Vector2<float>(textureCoordsMax.x * (Character % static_cast<int>(FontAtlasSize.x)),
                   (textureCoordsMax.y * int(Character / FontAtlasSize.x)) +
                   textureCoordsMax.y),
    Vector2<float>((textureCoordsMax.x * (Character % static_cast<int>(FontAtlasSize.x))) + textureCoordsMax.x,
                   (textureCoordsMax.y * int(Character / FontAtlasSize.x)) +
                   textureCoordsMax.y)
  };
}

char PieceTable::Index(unsigned Position) {
  unsigned counter = 0;
  for (auto entry : m_entries) {
    if (Position >= counter && Position <= (counter + (entry.Length - 1))) {
      // Position is within this entry, find exact buffer index
      unsigned relativeIndex = Position - counter;
      
      return (entry.Type == PieceTableBufferType::Original)
        ? m_original[relativeIndex + entry.Start]
        : m_add[relativeIndex + entry.Start];
    }

    counter += entry.Length;
  }

  ExitWithError("Index out of bounds of PieceTable", -45);

  return NULL;
}

void PieceTable::Insert(char Character, int Position) {
  m_add.push_back(Character);

  // Need to find which entry to add to
  unsigned counter = 0;
  long long entryIndex = -1;
  for (size_t i = 0; i < m_entries.size(); i++) {
    auto entry = m_entries[i];
    if (Position >= counter && Position <= (counter + (entry.Length - 1))) {
      entryIndex = i;
      break;
    }

    counter += entry.Length;
  }

  PieceTableEntry newEntry {
    .Type = PieceTableBufferType::Add,
    .Start = m_add.size() - 1,
    .Length = 1
  };

  // If position is after all entries add it to the end
  if (entryIndex < 0) {
    m_entries.push_back(newEntry);
    return;
  }

  PieceTableEntry& entry = m_entries[entryIndex];

  // Found the entry now split it
  if (counter == Position) { // Character to be inserted at start of this entry
    // Create a new entry and add before the current one
    m_entries.insert(m_entries.begin() + entryIndex, newEntry);
    return;
  }

  else { // Character to be inserted in the middle of this entry
    // Split current entry to 2 parts and add the new one between
    int relativePosition = Position - counter;

    size_t secondPartLength = entry.Length - relativePosition;
    entry.Length -= relativePosition;

    PieceTableEntry secondPart {
      .Type = entry.Type,
      .Start = entry.Length,
      .Length = secondPartLength
    };

    if (entryIndex + 1 == m_entries.size()) {
      m_entries.push_back(newEntry);
      m_entries.push_back(secondPart);
    } else {
      m_entries.insert(m_entries.begin() + entryIndex + 1, secondPart);
      m_entries.insert(m_entries.begin() + entryIndex + 1, newEntry);
    }
  } 
}

void PieceTable::Delete(int Position) {

}

