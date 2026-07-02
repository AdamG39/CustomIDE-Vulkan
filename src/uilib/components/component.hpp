#ifndef CUSTOM_COMPONENTS_H
#define CUSTOM_COMPONENTS_H

#include <string>
#include <unordered_map>
#include <functional>
#include <stdexcept>
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

class IInteractable : public IComponent {
private:
  std::unordered_map<std::string, std::function<void()>> m_actions;

public:
  template <typename... Params, typename... Args>
  void AddAction(std::string ActionName, void(*Function)(Params... params), Args... Arguments) {
    m_actions[ActionName] = [=] () { Function(Arguments...); };
  }

  bool RemoveAction(std::string ActionName) {
    return m_actions.erase(ActionName) != 0;
  }

  bool HasAction(std::string ActionName) {
    try {
      (void)m_actions.at(ActionName);
    }
    catch ( std::out_of_range Exception) {
      return false;
    }

    return true;
  }

  bool ExecAction(std::string ActionName) {
    if (!HasAction(ActionName)) return false;

    if (m_actions[ActionName]) {
      m_actions[ActionName]();
    } else {
      printf("[Warning]: No %s() function assigned for object: %p\n", ActionName.c_str(), this);
      return false;
    }

    return true;
  }
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

