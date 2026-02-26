#ifndef CUSTOM_UI_H
#define CUSTOM_UI_H

#include <vector>
#include <list>
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
std::vector<Vertex<T, C>> RectVectorToSortedVertexVector(std::list<Rect<T, C>> Rects,
    std::list<DrawBatch>& drawBatches, Vector2<uint32_t> framebufferSize) {
  std::vector<Vertex<T, C>> vertices;

  // Sort by zIndex
  Rects.sort();
  Rects.sort([](auto first, auto second){
    return (first.GetZIndex() < second.GetZIndex());
  });
  // sort Rects within the same zIndex in textureIndex order -N -> +M
  Rects.sort([](auto first, auto second){
    return (first.GetTextureIndex() < second.GetTextureIndex()) && first.GetZIndex() == second.GetZIndex();
  });

  // create a list of all array bounds
  // (a bound encapsulates Rects with the same zIndex and textureIndex)
  int counter = 0;
  uint32_t start = 0;
  uint32_t count = 0;
  int currentIndex = (*(Rects.begin())).GetTextureIndex();
  for (Rect<T, C>& rect : Rects) {
    if (rect.GetTextureIndex() != currentIndex) {
      count *= 6;

      DrawBatch batch{
        .textureIndex = currentIndex,
        .vertexOffset = start,
        .vertexCount = count,
        .clipRect = {
          .xOffset = 0,
          .yOffset = 0,
          .width = framebufferSize.x,
          .height = framebufferSize.y
        }
      };
      drawBatches.push_back(batch);
      start = counter * 6;
      count = 0;
      currentIndex = rect.GetTextureIndex();
    }

    counter++;
    count++;

    auto Tris = rect.GetTris();

    // push the vertices to the vertex array
    vertices.push_back(Tris[0].vertices[0]);
    vertices.push_back(Tris[0].vertices[1]);
    vertices.push_back(Tris[0].vertices[2]);
    vertices.push_back(Tris[1].vertices[0]);
    vertices.push_back(Tris[1].vertices[1]);
    vertices.push_back(Tris[1].vertices[2]);
  }

  // Add final bound
  count *= 6;
  DrawBatch batch{
    .textureIndex = currentIndex,
    .vertexOffset = start,
    .vertexCount = count,
    .clipRect = {
      .xOffset = 0,
      .yOffset = 0,
      .width = framebufferSize.x,
      .height = framebufferSize.y
    }
  };
  drawBatches.push_back(batch);

  return vertices;
}

#endif

