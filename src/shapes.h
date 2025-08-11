#pragma once

#include <array>
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
  T x{};
  T y{};

  Vector2() = default;

  template <typename U1, typename U2,
            typename = std::enable_if_t<std::is_convertible_v<T, U1> &&
                                        std::is_convertible_v<T, U2>>>
  Vector2(U1&& X, U2&& Y)
  : x(std::forward<U1>(X)), y(std::forward<U2>(Y)) {}

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

enum class UIAnchorType { Center, Top, Bottom, Left, Right, TopLeft, TopRight, BottomLeft, BottomRight };

template <typename T>
struct UIAnchor {
  Vector2<UISize<T>> Offset;
  UIAnchorType Type = UIAnchorType::Center;

  UIAnchor() = default;

  UIAnchor(Vector2<UISize<T>> Offset, UIAnchorType Type = UIAnchorType::Center)
  : Offset(Offset), Type(Type) {}
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
  Vector2<UISize<T>> m_size;
  Vector2<UISize<T>> m_position;
  UIAnchor<T> m_anchor;

  UIType m_type = UIType::Base;
  bool m_visible = false;
  Rect<T, C> m_geometry;

  std::vector<std::shared_ptr<UIElement<T, C>>> m_childObjects;
  UIElement<T, C>* m_parent = nullptr;

public:
  UIElement(Vector2<UISize<T>> Size, Vector2<UISize<T>> Position)
  : m_geometry(Vector2(Size.x.Value, Size.y.Value), Vector2(Position.x.Value, Position.y.Value)),
    m_size(Size), m_position(Position) {}

  virtual ~UIElement() {
    delete m_parent;
  }

  T GetWidth() const { return m_geometry.GetSize().x; }

  T GetHeight() const { return m_geometry.GetSize().y; }

  Vector2<T> GetSize() const { return m_geometry.GetSize(); }

  virtual void SetSize(const Vector2<UISize<T>> NewSize) {
    m_geometry.SetSize(Vector2(NewSize.x.Value, NewSize.y.Value));
    m_size = NewSize;
  }

  Vector2<T> GetPosition() const { return m_geometry.GetPosition(); }

  virtual void SetPosition(const Vector2<UISize<T>> NewPosition) {
    m_geometry.SetPosition(Vector2(NewPosition.x.Value, NewPosition.y.Value));
    m_position = NewPosition;
  }

  virtual void SetAnchor(const UIAnchor<T> AnchorValue) {
    m_anchor = AnchorValue;
  }

  bool IsVisible() const { return m_visible; }

  void SetVisibility(const bool Value) { m_visible = Value; }

  Rect<T, C> GetGeometry() const { return m_geometry; }

  void AddChild(std::shared_ptr<UIElement<T, C>> Element) {
    m_childObjects.push_back(Element);
  }

  void RemoveChildren() {
    for (size_t i = 0; i < m_childObjects.size(); i++) {
      if (m_childObjects[i]->GetChildCount() > 0) { 
        m_childObjects[i]->RemoveChildren();
      }
    }
    m_childObjects.clear();
  }

  UIElement<T, C>* GetChildByIndex(const int Index) const {
    if (Index > m_childObjects.size()) {
      ExitWithError("Index out of range", -2);
    }

    return m_childObjects[Index];
  }

  virtual std::vector<std::shared_ptr<UIElement<T, C>>> GetChildren() const { return m_childObjects; }

  virtual size_t GetChildCount() const { return m_childObjects.size(); }

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

  void AdjustPositionRelativeToAnchor(Vector2<T>& Position, int framebufferWidth, int framebufferHeight) {
    switch (m_anchor.Type) {
      case UIAnchorType::Center:
        Position.x += (T)(framebufferWidth / 2);
        Position.y += (T)(framebufferHeight / 2);
        break;
      case UIAnchorType::Top:
        Position.x += (T)(framebufferWidth / 2);
        break;
      case UIAnchorType::Left:
        Position.y += (T)(framebufferHeight / 2);
        break;
      case UIAnchorType::Right:
        Position.x += (T)(framebufferWidth);
        Position.y += (T)(framebufferHeight / 2);
        break;
      case UIAnchorType::Bottom:
        Position.x += (T)(framebufferWidth / 2);
        Position.y += (T)(framebufferHeight);
        break;
      case UIAnchorType::TopLeft:         
        // Already relative to 0, 0 so no changes needed
        break;
      case UIAnchorType::TopRight:
        Position.x += (T)(framebufferWidth);
        break;
      case UIAnchorType::BottomLeft:
        Position.y += (T)(framebufferHeight);
        break;
      case UIAnchorType::BottomRight:
        Position.x += (T)(framebufferWidth);
        Position.y += (T)(framebufferHeight);
        break;
    }

    if (m_anchor.Offset.x.Mode == SizeMode::Proportional) {
      Position.x += (T)(framebufferWidth * m_anchor.Offset.x.Value);
    } else { Position.x += (T)(m_anchor.Offset.x.Value); }

    if (m_anchor.Offset.y.Mode == SizeMode::Proportional) {
      Position.y += (T)(framebufferHeight * m_anchor.Offset.y.Value);
    } else { Position.y += (T)(m_anchor.Offset.y.Value); }
  }

  virtual void RecalculateGeometry(int framebufferWidth, int framebufferHeight) {
    Vector2<T> newSize;
    Vector2<T> newPosition;
    if (m_size.x.Mode == SizeMode::Proportional) {
      newSize.x = (T)(framebufferWidth * m_size.x.Value);
    } else { newSize.x = m_size.x.Value; }

    if (m_size.y.Mode == SizeMode::Proportional) {
      newSize.y = (T)(framebufferHeight * m_size.y.Value);
    } else { newSize.y = m_size.y.Value; }

    AdjustPositionRelativeToAnchor(newPosition, framebufferWidth, framebufferHeight);

    if (m_position.x.Mode == SizeMode::Proportional) {
      newSize.x += (T)(framebufferWidth * m_size.x.Value);
    } else { newPosition.x += m_position.x.Value; }

    if (m_position.y.Mode == SizeMode::Proportional) {
      newSize.y += (T)(framebufferHeight * m_size.y.Value);
    } else { newPosition.y += m_position.y.Value; }

    m_geometry.SetSize(newSize);
    m_geometry.SetPosition(newPosition);
  }

  UIType GetType() const { return m_type; }
};

template <typename T, typename C>
class Panel : public UIElement<T, C> {
public:
  Panel(Vector2<UISize<T>> Size, Vector2<UISize<T>> Position, Colour<C> Colour)
  : UIElement<T, C>(Size, Position)
    { SetColour(Colour); UIElement<T, C>::SetVisibility(true); UIElement<T, C>::m_type = UIType::Panel; }

  Colour<C> GetColour() const { return UIElement<T, C>::m_geometry.GetColour(); }

  void SetColour(const Colour<C> NewColour) { UIElement<T, C>::m_geometry.SetColour(NewColour); }

  UIAnchor<T> GetAnchor() const { return UIElement<T, C>::m_anchor; }
};

template <typename T, typename C>
class Button : public UIElement<T, C> {
public:
  template <typename Func, typename... Args>
  Button(Vector2<UISize<T>> Size, Vector2<UISize<T>> Position, Func&& Function, Args&&... Arguments)
  : UIElement<T, C>(Size, Position)
    { 
      UIElement<T, C>::m_type = UIType::Button; 
      
      m_onClick = std::bind(std::forward<Func>(Function),
                            std::decay_t<Args>(Arguments)...
                           );
    }

  void OnClick() {
    if (m_onClick) m_onClick();
  }

private:
  std::function<void()> m_onClick;
};

template <typename T, typename C>
class PanelButton : public Panel<T, C> {
public:
  template <typename Func, typename... Args>
  PanelButton(Vector2<UISize<T>> Size, Vector2<UISize<T>> Position,
              Colour<C> Colour, Func&& Function, Args&&... Arguments)
  : Panel<T, C>(Size, Position, Colour),
    m_button(Size, Position, std::forward<Func>(Function), std::forward<Args>(Arguments)...)
    {
      UIElement<T, C>::m_type = UIType::Panel; 
    }

  void OnClick() {
    m_button.OnClick();
  }

  void SetSize(const Vector2<UISize<T>> NewSize) override {
    m_button.SetSize(NewSize);
    Panel<T, C>::SetSize(NewSize);
  }

  void SetPosition(const Vector2<UISize<T>> NewPosition) override {
    m_button.SetPosition(NewPosition);
    Panel<T, C>::SetPosition(NewPosition);
  }

  void SetAnchor(const UIAnchor<T> AnchorValue) override {
    m_button.SetAnchor(AnchorValue);
    Panel<T, C>::SetAnchor(AnchorValue);
  }

  size_t GetChildCount() const override {
    return Panel<T, C>::GetChildCount() + 1; // Add one for the sorted button object
  }

  void RecalculateGeometry(int framebufferWidth, int framebufferHeight) override {
    m_button.RecalculateGeometry(framebufferWidth, framebufferHeight);
    Panel<T, C>::RecalculateGeometry(framebufferWidth, framebufferHeight);
  }

  std::vector<std::shared_ptr<UIElement<T, C>>> GetChildren() const override {
    auto ret = UIElement<T, C>::GetChildren();
    ret.push_back(std::make_shared<Button<T, C>>(m_button));
    return ret;
  }

private:
  Button<T, C> m_button;
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

