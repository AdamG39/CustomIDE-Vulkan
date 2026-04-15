#include "../helpers/errors/errors.hpp"
#include "text.hpp"

UVRect2D CalculateCharUV(Vector2<float> FontAtlasSize, char Character) {
  Vector2 characterAtlasSize { 1.f / FontAtlasSize.x, 1.f / FontAtlasSize.y };

  return {
    (characterAtlasSize.x / 2) + characterAtlasSize.x * (Character % static_cast<int>(FontAtlasSize.x)),
    characterAtlasSize.y * int(Character / FontAtlasSize.x) + (characterAtlasSize.y / 2),
    characterAtlasSize.x,
    characterAtlasSize.y
  };
}

char PieceTable::Index(unsigned Position) const {
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

  m_recalculateContent = true;
  m_recalculateNewLines = true;

  // Need to find which entry to add to
  unsigned counter = 0;
  auto it = m_entries.begin();
  for (; it != m_entries.end(); it++) {
    auto entry = *it;
    if (Position >= counter && Position <= (counter + (entry.Length - 1))) {
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
  if (it == m_entries.end()) {
    m_entries.push_back(newEntry);
    return;
  }

  PieceTableEntry& entry = *it;

  // Found the entry now split it
  if (counter == Position) { // Character to be inserted at start of this entry
    // Create a new entry and add before the current one
    m_entries.insert(it, newEntry);
    return;
  }

  else { // Character to be inserted in the middle of this entry
    // Split current entry to 2 parts and add the new one between
    int relativePosition = Position - counter;

    size_t secondPartLength = entry.Length - relativePosition;
    entry.Length = relativePosition;

    PieceTableEntry secondPart {
      .Type = entry.Type,
      .Start = entry.Length + entry.Start,
      .Length = secondPartLength
    };

    if (++it == m_entries.end()) {
      m_entries.push_back(newEntry);
      m_entries.push_back(secondPart);
    } else {
      m_entries.insert(it, secondPart);
      m_entries.insert(it, newEntry);
    }
  } 
}

void PieceTable::Delete(int Position) {
  m_recalculateContent = true;
  m_recalculateNewLines = true;

  unsigned counter = 0;
  long long entryIndex = -1;
  auto it = m_entries.begin();
  for (size_t i = 0; i < m_entries.size(); i++, it++) {
    auto entry = *it;
    if (Position >= counter && Position <= (counter + (entry.Length - 1))) {
      entryIndex = i;
      break;
    }

    counter += entry.Length;
  }

  // Character is out of bounds so just ignore call
  if (entryIndex < 0) return;

  PieceTableEntry& entry = *it;

  // Found the entry now split it
  if (counter == Position) { // Character to be deleted at start of this entry
    entry.Start++;
    entry.Length--;
  }

  else if (counter + (entry.Length - 1) == Position) { // Character to be deleted at end of this entry
    entry.Length--;
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
    else {
      it++;
      m_entries.insert(it, secondPart);
    }
  }

  // If this entry is now empty remove it
  if (entry.Length == 0) m_entries.erase(it);
}

std::string PieceTable::GetContent() {
  // Recalculates content if contents state changed otherwise returns cached result
  if (!m_recalculateContent) return m_content;

  // Recalculate content
  m_content.clear();

  for (auto entry : m_entries) {
    if (entry.Type == PieceTableBufferType::Original) {
      m_content.append(m_original.substr(entry.Start, entry.Length));
    } else {
      m_content.append(m_add.substr(entry.Start, entry.Length));
    }
  }

  m_recalculateContent = false;

  return m_content;
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

const std::vector<int>& PieceTable::GetNewLines() {
  // Recalculates new line positions if content state changed otherwise returns cached result
  if (!m_recalculateNewLines) return m_newLines;

  m_newLines.clear();
  // Recalculate newlines
  int counter = 0;
  for (char c : GetContent()) {
    if (c == '\n')
      m_newLines.push_back(counter);

    counter++;
  }
  m_recalculateNewLines = false;

  return m_newLines;
}

#ifdef _DEBUG
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

#endif // _DEBUG

