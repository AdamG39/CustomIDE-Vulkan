#ifndef CUSTOM_COMPONENT_TEXT_BOX_H
#define CUSTOM_COMPONENT_TEXT_BOX_H

#include "component.hpp"
#include "GLFW/glfw3.h"

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

  std::string GetContent() override;
  std::string GetContent() const override;

#ifdef _DEBUG
  void Print();

  void DebugPrint();
#endif // _DEBUG
};

#endif

