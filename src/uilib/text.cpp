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

  if (entryIndex < 0) ExitWithError("Character to delete is out of bounds of PieceTable", -46);

  PieceTableEntry& entry = m_entries[entryIndex];

  // Found the entry now split it
  if (counter == Position) { // Character to be deleted at start of this entry
    entry.Start++;
    entry.Length--;
    // If this entry is now empty remove it
    if (entry.Length == 0) m_entries.erase(m_entries.begin() + entryIndex);
  }

  else if (counter + (entry.Length - 1) == Position) { // Character to be deleted at end of this entry
    entry.Length--;
    // If this entry is now empty remove it
    if (entry.Length == 0) m_entries.erase(m_entries.begin() + entryIndex);
  }

  else { // Character to be deleted in the midle of this entry
    int relativePosition = Position - counter;

    size_t previousLength = entry.Length;
    entry.Length = relativePosition;

    PieceTableEntry secondPart {
      .Type = entry.Type,
      .Start = entry.Start + entry.Length + 1,
      .Length = previousLength - (entry.Length + 1)
    };

    if (entryIndex + 1 == m_entries.size())
      m_entries.push_back(secondPart);
    else
      m_entries.insert(m_entries.begin() + entryIndex + 1, secondPart);
  }
}

std::string PieceTable::GetContent() const {
  std::string result;

  for (auto entry : m_entries) {
    if (entry.Type == PieceTableBufferType::Original) {
      result.append(m_original.substr(entry.Start, entry.Length));
    } else {
      result.append(m_add.substr(entry.Start, entry.Length));
    }
  }

  return result;
}

#include <iostream>

void PieceTable::Print() {
  std::string result;

  for (auto entry : m_entries) {
    if (entry.Type == PieceTableBufferType::Original) {
      result.append(m_original.substr(entry.Start, entry.Length));
    } else {
      result.append(m_add.substr(entry.Start, entry.Length));
    }
  }

  std::cout << result << std::endl;
}

void PieceTable::DebugPrint() {
  std::cout << "PieceTable:" << std::endl;

  std::cout << "Original Buffer: { " << m_original << " }" << std::endl;
  std::cout << "Add Buffer: { " << m_add << " }\n" << std::endl;

  for (auto entry : m_entries) {
    std::cout << "(Entry): ";
    std::cout << "Type: " << ((entry.Type == PieceTableBufferType::Original) ? "Original" : "Add");
    std::cout << ", Start: " << entry.Start << ", Length: " << entry.Length << std::endl;
  }
}

