#ifndef CUSTOM_COMPONENTS_H
#define CUSTOM_COMPONENTS_H

#include <functional>
#include <cassert>
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
  UIAnchorType m_anchor = UIAnchorType::Center;

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

  Vector2<float> CalculateEntityPosition(float ParentWidth, float ParentHeight, const UIAnchorType& Anchor) const {
    Vector2<float> calculatedPosition;

    switch (Anchor) {
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

  Vector2<float> GetPixelSize(float ParentWidth = (float)framebufferWidth,
                              float ParentHeight = (float)framebufferHeight) const {
    return CalculateEntitySize(ParentWidth, ParentHeight);
  }

  void SetPosition(const Vector2<UISize<float>>& Position) {
    m_position = Position;
  }

  Vector2<UISize<float>> GetPosition() const { return m_position; }

  Vector2<float> GetPixelPosition(float ParentWidth = (float)framebufferWidth,
                                  float ParentHeight = (float)framebufferHeight) const {
    return CalculateEntityPosition(ParentWidth, ParentHeight, m_anchor);
  }

  void SetAnchor(const UIAnchorType& AnchorValue) {
    m_anchor = AnchorValue;
  }

  UIAnchorType GetAnchor() const { return m_anchor; }
};

class StaticColour : public IComponent {
private:
  Colour<float> m_colour;

public:
  static int TypeValue() { return TypeStaticColour; }
  int GetType() override { return TypeValue(); }

  StaticColour() : m_colour() {}
  StaticColour(const Colour<float>& Colour) : m_colour(Colour) {}

  void SetColour(const Colour<float>& Colour) {
    m_colour = Colour;
  }

  Colour<float> GetColour() const {
    return m_colour;
  }
};

class Texture : public IComponent {
private:
  std::array<Vector2<float>, 4> m_textureCoords = {Vector2<float>(0.f, 0.f), Vector2<float>(1.f, 0.f),
                                                   Vector2<float>(0.f, 1.f), Vector2<float>(1.f, 1.f)};
  int m_textureIndex;

public:
  static int TypeValue() { return TypeTexture; }
  int GetType() override { return TypeValue(); }

  Texture() = default;
  Texture(int TextureIndex)
  : m_textureIndex(TextureIndex) {}
  Texture(int TextureIndex, const std::array<Vector2<float>, 4> TextureCoords)
  : m_textureCoords(std::move(TextureCoords)), m_textureIndex(TextureIndex) {}

  const std::array<Vector2<float>, 4>& GetTextureCoords() const {
    return m_textureCoords;
  }

  int GetTextureIndex() const {
    return m_textureIndex;
  }
};

class Button : public IComponent {
private:
  std::function<void()> m_onPress;
  std::function<void()> m_onRelease;
  std::function<void()> m_onHover;

public:
  static int TypeValue() { return TypeButton; }
  int GetType() override { return TypeValue(); }

  Button() = default;

  template <typename Func, typename... Args>
  void SetOnPress(Func&& Function, Args&&... Arguments) {
    m_onPress = std::bind(std::forward<Func>(Function), std::decay_t<Args>(Arguments)...);
  }

  template <typename Func, typename... Args>
  void SetOnRelease(Func&& Function, Args&&... Arguments) {
    m_onRelease = std::bind(std::forward<Func>(Function), std::decay_t<Args>(Arguments)...);
  }

  template <typename Func, typename... Args>
  void SetOnHover(Func&& Function, Args&&... Arguments) {
    m_onHover = std::bind(std::forward<Func>(Function), std::decay_t<Args>(Arguments)...);
  }

  void OnPress() {
    if (m_onPress) {
      m_onPress();
    } else {
      puts("no onpress");
      printf("[Warning]: No OnPress() function assigned for object: %p", this);
    }
  }

  void OnRelease() {
    if (m_onRelease) {
      m_onRelease();
    } else
      printf("[Warning]: No OnRelease() function assigned for object: %p", this);
  }

  void OnHover() {
    if (m_onHover) {
      m_onHover();
    } else
      printf("[Warning]: No OnHover() function assigned for object: %p", this);
  }
};

class Text : public IComponent {
public:
  static int TypeValue() { return TypeText; }
  int GetType() override { return TypeValue(); }
};

#endif

