#ifndef CUSTOM_COMPONENTS_H
#define CUSTOM_COMPONENTS_H

#include <functional>
#include <string>
#include "../renderer/shapes.hpp"
#include "ui.hpp"
#include "text.hpp"
#include <GLFW/glfw3.h>

enum ComponentTypes{
  TypeTransform,
  TypeImage,
  TypeButton,
  TypeLabel,
  TypeTextBox,
  TypeMask
};

class Transform;
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
  virtual void Render(EntityManager& Manager, const Transform* Transform) override;

  Font GetFont() const { return m_font; }

  void SetWordWrap(bool Value) { m_wordWrap = Value; }
  bool GetWordWrap() { return m_wordWrap; }

  virtual std::string GetContent() = 0;
  virtual std::string GetContent() const = 0;
};

class Transform : public IComponent {
private:
  Vector2<UISize<float>> m_size;
  Vector2<UISize<float>> m_position;
  UIAnchorType m_anchor = UIAnchorType::Center;

  Vector2<float> m_pixelSize;
  Vector2<float> m_pixelPosition;

public:
  static int TypeValue() { return TypeTransform; }
  int GetType() override { return TypeValue(); }

  Transform() = default;
  Transform(Vector2<UISize<float>> Size, Vector2<UISize<float>> Position)
  : m_size(Size), m_position(Position), m_anchor(UIAnchorType::Center) {}

  void SetSize(const Vector2<UISize<float>>& Size) { m_size = Size; }

  Vector2<UISize<float>> GetSize() const { return m_size; }

  Vector2<float> RecalculateEntitySize(float ParentWidth, float ParentHeight);

  Vector2<float> GetPixelSize() const {
    return m_pixelSize;
  }

  void SetPosition(const Vector2<UISize<float>>& Position) {
    m_position = Position;
  }

  Vector2<UISize<float>> GetPosition() const { return m_position; }

  Vector2<float> RecalculateEntityPosition(Vector2<float> ParentSize, 
    Vector2<float> ParentPosition, const UIAnchorType& Anchor);

  Vector2<float> GetPixelPosition() const {
    return m_pixelPosition;
  }

  void SetAnchor(const UIAnchorType& AnchorValue) {
    m_anchor = AnchorValue;
  }

  UIAnchorType GetAnchor() const { return m_anchor; }
};

class UIImage : public IRenderable {
private:
  Colour m_colour;
  UVRect2D m_uvRect = { 0.5f, 0.5f, 1.f, 1.f };
  int m_textureIndex;

public:
  static int TypeValue() { return TypeImage; }
  int GetType() override { return TypeValue(); }

  UIImage(const Colour& _Colour = Colour(), int TextureCoords = -1)
  : m_colour(_Colour), m_textureIndex(TextureCoords) {}

  void Render(EntityManager& Manager, const Transform* Transform) override;

  void SetColour(const Colour& Colour) { m_colour = Colour; }
  Colour GetColour() const { return m_colour; }

  void SetUVRect(UVRect2D UVRect) { m_uvRect = UVRect; }
  const UVRect2D& GetUVRect() const { return m_uvRect; }

  void SetTextureIndex(int TextureIndex) { m_textureIndex = TextureIndex; }
  int GetTextureIndex() const { return m_textureIndex; }
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
class Label : public IText {
private:
  const std::string m_content;

public:
  static int TypeValue() { return TypeLabel; }
  int GetType() override { return TypeValue(); }

  Label(Font Font, std::string Content, bool WordWrap = false)
  : m_content(Content) {
    m_font = Font;
    m_wordWrap = WordWrap;
  }

  std::string GetContent() override { return m_content; }
  std::string GetContent() const override { return m_content; }
};

// Mutable text box
class TextBox : public IText {
private:
  enum SelectionDirection {
    None,
    Left,
    Right
  };

  std::string m_filepath;

  TextCursor m_cursor{};
  PieceTable m_table;

  bool m_selectionState = false;
  SelectionDirection m_selectionDirection = None;
  TextSelection m_textSelection;

public:
  static int TypeValue() { return TypeTextBox; }
  int GetType() override { return TypeValue(); }

  TextBox(Font Font, std::string Filepath = "", bool WordWrap = false)
  : m_filepath(Filepath) {
    LoadFile();
    m_font = Font;
    m_wordWrap = WordWrap;
    m_cursor.Colour = Font.colour;
  }

  void Render(EntityManager& Manager, const Transform* Transform) override;

  char Index(unsigned Position) { return m_table.Index(Position); }
  void Insert(char Character, int Position);
  void Delete(int Position) { m_table.Delete(Position); }

  int GetCursorPosition() const { return m_cursor.Position; }
  void MoveCursorLeft();
  void MoveCursorRight();
  void MoveCursorUp();
  void MoveCursorDown();
  void MoveBackWord();
  void MoveForwardWord();

  bool GetSelectionState() const { return m_selectionState; }
  void StartSelection();
  void SelectLeft();
  void SelectRight();
  void EndSelection();
  void CopySelection(GLFWwindow* Window);
  void PasteText(GLFWwindow* Window);
  void DeleteSelection();

  void SaveFile();
  void LoadFile();

  Colour GetCursorColour() const { return m_cursor.Colour; }
  void SetCursorColour(const Colour& NewColour) { m_cursor.Colour = NewColour; }

  std::string GetContent() override { return m_table.GetContent(); }
  std::string GetContent() const override { return m_table.GetContent(); }

#ifdef _DEBUG
  void Print() { m_table.Print(); }

  void DebugPrint() { m_table.DebugPrint(); }
#endif // _DEBUG
};

class Mask : public IComponent {
private:
  ClipRect m_clipRect;

public:
  static int TypeValue() { return TypeMask; }
  int GetType() override { return TypeValue(); }

  Mask(ClipRect Rect = {}) : m_clipRect(Rect) {}

  ClipRect GetClipArea() { return m_clipRect; }
};

class ScrollContainer : public IRenderable {
  
};

#endif

