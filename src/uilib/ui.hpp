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
std::vector<Vertex<T, C>> TriVectorToSortedVertexVector(std::vector<Triangle<T, C>> Tris,
    std::vector<TextureArrayBounds>& arrayBounds) {
  std::vector<Vertex<T, C>> vertices;

  std::sort(Tris.begin(), Tris.end());
  // sort tris within the same zIndex in textureIndex order -N -> +M
  std::sort(Tris.begin(), Tris.end(), [](auto first, auto second){
    return (first.textureIndex < second.textureIndex) && first.zIndex == second.zIndex;
  });

  // create a list of all array bounds
  // (a bound encapsulates tris with the same zIndex and textureIndex)
  for (size_t i = 0; i < Tris.size(); i++) {
    size_t start = i * 3;
    size_t count = 3;

    for (size_t j = i + 1; j < Tris.size(); j++) {
      if (Tris[i].textureIndex != Tris[j].textureIndex) {
        ArrayBounds bounds { start, count };
        arrayBounds.push_back({ bounds, Tris[i].textureIndex });
        i = j - 1;
        break;
      }

      count += 3;
      if (j == (Tris.size() - 1)) {
        ArrayBounds bounds { start, count };
        arrayBounds.push_back({ bounds, Tris[i].textureIndex });
      }
    }
  }

  for (size_t i = 0; i < Tris.size(); i++) {
    vertices.push_back(Tris[i].vertices[0]);
    vertices.push_back(Tris[i].vertices[1]);
    vertices.push_back(Tris[i].vertices[2]);
  }

  return vertices;
}

#endif

