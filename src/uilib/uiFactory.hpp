#ifndef CUSTOM_UI_FACTORY_H
#define CUSTOM_UI_FACTORY_H

#include "entity.hpp"
#include "ecs.hpp"
#include "../event/eventManager.hpp"
#include <optional>
#include <memory>

namespace CustomIDE::UIFactory {

void SetEntityManager(const std::shared_ptr<UI::ECS::EntityManager>& _EntityManager);
void SetEventManager(const std::shared_ptr<EventSystem::EventManager>& _EventManager);
void SetRenderer(const std::shared_ptr<Vulkan::Renderer>& _Renderer);

UI::Font DefaultFont();

struct ColourPalette {
  Colour BackgroundColour;
  Colour AccentColour1;
  Colour AccentColour2;
  Colour TextColour;
};

void SetDefaultColourPalette(const ColourPalette& Palette);

enum class TopBarButtonAlignment {
  LEFT,
  RIGHT
};

enum class TopBarLabelAlignment {
  LEFT,
  CENTER,
  RIGHT
};

struct ImageSettings {
  TextureID ImageIndex;
  Vector2D ImageSize;
  Colour ImageColour;
};

struct ButtonSettings {
  Vector2D Size;
  Colour ButtonColour;
  std::optional<ImageSettings> _ImageSettings;
};

struct TopBarButtonSettings {
  // Y size values of buttons will be clamped to top bar height
  std::optional<ButtonSettings> CloseButton;
  std::optional<ButtonSettings> MaximiseButton;
  std::optional<ButtonSettings> MinimiseButton;

  TopBarButtonAlignment Alignment;
};

struct TopBarLabelSettings {
  std::string Title;
  TopBarLabelAlignment Alignment;
  UI::Font TitleFont;
};

struct TopBarSettings {
  Colour BackgroundColour;
  float BarHeight;
  TopBarButtonSettings ButtonSettings;
  std::optional<TopBarLabelSettings> TitleSettings;
  //std::optional<std::vector<Object>> AdditionalObjects;
};

TopBarSettings DefaultTopBarSettings();
void SetTopBarTitleSettings(std::optional<TopBarLabelSettings>& Settings, std::string Title, TopBarLabelAlignment Alignment = TopBarLabelAlignment::LEFT, UI::Font Font = DefaultFont());

UI::ECS::Entity& CreateButton(const ButtonSettings& Settings, const Vector2D& Position);

UI::ECS::Entity& CreateTopBar(const TopBarSettings& Settings);

} // namespace UIFactory

#endif
