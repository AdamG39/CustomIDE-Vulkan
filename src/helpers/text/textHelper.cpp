#include "textHelper.hpp"

namespace CustomIDE {

void TextHelper::RenderText(UI::ECS::EntityManager& Manager, UI::ECS::Transform* Transform, const TextHelper::TextRenderSettings& Settings,
                            UI::TextCursor* Cursor, Vector2<int>* CursorPosition) {
  Vector2D textObjPos = Transform->GetGlobalPosition();
  Vector2D textObjSize = Transform->GetGlobalSize();
  Vector2D fontAtlasSize {64.f, 2.f};

  int charsPerLine = int(textObjSize.x) / Settings.Font.size.x;
  // create a rect for each character
  int linePosition = 0;
  int lineCount = 0;

  int cursorIndexPosition = (Cursor) ? Cursor->Position : -1;

  for (size_t i = 0; i < Settings.Content.size(); i++) {
    if (cursorIndexPosition == i && CursorPosition)
      *CursorPosition = { linePosition, lineCount };

    switch (Settings.Content[i]) {
    case '\n':
      lineCount++;
      linePosition = 0;
      continue;
    case '\t':
      linePosition += Settings.Spacing.TabWidth;
      if (Settings.WordWrap) {
        if (linePosition > charsPerLine) { 
          lineCount++;
          linePosition = 0;
        }
      }
      continue;
    case ' ':
      linePosition++;
      continue;
    }

    int32_t charSpacing{Settings.Spacing.CharSpacing};

    // If not an even number integer division causes spacing to be handled incorrectly
    if (charSpacing & 1) charSpacing += 1; // Round to nearest even number

    Vector2D charPosition {
      textObjPos.x + (Settings.Font.size.x * linePosition) + (linePosition * charSpacing),
      textObjPos.y + (lineCount * Settings.Font.size.y) + (lineCount * Settings.Spacing.LineSpacing)
    };

    // Adjust to upper left corner instead of center of text element
    charPosition.x -= textObjSize.x / 2.f - Settings.Font.size.x;
    charPosition.y -= textObjSize.y / 2.f - Settings.Font.size.y;

    linePosition++;

    if (Settings.WordWrap) {
      if (linePosition > charsPerLine) { 
        lineCount++;
        linePosition = 0;
      }
    }

    Colour textColour = Settings.Font.colour;
    if (cursorIndexPosition == i && Cursor->Type == UI::TextCursorType::BLOCK) {
      textColour.r = 1.f - textColour.r;
      textColour.g = 1.f - textColour.g;
      textColour.b = 1.f - textColour.b;
    }
    auto imageIndex = Manager.GetRenderer().lock()->GetImageIndexFromName(Settings.Font.familyName);
    if (imageIndex < 0) Errors::ExitWithError("No image with that name found", -35);

    ClipRect clipRect {};
    if (!Manager.GetClipStack().empty())
      clipRect = Manager.GetClipStack().top();

    Manager.GetRenderer().lock()->DrawTexturedRectEx(
        Rect2D{ static_cast<int32_t>(charPosition.x), static_cast<int32_t>(charPosition.y),
                static_cast<uint32_t>(Settings.Font.size.x), static_cast<uint32_t>(Settings.Font.size.y) },
        UI::CalculateCharUV(fontAtlasSize, Settings.Content[i]), Settings.DrawDepth + 1, imageIndex, clipRect,
        textColour);
  }

  if (cursorIndexPosition == Settings.Content.size())
    *CursorPosition = { linePosition, lineCount };
}

} // namespace CustomIDE
