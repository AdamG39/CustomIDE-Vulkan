#ifndef CUSTOM_UI_H
#define CUSTOM_UI_H

#include <vector>
#include <algorithm>
#include "../renderer/shapes.hpp"

enum class SizeMode { Fixed, Proportional };

template <typename T>
struct UISize {
  T Value{};
  SizeMode Mode{SizeMode::Fixed};

  UISize() = default;

  UISize(T Value, SizeMode Mode = SizeMode::Fixed)
  : Value(Value), Mode(Mode) {}

  template<typename U>
  UISize(const UISize<U>& That)
  : Value(static_cast<T>(That.Value)), Mode(That.Mode) {}
};

enum class UIAnchorType {
  Center,
  Top,
  Bottom,
  Left,
  Right,
  TopLeft,
  TopRight,
  BottomLeft,
  BottomRight
};

template <typename T, typename C>
std::vector<Vertex<T, C>> TriVectorToSortedVertexVector(std::vector<Triangle<T, C>> Tris) {
  std::vector<Vertex<T, C>> vertices;

  std::sort(Tris.begin(), Tris.end());

  for (size_t i = 0; i < Tris.size(); i++) {
    vertices.push_back(Tris[i].vertices[0]);
    vertices.push_back(Tris[i].vertices[1]);
    vertices.push_back(Tris[i].vertices[2]);
  }

  return vertices;
}

template <typename T1, typename C1, typename T2, typename C2>
std::vector<Vertex<T2, C2>> ConvertVertexVector(const std::vector<Vertex<T1, C1>>& Source) {
  if constexpr (std::is_same_v<T1, T2> && std::is_same_v<C1, C2>) {
    // No conversion needed
    return Source;
  }
  std::vector<Vertex<T2, C2>> result;
  result.reserve(Source.size());
  for (const auto& v: Source) {
    result.emplace_back(v);
  }

  return result;
}

#endif

