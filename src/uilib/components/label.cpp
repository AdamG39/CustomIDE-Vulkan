#include "label.hpp"
#include "transform.hpp"
#include "../ecs.hpp"
#include "../../helpers/errors/errors.hpp"
#include "../../helpers/text/textHelper.hpp"

namespace CustomIDE {

int UI::ECS::Label::GetType() { return TypeValue(); }

void UI::ECS::Label::Render(EntityManager& Manager, Transform* Transform) {
  TextHelper::TextRenderSettings settings {
    .Spacing = {
      .TabWidth = 4,
      .CharSpacing = 0,
      .LineSpacing = 8
    },
    .Font = GetFont(),
    .Content = GetContent(),
    .DrawDepth = m_drawDepth,
    .WordWrap = GetWordWrap()
  };

  TextHelper::RenderText(Manager, Transform, settings);
}

std::string UI::ECS::Label::GetContent() {
  return m_content;
}

std::string UI::ECS::Label::GetContent() const {
  return m_content;
}

} // namespace CustomIDE

