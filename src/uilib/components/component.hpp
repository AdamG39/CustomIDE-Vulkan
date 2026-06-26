#ifndef CUSTOM_COMPONENTS_H
#define CUSTOM_COMPONENTS_H

#include <string>
#include "../text.hpp"
#include <GLFW/glfw3.h>

enum ComponentTypes{
  TypeTransform,
  TypeImage,
  TypeButton,
  TypeLabel,
  TypeTextBox,
  TypeMask
};

// Pre-declare component classes full definition is within their own files
class Transform;
class UIImage;
class Button;
class Label;
class TextBox;
class Mask;
class EntityManager;

class IComponent {
public:
  virtual int GetType() = 0;
  virtual ~IComponent() = default;
};

class IRenderable : public IComponent {
protected:
  int m_drawDepth = 0;

public:
  void SetDrawDepth(int DrawDepth) { m_drawDepth = DrawDepth; }
  int GetDrawDepth() { return m_drawDepth; }

  virtual void Render(EntityManager& Manager, const Transform* Transform) = 0;
};

class IText : public IRenderable {
protected:
  Font m_font;
  bool m_wordWrap;

public:
  Font GetFont() const { return m_font; }

  void SetWordWrap(bool Value) { m_wordWrap = Value; }
  bool GetWordWrap() { return m_wordWrap; }

  virtual std::string GetContent() = 0;
  virtual std::string GetContent() const = 0;
};

#endif

