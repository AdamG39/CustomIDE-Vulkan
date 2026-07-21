#ifndef CUSTOM_COMPONENT_TEXT_BOX_H
#define CUSTOM_COMPONENT_TEXT_BOX_H

#include "component.hpp"
#include "GLFW/glfw3.h"

namespace CustomIDE::UI::ECS {

class TextBox : public IText {
private:
  static constexpr int TAB_WIDTH = 4; // FIXME: Should be set using a configuration file instead

  static constexpr int CHAR_SPACING = 0; // FIXME: Should be set using a configuration file instead
  static constexpr int LINE_SPACING = 8; // FIXME: Should be set using a configuration file instead

  static constexpr float DEFAULT_WIDTH_MULTIPLIER = 0.125f;
  static constexpr float BLOCK_WIDTH_MULTIPLIER   = 1.0f;
  static constexpr float CURSOR_HEIGHT_MULTIPLIER = 1.25f;
  static constexpr float DEFAULT_X_OFFSET         = -0.5f;
  static constexpr float BLOCK_X_OFFSET           = 0.0f;

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

  void RenderSelection(EntityManager& Manager, const Vector2D& TextObjPos);
  void RenderCursor(EntityManager& Manager, const Vector2D& TextObjPos, const Vector2<int>& CursorPosition);

public:
  static int TypeValue() { return TypeTextBox; }
  int GetType() override;

  TextBox(Font Font, std::string Filepath = "", bool WordWrap = false);

  void Render(EntityManager& Manager, const Transform* Transform) override;

  char Index(unsigned Position);
  void Insert(char Character, int Position);
  void Delete(int Position);

  int GetCursorPosition() const;
  void MoveCursorLeft();
  void MoveCursorRight();
  void MoveCursorUp();
  void MoveCursorDown();
  void MoveBackWord();
  void MoveForwardWord();

  bool GetSelectionState() const;
  void StartSelection();
  void UpdateSelection(int PreviousPosition);
  void CancelSelection();
  void CopySelection(GLFWwindow* Window);
  void PasteText(GLFWwindow* Window);
  void DeleteSelection();

  void SaveFile();
  void LoadFile();

  Colour GetCursorColour() const;
  void SetCursorColour(const Colour& NewColour);

  TextCursorType GetCursorType() const;
  void SetCursorType(const TextCursorType & NewType);

  std::string GetContent() override;
  std::string GetContent() const override;

#ifdef _DEBUG
  void Print();

  void DebugPrint();
#endif // _DEBUG
};

class TextInteraction : public IInteractable {
private:
  const TextBox& m_textBox;

  void MoveTextCursorToMousePosition();
  void DragSelect(); // ?

public:
  static int TypeValue() { return TypeTextInteraction; }
  int GetType() override;

  TextInteraction(const TextBox& TextBox, GLFWwindow* Window);

  static void OnPress(GLFWwindow* Window);

  static void OnRelease(GLFWwindow* Window);
};

} // namespace UI::ECS

#endif

