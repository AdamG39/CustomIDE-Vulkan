#include "label.hpp"
#include "transform.hpp"
#include "../ecs.hpp"
#include "../../helpers/errors/errors.hpp"

namespace CustomIDE {

int UI::ECS::Label::GetType() { return TypeValue(); }

void UI::ECS::Label::Render(EntityManager& Manager, const Transform* Transform) {
  // calculate size of each character based on font
  Font font = GetFont();
  Vector2 textObjPos = Transform->GetPixelPosition();
  Vector2 fontAtlasSize {64.f, 2.f};
  std::string content = GetContent();

  int charsPerLine = int(Transform->GetPixelSize().x) / font.size.x;
  // create a rect for each character
  int linePosition = 0;
  int lineCount = 0;

  for (size_t i = 0; i < content.size(); i++) {
    switch (content[i]) {
    case '\n':
      lineCount++;
      linePosition = 0;
      continue;
    case '\t':
      linePosition += 4;
      if (GetWordWrap()) {
        if (linePosition > charsPerLine) { 
          lineCount++;
          linePosition = 0;
        }
      }
      continue;
    }

    Vector2D charPosition {
      textObjPos.x + (font.size.x * linePosition),
      textObjPos.y + (lineCount * font.size.y)
    };

    linePosition++;

    if (GetWordWrap()) {
      if (linePosition > charsPerLine) { 
        lineCount++;
        linePosition = 0;
      }
    }

    Colour textColour = font.colour;
    auto imageIndex = Manager.GetRenderer().lock()->GetImageIndexFromName(font.familyName);
    if (imageIndex < 0) Errors::ExitWithError("No image with that name found", -35);

    ClipRect clipRect {.clippingEnabled = false};
    if (!Manager.GetClipStack().empty())
      clipRect = Manager.GetClipStack().top();

    Manager.GetRenderer().lock()->DrawTexturedRectEx(
        Rect2D{ static_cast<int32_t>(charPosition.x), static_cast<int32_t>(charPosition.y),
                static_cast<uint32_t>(font.size.x), static_cast<uint32_t>(font.size.y) },
        CalculateCharUV(fontAtlasSize, content[i]), m_drawDepth, imageIndex, clipRect,
        textColour);
  }
}

std::string UI::ECS::Label::GetContent() {
  return m_content;
}

std::string UI::ECS::Label::GetContent() const {
  return m_content;
}

} // namespace CustomIDE

