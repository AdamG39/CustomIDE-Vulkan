#ifndef CUSTOM_COMPONENTS_H
#define CUSTOM_COMPONENTS_H

#include "../renderer/shapes.hpp"
#include "ui.hpp"

enum ComponentTypes{
  TypeTransform,
  TypeStaticColour,
  TypeTexture,
  TypeButton,
  TypeText
};

class IComponent {
public:
  virtual int GetType() = 0; 
};

class Transform : public IComponent {
private:
  Vector2<UISize<float>> m_size;
  Vector2<UISize<float>> m_position;
  UIAnchor<float> m_anchor;

public:
  int GetType() override { return TypeTransform; }

  Transform() = default;
  Transform(Vector2<UISize<float>> Size, Vector2<UISize<float>> Position)
  : m_size(Size), m_position(Position) {}

  void SetSize(const Vector2<UISize<float>>& Size) { m_size = Size; }

  Vector2<UISize<float>> GetSize() const { return m_size; }

  void SetPosition(const Vector2<UISize<float>>& Position) { m_position = Position; }

  Vector2<UISize<float>> GetPosition() const { return m_position; }

  void SetAnchor(const UIAnchor<float>& AnchorValue) { m_anchor = AnchorValue; }

  UIAnchor<float> GetAnchor() const { return m_anchor; }
};

class StaticColour : public IComponent {
private:
  Colour<float> m_colour;

public:
  int GetType() override { return TypeStaticColour; }

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
  int GetType() override { return TypeTexture; }
};

/*
class Button : public IComponent {
public:
  int GetType() override { return TypeButton; }
};*/

class Text : public IComponent {
public:
  int GetType() override { return TypeText; }
};

#endif

