#include "uiFactory.hpp"
#include "text.hpp"
#include <GLFW/glfw3.h>

namespace CustomIDE {

#define UNUSED_PROPERTY 0

// Default values
#define DEFAULT_TOP_BAR_HEIGHT 40
#define DEFAULT_TOP_BAR_BUTTON_WIDTH 50
#define DEFAULT_TOP_BAR_BUTTON_IMAGE_SIZE Vector2D{ 10.f, 10.f }
#define DEFAULT_FONT_FILEPATH "../assets/unscii-alt-font-16.png"
#define DEFAULT_TOP_BAR_BUTTON_COLOUR Colour(0x000000, 0.0f) /* Transparent colour */
#define DEFAULT_CLOSE_BUTTON_FOCUSED_COLOUR Colour(0xE81123, 1.0f)
#define DEFAULT_MAXIMISE_BUTTON_FOCUSED_COLOUR Colour(0xFFFFFF, 0.01f)
#define DEFAULT_MINIMISE_BUTTON_FOCUSED_COLOUR Colour(0xFFFFFF, 0.01f)
#define DEFAULT_TOP_BAR_BUTTON_IMAGE_FOCUSED_COLOUR Colour(0xFFFFFF, 1.0f)
#define DEFAULT_TOP_BAR_BUTTON_IMAGE_UNFOCUSED_COLOUR Colour(0x79797b, 1.0f)

#define DEFAULT_BUTTON_HOVER_LAMBDA(entity, backgroundColour, imageColour)  \
[Entity = entity, bc = backgroundColour, ic = imageColour]() {              \
  Entity->GetComponent<UI::ECS::Image>()->SetColour(bc);                    \
  auto child = Entity->GetLastChild();                                      \
  if (!child) return;                                                       \
  auto* imageComponent = child->GetComponent<UI::ECS::Image>();             \
  if (imageComponent == nullptr) return;                                    \
  imageComponent->SetColour(ic);                                            \
}

namespace UIFactory {
  std::shared_ptr<UI::ECS::EntityManager> EntityManager;
  std::shared_ptr<EventSystem::EventManager> EventManager;
  std::shared_ptr<Vulkan::Renderer> Renderer;

  ColourPalette DefaultColourPalette {
    .BackgroundColour = THEME_DARK_COLOUR_0,
    .AccentColour1 = THEME_DARK_COLOUR_1,
    .AccentColour2 = {},
    .TextColour = {}
  };

  ButtonSettings DefaultCloseButtonSettings() {
    return {
      .Size = Vector2D{ DEFAULT_TOP_BAR_BUTTON_WIDTH, DEFAULT_TOP_BAR_HEIGHT },
      .ButtonColours = {
        .UnfocusedColour = std::optional<Colour>(std::in_place, DEFAULT_TOP_BAR_BUTTON_COLOUR),
        .FocusedColour = std::optional<Colour>(std::in_place, DEFAULT_CLOSE_BUTTON_FOCUSED_COLOUR)
      },
      ._ImageSettings = std::optional<ImageSettings>(std::in_place, ImageSettings{
        .ImageIndex = Renderer->GetImageIndexFromName("closeButtonImage"),
        .ImageSize = DEFAULT_TOP_BAR_BUTTON_IMAGE_SIZE,
        .ImageColour = DEFAULT_TOP_BAR_BUTTON_IMAGE_UNFOCUSED_COLOUR
      }),
      .Callbacks = std::optional<ButtonCallbacks>(std::in_place, ButtonCallbacks{
        .OnPressCallback = nullptr,
        .OnReleaseCallback = [=]() { glfwSetWindowShouldClose(Renderer->GetWindow(), GLFW_TRUE); },
        .OnHoverEnterCallback = nullptr,
        .OnHoverExitCallback = nullptr
      })
    };
  }

  ButtonSettings DefaultMaximiseButtonSettings() {
    return {
      .Size = Vector2D{ DEFAULT_TOP_BAR_BUTTON_WIDTH, DEFAULT_TOP_BAR_HEIGHT },
      .ButtonColours = {
        .UnfocusedColour = std::optional<Colour>(std::in_place, DEFAULT_TOP_BAR_BUTTON_COLOUR),
        .FocusedColour = std::optional<Colour>(std::in_place, DEFAULT_MAXIMISE_BUTTON_FOCUSED_COLOUR)
      },
      ._ImageSettings = std::optional<ImageSettings>(std::in_place, ImageSettings{
        .ImageIndex = Renderer->GetImageIndexFromName("maximiseButtonImage"),
        .ImageSize = DEFAULT_TOP_BAR_BUTTON_IMAGE_SIZE,
        .ImageColour = DEFAULT_TOP_BAR_BUTTON_IMAGE_UNFOCUSED_COLOUR
      }),
      .Callbacks = std::optional<ButtonCallbacks>(std::in_place, ButtonCallbacks{
        .OnPressCallback = nullptr,
        .OnReleaseCallback = [Window = Renderer->GetWindow()]() {
          if (glfwGetWindowAttrib(Window, GLFW_MAXIMIZED)) {
            // Un-maximise window if already maximised
            glfwRestoreWindow(Window);
          } else {
            // Maximise window if already un-maximised
            glfwMaximizeWindow(Window);
          }
        },
        .OnHoverEnterCallback = nullptr,
        .OnHoverExitCallback = nullptr
      })
    };
  }

  ButtonSettings DefaultMinimiseButtonSettings() {
    return {
      .Size = Vector2D{ DEFAULT_TOP_BAR_BUTTON_WIDTH, DEFAULT_TOP_BAR_HEIGHT },
      .ButtonColours = {
        .UnfocusedColour = std::optional<Colour>(std::in_place, DEFAULT_TOP_BAR_BUTTON_COLOUR),
        .FocusedColour = std::optional<Colour>(std::in_place, DEFAULT_MINIMISE_BUTTON_FOCUSED_COLOUR)
      },
      ._ImageSettings = std::optional<ImageSettings>(std::in_place, ImageSettings{
        .ImageIndex = Renderer->GetImageIndexFromName("minimiseButtonImage"),
        .ImageSize = DEFAULT_TOP_BAR_BUTTON_IMAGE_SIZE,
        .ImageColour = DEFAULT_TOP_BAR_BUTTON_IMAGE_UNFOCUSED_COLOUR
      }),
      .Callbacks = std::optional<ButtonCallbacks>(std::in_place, ButtonCallbacks{
        .OnPressCallback = nullptr,
        .OnReleaseCallback = [=]() { glfwIconifyWindow(Renderer->GetWindow()); },
        .OnHoverEnterCallback = nullptr,
        .OnHoverExitCallback = nullptr
      })
    };
  }

  void CreateTopBarButton(UI::ECS::Entity& TopBarEntity, const ButtonSettings& Settings, const TopBarButtonAlignment& Alignment) {
    Vector2D buttonPosition{
      0.0f, UNUSED_PROPERTY
    };

    float buttonHalfWidth = Settings.Size.x / 2;

    switch (Alignment) {
    case TopBarButtonAlignment::LEFT:
      buttonPosition.x += buttonHalfWidth;
      break;
    case TopBarButtonAlignment::RIGHT:
      buttonPosition.x -= buttonHalfWidth;
      break;
    }

    UI::ECS::IRenderable* topBarRenderable = TopBarEntity.GetRenderableComponent();
    TopBarEntity.AddChild(CreateButton(false, Settings, buttonPosition, (topBarRenderable ? topBarRenderable->GetDrawDepth() : -1)));
    auto button = TopBarEntity.GetLastChild();
    button->GetComponent<UI::ECS::Transform>()->SetAnchorPreset(
      Alignment == TopBarButtonAlignment::LEFT
        ? UI::AnchorPresets::CENTER_LEFT
        : UI::AnchorPresets::CENTER_RIGHT
    );

    auto* buttonComponent = button->GetComponent<UI::ECS::Button>();

    if (buttonComponent && Settings.Callbacks.has_value()) {
      auto callbacks = Settings.Callbacks.value();
      if (!callbacks.OnHoverEnterCallback) {
        auto focusedColour = Settings.ButtonColours.UnfocusedColour.value_or(DEFAULT_TOP_BAR_BUTTON_COLOUR);
        buttonComponent->SetOnHoverEnter(DEFAULT_BUTTON_HOVER_LAMBDA(button, focusedColour, DEFAULT_TOP_BAR_BUTTON_IMAGE_FOCUSED_COLOUR));
      }

      if (!callbacks.OnHoverExitCallback) {
        auto unfocusedColour = Settings.ButtonColours.UnfocusedColour.value_or(DEFAULT_TOP_BAR_BUTTON_COLOUR);
        buttonComponent->SetOnHoverExit(DEFAULT_BUTTON_HOVER_LAMBDA(button, unfocusedColour, DEFAULT_TOP_BAR_BUTTON_IMAGE_UNFOCUSED_COLOUR));
      }
    }
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
      .MaximiseButton = DefaultMaximiseButtonSettings(),
      .MinimiseButton = DefaultMinimiseButtonSettings(),
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

std::shared_ptr<UI::ECS::Entity> UIFactory::CreateButton(bool AddToTree, const ButtonSettings& Settings, const Vector2D& Position, int ZIndex) {
  std::shared_ptr<UI::ECS::Entity> button = (AddToTree)
  ? EntityManager->AddEntity(UI::ECS::Entity(Settings.Size, Position)), EntityManager->GetLastEntity() // Add entity then get ptr to it
  : std::make_shared<UI::ECS::Entity>(Settings.Size, Position);

  button->AddComponent<UI::ECS::Image>(Settings.ButtonColours.UnfocusedColour.value_or(DEFAULT_TOP_BAR_BUTTON_COLOUR));

  if (Settings._ImageSettings.has_value()) {
    auto imageSettings = Settings._ImageSettings.value();
    button->AddChild(UI::ECS::Entity(imageSettings.ImageSize, Vector2D{ 0.0f, 0.0f }));
    auto image = button->GetLastChild();
    image->AddComponent<UI::ECS::Image>(imageSettings.ImageColour, imageSettings.ImageIndex);
    auto* parentRenderable = button->GetRenderableComponent();
    if (parentRenderable) {
      image->GetComponent<UI::ECS::Image>()->SetDrawDepth(parentRenderable->GetDrawDepth() + 1);
    }
  }

  button->AddComponent<UI::ECS::Button>();

  if (Settings.Callbacks.has_value()) {
    auto callbacks = Settings.Callbacks.value();
    auto* buttonComponent = button->GetComponent<UI::ECS::Button>();
    if (callbacks.OnPressCallback)
      buttonComponent->SetOnPress(callbacks.OnPressCallback);

    if (callbacks.OnReleaseCallback)
      buttonComponent->SetOnRelease(callbacks.OnReleaseCallback);

    if (callbacks.OnHoverEnterCallback)
      buttonComponent->SetOnHoverEnter(callbacks.OnHoverEnterCallback);

    if (callbacks.OnHoverExitCallback)
      buttonComponent->SetOnHoverExit(callbacks.OnHoverExitCallback);
  }

  EventManager->RegisterEventListener(button.get(), EventSystem::EventType::Mouse);

  return button;
}

UI::ECS::Entity& UIFactory::CreateTopBar(const UIFactory::TopBarSettings& Settings) {
  // Create titlebar
  UI::ECS::Entity& topBar = EntityManager->AddEntity(Vector2D({UNUSED_PROPERTY, Settings.BarHeight}),
                                                     Vector2D({UNUSED_PROPERTY, Settings.BarHeight / 2.0f}));

  topBar.GetComponent<UI::ECS::Transform>()->SetAnchorPreset(UI::AnchorPresets::STRETCH_TOP);
  topBar.AddComponent<UI::ECS::Image>(Settings.BackgroundColour);

  if (Settings.ButtonSettings.CloseButton.has_value()) {
    CreateTopBarButton(topBar, Settings.ButtonSettings.CloseButton.value(), Settings.ButtonSettings.Alignment);
  }

  if (Settings.ButtonSettings.MaximiseButton.has_value()) {
    CreateTopBarButton(topBar, Settings.ButtonSettings.MaximiseButton.value(), Settings.ButtonSettings.Alignment);
    float offsetAmount = abs(topBar.GetLastChild()->GetComponent<UI::ECS::Transform>()->GetLocalPosition().x);
    if (Settings.ButtonSettings.CloseButton.has_value())
      offsetAmount += Settings.ButtonSettings.CloseButton.value().Size.x;
    
    topBar.GetLastChild()->GetComponent<UI::ECS::Transform>()->SetLocalPosition({
      (Settings.ButtonSettings.Alignment == TopBarButtonAlignment::LEFT
        ? offsetAmount
        : -offsetAmount
      ), 0.0f
    });
  }

  if (Settings.ButtonSettings.MinimiseButton.has_value()) {
    CreateTopBarButton(topBar, Settings.ButtonSettings.MinimiseButton.value(), Settings.ButtonSettings.Alignment);
    float offsetAmount = abs(topBar.GetLastChild()->GetComponent<UI::ECS::Transform>()->GetLocalPosition().x);
    if (Settings.ButtonSettings.CloseButton.has_value())
      offsetAmount += Settings.ButtonSettings.CloseButton.value().Size.x;
    if (Settings.ButtonSettings.MaximiseButton.has_value())
      offsetAmount += Settings.ButtonSettings.MaximiseButton.value().Size.x;
    
    topBar.GetLastChild()->GetComponent<UI::ECS::Transform>()->SetLocalPosition({
      (Settings.ButtonSettings.Alignment == TopBarButtonAlignment::LEFT
        ? offsetAmount
        : -offsetAmount
      ), 0.0f
    });
  }

  /*
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
  */

  return topBar;
}

} // namespace CustomIDE
