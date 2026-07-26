#include "uiFactory.hpp"
#include <GLFW/glfw3.h>
#include <variant>

namespace CustomIDE {

#define UNUSED_PROPERTY 0

// Default values
#define DEFAULT_TOP_BAR_HEIGHT 40
#define DEFAULT_TOP_BAR_BUTTON_WIDTH 50
#define DEFAULT_TOP_BAR_BUTTON_IMAGE_SIZE Vector2D{ 10.f, 10.f }
#define DEFAULT_FONT_FILEPATH "../assets/unscii-alt-font-16.png"

namespace UIFactory {
  std::shared_ptr<UI::ECS::EntityManager> EntityManager;
  std::shared_ptr<EventSystem::EventManager> EventManager;
  std::shared_ptr<Vulkan::Renderer> Renderer;

  ColourPalette DefaultColourPalette {
    .BackgroundColour = {},
    .AccentColour1 = {},
    .AccentColour2 = {},
    .TextColour = {}
  };

  ButtonSettings DefaultCloseButtonSettings() {
    return {
      .Size = Vector2D{ DEFAULT_TOP_BAR_BUTTON_WIDTH, DEFAULT_TOP_BAR_HEIGHT },
      .ButtonColour = DefaultColourPalette.AccentColour1,
      .ImageSettings = std::optional<ImageSettings>(std::in_place, ImageSettings{
        .ImageIndex = Renderer->GetImageIndexFromName("closeButtonCross"),
        .ImageSize = DEFAULT_TOP_BAR_BUTTON_IMAGE_SIZE,
        .ImageColour = COLOUR_WHITE
      })
    };
  }
} // namespace UIFactory

void UIFactory::SetEntityManager(const std::shared_ptr<UI::ECS::EntityManager>& _EntityManager) {
  UIFactory::EntityManager = _EntityManager;
}

void UIFactory::SetEventManager(const std::shared_ptr<EventSystem::EventManager>& _EventManager) {
  UIFactory::EventManager = _EventManager;
}

void UIFactory::SetRenderer(const std::shared_ptr<Vulkan::Renderer>& _Renderer) {
  UIFactory::Renderer = _Renderer;
}

void UIFactory::SetDefaultColourPalette(const ColourPalette& Palette) {
  UIFactory::DefaultColourPalette = Palette;
}

UI::Font UIFactory::DefaultFont() {
  return UI::CreateFont(DEFAULT_FONT_FILEPATH, DefaultColourPalette.TextColour);
}

UIFactory::TopBarSettings UIFactory::DefaultTopBarSettings() {
  return TopBarSettings{
    .BackgroundColour = DefaultColourPalette.AccentColour1,
    .BarHeight = DEFAULT_TOP_BAR_HEIGHT,
    .ButtonSettings = {
      .CloseButton = DefaultCloseButtonSettings(),
      .MaximiseButton = {},
      .MinimiseButton = {},
      .Alignment = TopBarButtonAlignment::RIGHT
    },
    .TitleSettings = std::nullopt
  };
}

void UIFactory::SetTopBarTitleSettings(std::optional<UIFactory::TopBarLabelSettings>& Settings, std::string Title, TopBarLabelAlignment Alignment, UI::Font Font) {
  Settings.emplace(TopBarLabelSettings{
    .Title = Title,
    .Alignment = Alignment,
    .TitleFont = Font
  });
}

UI::ECS::Entity& UIFactory::CreateButton(const ButtonSettings& Settings, const Vector2D& Position) {
  UI::ECS::Entity& closeButton = EntityManager->AddEntity(Settings.Size, Position);

  return closeButton;
}

UI::ECS::Entity& UIFactory::CreateTopBar(const UIFactory::TopBarSettings& Settings) {
  // Create titlebar
  UI::ECS::Entity& titleBar = EntityManager->AddEntity(Vector2D({UNUSED_PROPERTY, Settings.BarHeight}),
                                                       Vector2D({UNUSED_PROPERTY, Settings.BarHeight / 2.0f}));

  titleBar.GetComponent<UI::ECS::Transform>()->SetAnchorPreset(UI::AnchorPresets::STRETCH_TOP);
  titleBar.AddComponent<UI::ECS::Image>(Settings.BackgroundColour);

  // Create close button if settings are provided
  if (Settings.ButtonSettings.CloseButton.has_value()) {
    auto closeButtonSettings = Settings.ButtonSettings.CloseButton.value();
    Vector2D closeButtonPosition{
      0.0f, 0.0f
    };

    switch (Settings.ButtonSettings.Alignment) {
    case TopBarButtonAlignment::LEFT:
      closeButtonPosition.x ;
      break;
    }

    titleBar.AddChild(CreateButton(closeButtonSettings, closeButtonPosition));
  }

  std::shared_ptr closeButton = titleBar.GetLastChild();

  closeButton->GetComponent<UI::ECS::Transform>()->SetAnchorPreset(UI::AnchorPresets::CENTER_RIGHT);
  closeButton->AddComponent<UI::ECS::Image>(Colour(0xe81123, 1.f));
  closeButton->AddComponent<UI::ECS::Button>();
  closeButton->GetComponent<UI::ECS::Button>()->SetOnRelease(&glfwSetWindowShouldClose, Renderer->GetWindow(), GLFW_TRUE);

  // TODO: find a better solution for registering buttons for mouse events
  EventManager->RegisterEventListener(closeButton.get(), EventSystem::EventType::Mouse);


  closeButton->AddChild(UI::ECS::Entity(Vector2D{10.f, 10.f},
                                        Vector2D{0.f, 0.f}));

  std::shared_ptr closeButtonCross = closeButton->GetLastChild();

  closeButtonCross->AddComponent<UI::ECS::Image>(COLOUR_WHITE, Renderer->GetImageIndexFromName("cross"));

  titleBar.AddChild(UI::ECS::Entity(Vector2D{600.f, 40.f}, Vector2D{315.f, 13.f}));

  std::shared_ptr titleLabel = titleBar.GetLastChild();

  titleLabel->GetComponent<UI::ECS::Transform>()->SetAnchorPreset(UI::AnchorPresets::CENTER_LEFT);

  return titleBar;
}

} // namespace CustomIDE
