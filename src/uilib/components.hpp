#ifndef CUSTOM_COMPONENTS_H
#define CUSTOM_COMPONENTS_H

#include <functional>
#include <string>
#include "../renderer/shapes.hpp"
#include "ui.hpp"
#include "text.hpp"

extern int framebufferWidth;
extern int framebufferHeight;

enum ComponentTypes{
  TypeTransform,
  TypeStaticColour,
  TypeTexture,
  TypeButton,
  TypeLabel,
  TypeTextBox
};

class IComponent {
protected:
public:
  virtual int GetType() = 0;
  virtual ~IComponent() = default;
};

class IRenderable : public IComponent {};

class Transform : public IComponent {
private:
  Vector2<UISize<float>> m_size;
  Vector2<UISize<float>> m_position;
  UIAnchorType m_anchor = UIAnchorType::Center;

  Vector2<float> CalculateEntitySize(float ParentWidth, float ParentHeight) const;

  Vector2<float> CalculateEntityPosition(float ParentWidth, float ParentHeight, const UIAnchorType& Anchor) const;

public:
  static int TypeValue() { return TypeTransform; }
  int GetType() override { return TypeValue(); }

  Transform() = default;
  Transform(Vector2<UISize<float>> Size, Vector2<UISize<float>> Position)
  : m_size(Size), m_position(Position) {}

  void SetSize(const Vector2<UISize<float>>& Size) { m_size = Size; }

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

class StaticColour : public IRenderable {
private:
  Colour<float> m_colour;

public:
  static int TypeValue() { return TypeStaticColour; }
  int GetType() override { return TypeValue(); }

  StaticColour() : m_colour() {}
  StaticColour(const Colour<float>& Colour) : m_colour(Colour) {}

  void SetColour(const Colour<float>& Colour) { m_colour = Colour; }

  Colour<float> GetColour() const { return m_colour; }
};

class Texture : public IRenderable {
private:
  std::array<Vector2<float>, 4> m_textureCoords = {Vector2<float>(0.f, 0.f), Vector2<float>(1.f, 0.f),
                                                   Vector2<float>(0.f, 1.f), Vector2<float>(1.f, 1.f)};
  int m_textureIndex;

public:
  static int TypeValue() { return TypeTexture; }
  int GetType() override { return TypeValue(); }

  Texture(int TextureIndex = -1) : m_textureIndex(TextureIndex) {}
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

// Static immutable text label
class Label : public IRenderable, public IText {
private:
  const std::string m_content;

public:
  static int TypeValue() { return TypeLabel; }
  int GetType() override { return TypeValue(); }

  Label(Font Font, std::string Content) : IText::IText(Font), m_content(Content) {}

  std::string GetContent() const override {
    return m_content;
  }
};

// Mutable text box
class TextBox : public IRenderable, public IText {
private:
  PieceTable m_table;

public:
  static int TypeValue() { return TypeTextBox; }
  int GetType() override { return TypeValue(); }

  TextBox(Font Font, std::string FileContents = "") : IText::IText(Font), m_table(FileContents) {}

  char Index(unsigned Position) { return m_table.Index(Position); }

  void Insert(char Character, int Position) { m_table.Insert(Character, Position); }
  void Insert(std::string Content, int Position);

  void Delete(int Position) { m_table.Delete(Position); }
  void Delete(int Position, int Count);

  std::string GetContent() const override { return m_table.GetContent(); }

#ifdef _DEBUG
  void Print() { m_table.Print(); }

  void DebugPrint() { m_table.DebugPrint(); }
#endif // _DEBUG
};

#endif

