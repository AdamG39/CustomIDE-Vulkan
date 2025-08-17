#ifndef CUSTOM_LAYOUT_PARSER_H
#define CUSTOM_LAYOUT_PARSER_H

#include <string>
#include "app.hpp"

template <typename K, typename V>
struct KeyValuePair {
  K Key;
  V Value;
};

class Parser {
public:
  Parser(std::string FilePath, const UIManager<int, float>& Manager)
  : m_filePath(FilePath), m_manager(&Manager) {}

private:
  std::string m_filePath;
  const UIManager<int, float>* m_manager;

  void ParseElement();

  void ReadWord();

  std::string Peek(size_t Distance);

  template <typename K, typename V>
  KeyValuePair<K, V> ParseAttribute() {
    
  }
};

#endif

