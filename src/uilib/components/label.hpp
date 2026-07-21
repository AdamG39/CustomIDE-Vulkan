#ifndef CUSTOM_COMPONENT_LABEL_H
#define CUSTOM_COMPONENT_LABEL_H

#include "component.hpp"

namespace CustomIDE::UI::ECS {

class Label : public IText {
private:
  const std::string m_content;

public:
  static int TypeValue() { return TypeLabel; }
  int GetType() override;

  Label(Font Font, std::string Content, bool WordWrap = false)
  : m_content(Content) {
    m_font = Font;
    m_wordWrap = WordWrap;
  }

  void Render(EntityManager& Manager, const Transform* Transform) override;

  std::string GetContent() override;
  std::string GetContent() const override;
};

} // namespace UI::ECS

#endif

