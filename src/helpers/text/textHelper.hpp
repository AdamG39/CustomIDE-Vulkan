#ifndef CUSTOM_RENDER_TEXT_HELPER_H
#define CUSTOM_RENDER_TEXT_HELPER_H

#include "../errors/errors.hpp"
#include "../../renderer/shapes.hpp"
#include "../../uilib/ecs.hpp"
#include "../../uilib/text.hpp"
#include "../../uilib/components/component.hpp"

namespace CustomIDE::TextHelper {

struct TextSpacingSettings {
  int32_t TabWidth;
  int32_t CharSpacing;
  int32_t LineSpacing;
};

struct TextRenderSettings {
  TextSpacingSettings Spacing;
  UI::Font Font;
  std::string Content;
  int DrawDepth;
  bool WordWrap;
};

void RenderText(UI::ECS::EntityManager& Manager, UI::ECS::Transform* Transform, const TextRenderSettings& Settings,
                UI::TextCursor* Cursor = nullptr, Vector2<int>* CursorPosition = nullptr);

} // namespace TextHelper

#endif
