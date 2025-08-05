#pragma once

#include <array>
#include <string>
#include <vector>
#include <memory>
#include <algorithm>
#include <type_traits>
#include <functional>
#include <utility>
#include "errorHandler.h"

enum class UIType { Base, Panel, Button };

template <typename T>
struct Vector2 {
  T x, y;

  Vector2() : x(0), y(0) {}

  template<typename U>
  Vector2(const Vector2<U>& That)
  : x(static_cast<T>(That.x)), y(static_cast<T>(That.y)) {}

  Vector2(T X, T Y) : x(X), y(Y) {}
};

template <typename T>
struct Vector3 {
  T x, y, z;

  Vector3(): x(0), y(0), z(0) {}

  Vector3(const Vector2<T>& that) : x(that.x), y(that.y), z(0) {}

  template<typename U, typename = std::enable_if_t<!std::is_same_v<T, U>>>
  Vector3(const Vector3<U>& That)
  : x(static_cast<T>(That.x)), y(static_cast<T>(That.y)), z(static_cast<T>(That.z)) {}

  Vector3(T X, T Y, T Z) : x(X), y(Y), z(Z) {}
};

template <typename T>
struct Colour {
  T r, g, b, a;

  Colour() : r(0), g(0), b(0), a(1) {}

  Colour(int HexColour, T Alpha)
  : r(static_cast<T>(static_cast<float>((HexColour & 0xFF0000) >> 0x10) / 255.0f)),
    g(static_cast<T>(static_cast<float>((HexColour & 0x00FF00) >> 0x8) / 255.0f)),
    b(static_cast<T>(static_cast<float>(HexColour & 0x0000FF) / 255.0f)), a(Alpha) {}

  template<typename U, typename = std::enable_if_t<!std::is_same_v<T, U>>>
  Colour(const Colour<U>& That)
  : r(static_cast<T>(That.r)),
    g(static_cast<T>(That.g)),
    b(static_cast<T>(That.b)),
    a(static_cast<T>(That.a)) {}

  Colour(T R, T G, T B, T A) : r(R), g(G), b(B), a(A) {}
};

template <typename T, typename C>
struct Vertex {
  Vector3<T> position;
  Colour<C> colour;

  Vertex() : position(), colour() {}
  
  Vertex(const Vector2<T>& Position, const Colour<C>& Colour)
  : position(Vector3(Position)), colour(Colour) {}

  Vertex(const Vector3<T>& Position, const Colour<C>& Colour)
  : position(Position), colour(Colour) {}

  template <typename Ut, typename Uc, typename = std::enable_if_t<!std::is_same_v<T, Ut> || !std::is_same_v<C, Uc>>>
  Vertex(const Vertex<Ut, Uc>& That)
  : position(That.position), colour(That.colour) {}
};

template <typename T, typename C>
struct Triangle {
  std::array<Vertex<T, C>, 3> vertices;
  int zIndex;

  Triangle() : vertices(), zIndex(0) {}

  Triangle(std::array<Vertex<T, C>, 3> Vertices, int Z_Index)
  : vertices(Vertices), zIndex(Z_Index) {}

  Triangle(const Vertex<T, C>& V0, const Vertex<T, C>& V1, const Vertex<T, C>& V2, int Z_Index)
  : vertices{ V0, V1, V2 }, zIndex(Z_Index) {}

  bool operator < (const Triangle<T, C>& That) {
    return (zIndex < That.zIndex);
  }
};

template <typename T, typename C>
class Rect {
private:
  Vector2<T> m_position;
  Vector2<T> m_size;
  Colour<C> m_colour;
  int m_zIndex;

  std::array<Vertex<T, C>, 4> m_vertices;

  void AdjustVertices() {
    T xDiff = m_size.x / 2;
    T yDiff = m_size.y / 2;
    T xPos = m_position.x;
    T yPos = m_position.y;

    m_vertices[0].position = Vector2(xPos - xDiff, yPos - yDiff); // Top Left
    m_vertices[1].position = Vector2(xPos + xDiff, yPos - yDiff); // Top Right
    m_vertices[2].position = Vector2(xPos - xDiff, yPos + yDiff); // Bottom Left
    m_vertices[3].position = Vector2(xPos + xDiff, yPos + yDiff); // Bottom Right
  }

public:
  Rect(Vector3<T> TopLeft, Vector3<T> BotRight, Colour<C> Col = Colour<C>())
  : m_position(Vector2<T>((BotRight.x - TopLeft.x) / 2, (BotRight.y - TopLeft.y) / 2)),
    m_size(Vector2<T>(BotRight.x - TopLeft.x, BotRight.y - TopLeft.y)),
    m_colour(Col),
    m_zIndex(0),
    m_vertices{ Vertex<T, C>(TopLeft, Col),
                Vertex<T, C>(BotRight.x, TopLeft.y, Col), 
                Vertex<T, C>(TopLeft.x, BotRight.y, Col), 
                Vertex<T, C>(BotRight, Col) } {}

  Rect(Vector2<T> Size, Vector2<T> Position, Colour<C> Col = Colour<C>())
  : m_position(Position),
    m_size(Size),
    m_colour(Col),
    m_zIndex(0),
    m_vertices{ Vertex<T, C>(Vector2<T>(Position.x - (Size.x / 2), Position.y - (Size.y / 2)), Col),
                Vertex<T, C>(Vector2<T>(Position.x + (Size.x / 2), Position.y - (Size.y / 2)), Col),
                Vertex<T, C>(Vector2<T>(Position.x - (Size.x / 2), Position.y + (Size.y / 2)), Col),
                Vertex<T, C>(Vector2<T>(Position.x + (Size.x / 2), Position.y + (Size.y / 2)), Col) } {}

  std::array<Vertex<T, C>, 6> GetVertices() const {
    std::array<Vertex<T, C>, 6> vertices;

    // Triangle 1
    vertices[0] = m_vertices[0];
    vertices[1] = m_vertices[1];
    vertices[2] = m_vertices[2];

    // Triangle 2
    vertices[3] = m_vertices[1];
    vertices[4] = m_vertices[2];
    vertices[5] = m_vertices[3];

    return vertices;
  }

  std::array<Triangle<T, C>, 2> GetTris() const {
    std::array<Triangle<T, C>, 2> tris;

    tris[0] = Triangle<T, C>(m_vertices[0], m_vertices[1], m_vertices[2], m_zIndex);
    tris[1] = Triangle<T, C>(m_vertices[1], m_vertices[2], m_vertices[3], m_zIndex);

    return tris;
  }

  Vector2<T> GetSize() const { return m_size; }

  void SetSize(const Vector2<T> NewSize) {
    m_size = NewSize;

    AdjustVertices();
  }

  Vector2<T> GetPosition() const { return m_position; }

  void SetPosition(const Vector2<T> NewPosition) {
    m_position = NewPosition;

    AdjustVertices();
  }

  Colour<C> GetColour() const { return m_colour; }

  void SetColour(Colour<C> NewColour) {
    m_colour = NewColour;

    for (Vertex<T, C>& vertex: m_vertices) {
      vertex.colour = NewColour;
    }
  }

  int GetZIndex() const { return m_zIndex; }

  void SetZIndex(const int Value) { m_zIndex = Value; }
};

template <typename T, typename C>
class UIElement {
protected:
  UIType m_type = UIType::Base;
  bool m_visible = false;
  Rect<T, C> m_geometry;

  std::vector<std::shared_ptr<UIElement<T, C>>> m_childObjects;
  UIElement<T, C>* m_parent = nullptr;

public:
  UIElement(Vector2<T> Size, Vector2<T> Position)
  : m_geometry(Rect<T, C>(Size, Position)) {}

  virtual ~UIElement() {
    delete m_parent;
  }

  T GetWidth() const { return m_geometry.GetSize().x; }

  T GetHeight() const { return m_geometry.GetSize().y; }

  Vector2<T> GetSize() const { return m_geometry.GetSize(); }

  void SetSize(const Vector2<T> NewSize) { m_geometry.SetSize(NewSize); }

  Vector2<T> GetPositon() const { return m_geometry.GetPosition(); }

  void SetPosition(const Vector2<T> NewPosition) { m_geometry.SetPosition(NewPosition); }

  bool IsVisible() const { return m_visible; }

  void SetVisibility(const bool Value) { m_visible = Value; }

  Rect<T, C> GetGeometry() const { return m_geometry; }

  void AddChild(std::shared_ptr<UIElement<T, C>> Element) {
    m_childObjects.push_back(Element);
  }

  UIElement<T, C>* GetChildByIndex(const int Index) const {
    if (Index > m_childObjects.size()) {
      ExitWithError("Index out of range", -2);
    }

    return m_childObjects[Index];
  }

  std::vector<std::shared_ptr<UIElement<T, C>>> GetChildren() const { return m_childObjects; }

  size_t GetChildCount() const { return m_childObjects.size(); }

  std::vector<Rect<T, C>> GetTotalGeometries() const {
    std::vector<Rect<T, C>> geometries;

    geometries.push_back(GetGeometry());

    for (size_t i = 0; i < m_childObjects.size(); i++) {
      if (!(m_childObjects[i]->IsVisible())) continue;
      std::vector<Rect<T, C>> temp = m_childObjects[i]->GetTotalGeometries();

      for (size_t j = 0; j < temp.size(); i++) {
        geometries.push_back(temp[j]);
      }
    }

    return geometries;
  }

  UIType GetType() const { return m_type; }
};

template <typename T, typename C>
class Panel : public UIElement<T, C> {
public:
  std::string ID;

  Panel(std::string ID, Vector2<T> Size, Vector2<T> Position, Colour<C> Colour)
  : UIElement<T, C>(Size, Position),
    ID(ID)
    { SetColour(Colour); UIElement<T, C>::SetVisibility(true); UIElement<T, C>::m_type = UIType::Panel; }

  Colour<C> GetColour() const { return UIElement<T, C>::m_geometry.GetColour(); }

  void SetColour(const Colour<C> NewColour) { UIElement<T, C>::m_geometry.SetColour(NewColour); }
};

template <typename T, typename C>
class Button : public UIElement<T, C> {
public:
  template <typename Func, typename... Args>
  Button(Vector2<T> Size, Vector2<T> Position, Func&& Function, Args&&... Arguments)
  : UIElement<T, C>(Size, Position)
    { 
      UIElement<T, C>::m_type = UIType::Button; 
      
      m_onClick = std::bind(std::forward<Func>(Function), std::forward<Args>(Arguments)...);
    }

  void OnClick() {
    if (m_onClick) {
      m_onClick();
    }
  }

private:
  std::function<void()> m_onClick;
};

template <typename T, typename C>
class RenderableButton: public Button<T, C> {
private:
  Panel<T, C> m_panel;

public:
  using Callback = std::function<void()>;

  RenderableButton(std::string ID, Vector2<T> Size, Vector2<T> Position, Colour<C> Colour, Callback OnClick)
  : Button<T, C>(Size, Position, OnClick),
    m_panel(Panel<T, C>(ID, Size, Position, Colour)){}
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

