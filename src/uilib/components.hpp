#ifndef CUSTOM_COMPONENTS_H
#define CUSTOM_COMPONENTS_H

#include "../renderer/shapes.hpp"
#include "ui.hpp"

extern int framebufferWidth;
extern int framebufferHeight;

enum ComponentTypes{
  TypeTransform,
  TypeStaticColour,
  TypeTexture,
  TypeButton,
  TypeText
};

class IComponent {
protected:
public:
  virtual int GetType() = 0;
  virtual ~IComponent() = default;
};

class Transform : public IComponent {
private:
  Vector2<UISize<float>> m_size;
  Vector2<UISize<float>> m_position;
  UIAnchor<float> m_anchor;

  Vector2<float> CalculateEntitySize(float ParentWidth, float ParentHeight) const {
    Vector2<float> calculatedSize;

    if (m_size.x.Mode == SizeMode::Proportional) {
      calculatedSize.x = ParentWidth * m_size.x.Value;
    } else { calculatedSize.x = m_size.x.Value; }

    if (m_size.y.Mode == SizeMode::Proportional) {
      calculatedSize.y = ParentHeight * m_size.y.Value;
    } else { calculatedSize.y = m_size.y.Value; }

    return calculatedSize;
  }

  Vector2<float> CalculateEntityPosition(float ParentWidth, float ParentHeight, const UIAnchor<float>& Anchor) const {
    Vector2<float> calculatedPosition;

    switch (m_anchor.Type) {
      case UIAnchorType::Center:
        calculatedPosition.x = ParentWidth / 2;
        calculatedPosition.y = ParentHeight / 2;
        break;
      case UIAnchorType::Top:
        calculatedPosition.x = ParentWidth / 2;
        break;
      case UIAnchorType::Left:
        calculatedPosition.y = ParentHeight / 2;
        break;
      case UIAnchorType::Right:
        calculatedPosition.x = ParentWidth;
        calculatedPosition.y = ParentHeight / 2;
        break;
      case UIAnchorType::Bottom:
        calculatedPosition.x = ParentWidth / 2;
        calculatedPosition.y = ParentHeight;
        break;
      case UIAnchorType::TopLeft:         
        // Already relative to 0, 0 so no changes needed
        break;
      case UIAnchorType::TopRight:
        calculatedPosition.x = ParentWidth;
        break;
      case UIAnchorType::BottomLeft:
        calculatedPosition.y = ParentHeight;
        break;
      case UIAnchorType::BottomRight:
        calculatedPosition.x = ParentWidth;
        calculatedPosition.y = ParentHeight;
        break;
    }

    if (m_anchor.Offset.x.Mode == SizeMode::Proportional) {
      calculatedPosition.x += ParentWidth * m_anchor.Offset.x.Value;
    } else { calculatedPosition.x += m_anchor.Offset.x.Value; }

    if (m_anchor.Offset.y.Mode == SizeMode::Proportional) {
      calculatedPosition.y += ParentHeight * m_anchor.Offset.y.Value;
    } else { calculatedPosition.y += m_anchor.Offset.y.Value; }

    if (m_position.x.Mode == SizeMode::Proportional) {
      calculatedPosition.x += ParentWidth * m_position.x.Value;
    } else { calculatedPosition.x += m_position.x.Value; }

    if (m_position.y.Mode == SizeMode::Proportional) {
      calculatedPosition.y += ParentHeight * m_position.y.Value;
    } else { calculatedPosition.y += m_position.y.Value; }

    return calculatedPosition;
  }

public:
  static int TypeValue() { return TypeTransform; }
  int GetType() override { return TypeValue(); }

  Transform() = default;
  Transform(Vector2<UISize<float>> Size, Vector2<UISize<float>> Position)
  : m_size(Size), m_position(Position) {}

  void SetSize(const Vector2<UISize<float>>& Size) { 
    m_size = Size; 
  }

  Vector2<UISize<float>> GetSize() const { return m_size; }

  Vector2<float> GetPixelSize() const {
    // if no parent
    return CalculateEntitySize(float(framebufferWidth), float(framebufferHeight));
    // if has parent change arguments to parents width and size
  }

  void SetPosition(const Vector2<UISize<float>>& Position) {
    m_position = Position;
  }

  Vector2<UISize<float>> GetPosition() const { return m_position; }

  Vector2<float> GetPixelPosition() const {
    // if no parent
    return CalculateEntityPosition(float(framebufferWidth), float(framebufferHeight), m_anchor);
    // if has parent change arguments to parents width and size
  }

  void SetAnchor(const UIAnchor<float>& AnchorValue) {
    m_anchor = AnchorValue;
  }

  UIAnchor<float> GetAnchor() const { return m_anchor; }
};

class StaticColour : public IComponent {
private:
  Colour<float> m_colour;

public:
  static int TypeValue() { return TypeStaticColour; }
  int GetType() override { return TypeValue(); }

  StaticColour() : m_colour(Colour<float>()) {}
  StaticColour(const Colour<float>& Colour) : m_colour(Colour) {}

  void SetColour(const Colour<float>& Colour) {
    m_colour = Colour;
  }

  Colour<float> GetColour() const {
    return m_colour;
  }
};

class Texture : public IComponent {
public:
  static int TypeValue() { return TypeTexture; }
  int GetType() override { return TypeValue(); }
};

/*
class Button : public IComponent {
public:
  int GetType() override { return TypeButton; }
};*/

class Text : public IComponent {
public:
  static int TypeValue() { return TypeText; }
  int GetType() override { return TypeValue(); }
};

#endif

