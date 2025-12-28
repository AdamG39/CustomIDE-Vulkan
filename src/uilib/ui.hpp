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

#endif

