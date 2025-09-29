#ifndef CUSTOM_UI_H
#define CUSTOM_UI_H

#include <vector>
#include <memory>
#include <algorithm>
#include <functional>
#include "../helpers/errors/errors.hpp"
#include "../renderer/shapes.hpp"

enum class UIType { Base, Panel, Button, PanelButton };

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
class UIElement {
protected:
  Vector2<UISize<T>> m_size;
  Vector2<UISize<T>> m_position;
  Vector2<T> m_calculatedSize;
  Vector2<T> m_calculatedPosition;
  UIAnchor<T> m_anchor;

  UIType m_type = UIType::Base;
  bool m_visible = false;

  std::vector<std::shared_ptr<UIElement<T, C>>> m_childObjects;
  UIElement<T, C>* m_parent = nullptr;

public:
  UIElement(Vector2<UISize<T>> Size, Vector2<UISize<T>> Position, UIElement<T, C>* Parent = nullptr)
  : m_size(Size), m_position(Position), m_parent(Parent) {}

  virtual ~UIElement() {
    delete m_parent;
  }

  T GetWidth() const { return m_size.x; }

  T GetHeight() const { return m_size.y; }

  Vector2<UISize<T>> GetSize() const { return m_size; }

  Vector2<T> GetPixelSize() const { return m_calculatedSize; }

  virtual void SetSize(const Vector2<UISize<T>> NewSize) {
    m_size = NewSize;
  }

  Vector2<UISize<T>> GetPosition() const { return m_position; }

  Vector2<T> GetPixelPosition() const { return m_calculatedPosition; }

  virtual void SetPosition(const Vector2<UISize<T>> NewPosition) {
    m_position = NewPosition;
  }

  UIAnchor<T> GetAnchor() const { return UIElement<T, C>::m_anchor; }

  virtual void SetAnchor(const UIAnchor<T> AnchorValue) {
    m_anchor = AnchorValue;
  }

  bool IsVisible() const { return m_visible; }

  void SetVisibility(const bool Value) { m_visible = Value; }

  virtual Rect<T, C> GetGeometry() const {
    return Rect<T, C>(m_calculatedSize,
                      m_calculatedPosition,
                      COLOUR_CLEAR);
  }

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

  void AdjustPositionRelativeToAnchor(Vector2<T>& Position, T Width, T Height) {
    switch (m_anchor.Type) {
      case UIAnchorType::Center:
        Position.x += Width / 2;
        Position.y += Height / 2;
        break;
      case UIAnchorType::Top:
        Position.x += Width / 2;
        break;
      case UIAnchorType::Left:
        Position.y += Height / 2;
        break;
      case UIAnchorType::Right:
        Position.x += Width;
        Position.y += Height / 2;
        break;
      case UIAnchorType::Bottom:
        Position.x += Width / 2;
        Position.y += Height;
        break;
      case UIAnchorType::TopLeft:         
        // Already relative to 0, 0 so no changes needed
        break;
      case UIAnchorType::TopRight:
        Position.x += Width;
        break;
      case UIAnchorType::BottomLeft:
        Position.y += Height;
        break;
      case UIAnchorType::BottomRight:
        Position.x += Width;
        Position.y += Height;
        break;
    }

    if (m_anchor.Offset.x.Mode == SizeMode::Proportional) {
      Position.x += Width * m_anchor.Offset.x.Value;
    } else { Position.x += m_anchor.Offset.x.Value; }

    if (m_anchor.Offset.y.Mode == SizeMode::Proportional) {
      Position.y += Height * m_anchor.Offset.y.Value;
    } else { Position.y += m_anchor.Offset.y.Value; }
  }

  virtual void RecalculateElementDimensions(T ParentWidth, T ParentHeight) {
    Vector2<T> newSize;
    Vector2<T> newPosition;
    if (m_size.x.Mode == SizeMode::Proportional) {
      newSize.x = ParentWidth * m_size.x.Value;
    } else { newSize.x = m_size.x.Value; }

    if (m_size.y.Mode == SizeMode::Proportional) {
      newSize.y = ParentHeight * m_size.y.Value;
    } else { newSize.y = m_size.y.Value; }


    AdjustPositionRelativeToAnchor(newPosition, ParentWidth, ParentHeight);

    if (m_position.x.Mode == SizeMode::Proportional) {
      newSize.x += ParentWidth * m_size.x.Value;
    } else { newPosition.x += m_position.x.Value; }

    if (m_position.y.Mode == SizeMode::Proportional) {
      newSize.y += ParentHeight * m_size.y.Value;
    } else { newPosition.y += m_position.y.Value; }

    m_calculatedSize = newSize;
    m_calculatedPosition = newPosition;
  }

  UIType GetType() const { return m_type; }
};

// TODO: Remove these unessessary classes in favour of the ECS /*
template <typename T, typename C>
class Panel : public UIElement<T, C> {
private:
  Colour<C> m_colour;
  int m_zIndex;

public:
  Panel(Vector2<UISize<T>> Size, Vector2<UISize<T>> Position, Colour<C> Colour,
        UIElement<T, C>* Parent = nullptr, int zIndex = 0)
  : UIElement<T, C>(Size, Position, Parent), m_colour(Colour), m_zIndex(zIndex) {
    UIElement<T, C>::SetVisibility(true); UIElement<T, C>::m_type = UIType::Panel;
  }

  Rect<T, C> GetGeometry() const override {
    return Rect<T, C>(UIElement<T, C>::m_calculatedSize,
                      UIElement<T, C>::m_calculatedPosition,
                      m_colour, m_zIndex);
  }

  Colour<C> GetColour() const { return m_colour; }

  void SetColour(const Colour<C> NewColour) { m_colour = NewColour; }

  int GetZIndex() const { return m_zIndex; }

  void SetZIndex(const int NewValue) { m_zIndex = NewValue; }
};

template <typename T, typename C>
class Button : public UIElement<T, C> {
public:
  template <typename Func, typename... Args>
  Button(Vector2<UISize<T>> Size, Vector2<UISize<T>> Position, Func&& Function, Args&&... Arguments)
  : UIElement<T, C>(Size, Position) { 
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
      UIElement<T, C>::m_type = UIType::PanelButton; 
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

  void RecalculateElementDimensions(T ParentWidth, T ParentHeight) override {
    UIElement<T, C>::RecalculateElementDimensions(ParentWidth, ParentHeight);
    m_button.RecalculateElementDimensions(ParentWidth, ParentHeight);
  }

  Button<T, C> GetButton() const {
    return m_button;
  }

  std::shared_ptr<Button<T, C>> GetButtonPtr() const {
    return std::make_shared<Button<T, C>>(m_button);
  }

private:
  Button<T, C> m_button;
}; //*/

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

