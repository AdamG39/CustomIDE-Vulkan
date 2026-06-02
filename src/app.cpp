#include "app.hpp"
#include "io/io.hpp"
#include "helpers/errors/errors.hpp"
#include "renderer/vulkanCore.hpp"
#include <GLFW/glfw3.h>
#include <chrono>

/*\ ---- TODO: ----
 *  [ ] Dim or change colour of title bar when window is unfocused
 *  [ ] Create a dropdown UI element
\*/

bool framebufferResized = false;
int framebufferWidth = 0;
int framebufferHeight = 0;

bool maximisedState = true;
bool resizeHover = false;
bool resizing = false;

ResizeSide resizeSide;

void CustomIDEApplication::InitApplication() {
  std::vector<std::shared_ptr<Image>> loadedImages;
  ReadImageFile("../CustomIDE icon.ico", loadedImages);
  std::vector<GLFWimage> appIcon;
  appIcon.reserve(loadedImages.size());
  for (size_t i = 0; i < loadedImages.size(); i++) {
    GLFWimage image = GLFWimage();
    image.width = loadedImages[i]->width;
    image.height = loadedImages[i]->height;
    image.pixels = loadedImages[i]->pixels;
    appIcon.push_back(image);
  }

  m_renderer = std::make_shared<VulkanRenderer>(m_applicationName, THEME_DARK_COLOUR_0.ConvertSRGBToLinear());
  m_entityManager = std::make_unique<EntityManager>(m_renderer);
  m_eventManager = std::make_unique<EventManager>();

  glfwGetFramebufferSize(m_renderer->GetWindow(), &m_windowWidth, &m_windowHeight);

  float xScale, yScale;
  glfwGetWindowContentScale(m_renderer->GetWindow(), &xScale, &yScale);
  m_renderer->SetWindowContentScale(xScale, yScale);

  glfwSetWindowIcon(m_renderer->GetWindow(), appIcon.size(), appIcon.data());
  framebufferWidth = m_windowWidth;
  framebufferHeight = m_windowHeight;

  CreateUIElements();

  m_cursorObjects["DEFAULT"] = nullptr;
  m_cursorObjects["HRESIZE"] = glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR);
  m_cursorObjects["VRESIZE"] = glfwCreateStandardCursor(GLFW_VRESIZE_CURSOR);
  if (m_cursorObjects["HRESIZE"] == nullptr ||
      m_cursorObjects["VRESIZE"] == nullptr) {
    ExitWithError("Failed to create cursor objects!", -1);
  }
}

void CustomIDEApplication::RunApplication() {
  while (!glfwWindowShouldClose(m_renderer->GetWindow())) {
    glfwWaitEventsTimeout(0.5f);

    /*
    HandleResizing();

    UpdateCursorState();

    HandleDragging();
    */

    m_eventManager->HandleEvents();

    m_entityManager->RenderTree(m_windowWidth, m_windowHeight);

    m_renderer->DrawFrame();
  }

  vkDeviceWaitIdle(m_renderer->GetDevice());
}

void CustomIDEApplication::EndApplication() {
  // Currently nothing to do on end
}

std::weak_ptr<VulkanRenderer> CustomIDEApplication::GetRenderer() const {
  return m_renderer;
}

std::weak_ptr<EntityManager> CustomIDEApplication::GetEntityManager() const {
  return m_entityManager;
}

std::weak_ptr<EventManager> CustomIDEApplication::GetEventManager() const {
  return m_eventManager;
}

GLFWcursor* CustomIDEApplication::GetCursorObject(std::string Index) {
  return m_cursorObjects[Index];
}

void CustomIDEApplication::SetCursorState(int State) {
  m_cursorState = State;
}

void CustomIDEApplication::HandleResizing() {
  if (resizing) {
    double xpos, ypos;
    int xwin, ywin;
    int xsize, ysize;
    GLFWwindow* window = m_renderer->GetWindow();
    glfwGetCursorPos(window, &xpos, &ypos);
    glfwGetWindowPos(window, &xwin, &ywin);
    glfwGetWindowSize(window, &xsize, &ysize);
    int x = (xpos < MIN_WIDTH) ? MIN_WIDTH : static_cast<int>(xpos);
    int y = (ypos < MIN_HEIGHT) ? MIN_HEIGHT : static_cast<int>(ypos);
    switch (resizeSide) {
      case ResizeSide::Left:
        // Resize to still be same width
        x = ((xsize - static_cast<int>(xpos)) < MIN_WIDTH) ? MIN_WIDTH : xsize - static_cast<int>(xpos);
        if (x > MIN_WIDTH) glfwSetWindowPos(window, xwin + static_cast<int>(xpos), ywin);
        glfwSetWindowSize(window, x, ysize);
        break;
      case ResizeSide::Right:
        glfwSetWindowSize(window, x, m_windowHeight);
        break;
      case ResizeSide::Top:
        y = ((ysize - static_cast<int>(ypos)) < MIN_HEIGHT) ? MIN_HEIGHT : ysize - static_cast<int>(ypos);
        if (y > MIN_HEIGHT) glfwSetWindowPos(window, xwin, ywin + static_cast<int>(ypos));
        glfwSetWindowSize(window, xsize, y);
        break;
      case ResizeSide::Bottom:
        glfwSetWindowSize(window, m_windowWidth, y);
        break;
    }
  }

  if (framebufferResized) {
    framebufferResized = false;
    while (framebufferWidth == 0 || framebufferHeight == 0) {
      glfwWaitEvents();
      glfwGetFramebufferSize(m_renderer->GetWindow(), &framebufferWidth, &framebufferHeight);
    }
    m_renderer->RecreateSwapChain();

    m_windowWidth = framebufferWidth;
    m_windowHeight = framebufferHeight;

    //m_root->RecalculateUILayout(framebufferWidth, framebufferHeight);
  }
}

void CustomIDEApplication::UpdateCursorState() {
  if (resizeHover && m_cursorState == CURSOR_STATE_DEFAULT) {
    switch (resizeSide) {
      case ResizeSide::Left:
      case ResizeSide::Right:
        SetCursorState(CURSOR_STATE_HRESIZE);
        glfwSetCursor(m_renderer->GetWindow(), GetCursorObject("HRESIZE"));
        break;
      case ResizeSide::Top:
      case ResizeSide::Bottom:
        SetCursorState(CURSOR_STATE_VRESIZE);
        glfwSetCursor(m_renderer->GetWindow(), GetCursorObject("VRESIZE"));
        break;
    }
  }

  if (!resizeHover && m_cursorState != CURSOR_STATE_DEFAULT) {
    SetCursorState(CURSOR_STATE_DEFAULT);
    glfwSetCursor(m_renderer->GetWindow(), GetCursorObject("DEFAULT"));
  }
}

// FIXME Transition from old uimanager to new ecs manager
/*
void CustomIDEApplication::HandleDragging() {
  if ((m_root->EventFlags & EVENT_FLAG_DRAGGING) != 0) {
    double xpos, ypos;
    GLFWwindow* window = m_renderer->GetWindow();
    glfwGetCursorPos(window, &xpos, &ypos);
    int newXPos, newYPos;
    glfwGetWindowPos(window, &newXPos, &newYPos);
    newXPos += (int)xpos - m_root->MousePressPosition.x;
    newYPos += (int)ypos - m_root->MousePressPosition.y;
    glfwSetWindowPos(window, newXPos, newYPos);
  }
}*/

void CustomIDEApplication::CreateUIElements() {
  Entity& titleBar = m_entityManager->AddEntity(Vector2<UISize<float>>({1.0f, SizeMode::Proportional}, {40.0f}),
                                                Vector2<UISize<float>>({0.0f}, {20.0f}));

  titleBar.GetComponent<Transform>()->SetAnchor(UIAnchorType::Top);

  titleBar.AddComponent<UIImage>(THEME_DARK_COLOUR_1);

  Entity& closeButton = m_entityManager->AddEntity(Vector2<UISize<float>>({50.f}, {40.f}),
                                                   Vector2<UISize<float>>({-25.f}, {20.f}));

  closeButton.GetComponent<Transform>()->SetAnchor(UIAnchorType::TopRight);

  closeButton.AddComponent<UIImage>(Colour(0xe81123, 1.f));
  closeButton.AddComponent<Button>();
  closeButton.GetComponent<Button>()->SetOnRelease(&glfwSetWindowShouldClose, m_renderer->GetWindow(), GLFW_TRUE);

  closeButton.AddChild(Entity(Vector2<UISize<float>>({10.f}, {10.f}),
                              Vector2<UISize<float>>({0.f, SizeMode::Proportional}, {0.f, SizeMode::Proportional})));

  std::shared_ptr closeButtonCross = closeButton.GetChild(0);

  closeButtonCross->AddComponent<UIImage>(COLOUR_WHITE, 2);

  Entity& titleLabel = m_entityManager->AddEntity(Vector2<UISize<float>>({600.f, 40.f}),
                                                  Vector2<UISize<float>>({20.f, 20.f}));

  titleLabel.GetComponent<Transform>()->SetAnchor(UIAnchorType::TopLeft);

  Font font = CreateFont("../assets/unscii-alt-font-16.png", Colour(0xD4D6DE, 1.f));

  titleLabel.AddComponent<Label>(font, "CustomIDE | File | Edit");

  Entity& textBoxBackground = m_entityManager->AddEntity(Vector2<UISize<float>>({0.99f, SizeMode::Proportional},
                                                         {0.95f, SizeMode::Proportional}),
                                                         Vector2<UISize<float>>({0.f, 20.f}));

  textBoxBackground.AddComponent<UIImage>(THEME_DARK_COLOUR_1);

  textBoxBackground.AddChild(Entity(Vector2<UISize<float>>({600.f, 600.f}),
                                    Vector2<UISize<float>>({30.f, 80.f})));

  std::shared_ptr textBox = textBoxBackground.GetChild(0);

  textBox->GetComponent<Transform>()->SetAnchor(UIAnchorType::TopLeft);
  
  textBox->AddComponent<TextBox>(font, "../src/app.cpp");

  textBox->AddComponent<Mask>(ClipRect{.clippingEnabled = true,
      .rect = {.xOffset = 1280, .yOffset = 716, .width = 2534, .height = 1322}});
}

bool CursorAtHorizontalBorder(double xpos, ResizeSide& side) {
  if (xpos >= -BORDER_THICKNESS && xpos <= BORDER_THICKNESS) {
    side = ResizeSide::Left;
    return true;
  } else if(xpos >= framebufferWidth - BORDER_THICKNESS && xpos <= framebufferWidth + BORDER_THICKNESS) {
    side = ResizeSide::Right;
    return true;
  } else return false;
}

bool CursorAtVerticalBorder(double ypos, ResizeSide& side) {
  if (ypos >= -BORDER_THICKNESS && ypos <= BORDER_THICKNESS) {
    side = ResizeSide::Top;
    return true;
  } else if (ypos >= framebufferHeight - BORDER_THICKNESS && ypos <= framebufferHeight + BORDER_THICKNESS) {
    side = ResizeSide::Bottom;
    return true;
  } else return false;
}

void CloseWindowCallback(GLFWwindow* Window){
  glfwDestroyWindow(Window);
}

void MouseButtonCallback(GLFWwindow* Window, int Button, int Action, int Mods) { 
  double xPos, yPos;
  glfwGetCursorPos(Window, &xPos, &yPos);

  EventInfo info {
    .Manager = CustomIDEApplication::GetInstance()->GetEntityManager().lock(),
    .MouseInfo = {
      .Position = Vector2<float>((float)xPos, (float)yPos),
      .Button = Button,
      .Action = Action,
      .Modifications = Mods
    },
  };
  CustomIDEApplication::GetInstance()->GetEventManager().lock()->AddEvent(EventType::Mouse, &info);
}

void FramebufferResizeCallback(GLFWwindow* Window, int Width, int Height) {
  framebufferResized = true;
  framebufferWidth = Width;
  framebufferHeight = Height;
}

void CursorPositionCallback(GLFWwindow* Window, double xpos, double ypos) {
  // Check if mouse is at window border
  // Change cursor to horizonal/vertical resizers
  // Start resizing if at the border and left click is pressed
  // Stop resizing if left click is released
  if (maximisedState) return; // If maximised then resizing doesnt make sense so just return
  if (CursorAtHorizontalBorder(xpos, resizeSide)) {
    resizeHover = true;
  } else if (CursorAtVerticalBorder(ypos, resizeSide)) {
    resizeHover = true;
  } else {
    resizeHover = false;
  }
}

// FIXME transition from old uimanager to new ecs manager
/*
void ToggleMaximiseCallback(GLFWwindow* Window) {
  if (glfwGetWindowAttrib(Window, GLFW_MAXIMIZED)) {
    // Un-maximise window if already maximised
    glfwRestoreWindow(Window);
    maximisedState = false;
    if (CustomIDEApplication::s_instance) {
      CustomIDEApplication::s_instance->GetUIManager()->AddEvent(std::make_shared<UIEvent>(UIEvent(UIEventType::WINDOW_RESTORE)));
    }
  } else {
    // Maximise window if already un-maximised
    glfwMaximizeWindow(Window);
    maximisedState = true;
    if (CustomIDEApplication::s_instance) {
      CustomIDEApplication::s_instance->GetUIManager()->AddEvent(std::make_shared<UIEvent>(UIEvent(UIEventType::WINDOW_MAXIMISE)));
    }
  }
}*/

void MinimiseCallback(GLFWwindow* Window) {
  glfwIconifyWindow(Window);
}

void KeyCallback(GLFWwindow* Window, int Key, int Scancode, int Action, int Mods) {
  EventInfo info {
    .Manager = CustomIDEApplication::GetInstance()->GetEntityManager().lock(),
    .Window = Window,
    .KeyboardInfo {
      .Key = Key,
      .Scancode = Scancode,
      .Action = Action,
      .Modifications = Mods
    },
  };

  CustomIDEApplication::GetInstance()->GetEventManager().lock()->AddEvent(EventType::Keyboard, &info);
}

void CharacterCallback(GLFWwindow *Window, unsigned int Codepoint) {
  EventInfo info {
    .Manager = CustomIDEApplication::GetInstance()->GetEntityManager().lock(),
    .Window = Window,
    .CharacterInfo {
      .Codepoint = Codepoint
    },
  };

  CustomIDEApplication::GetInstance()->GetEventManager().lock()->AddEvent(EventType::Character, &info);
}

Font CreateFont(const std::string& Filepath, const Colour& FontColour) {
  Font font;

  font.colour = FontColour;

  auto renderer = CustomIDEApplication::GetInstance()->GetRenderer().lock();
  renderer->LoadImage(Filepath, true);
  std::string fileName = GetFileNameFromPath(Filepath);
  Vector2<int> dimensions = renderer->GetImageDimensions(renderer->GetImageIndexFromName(fileName));
  font.size = { dimensions.x / 64, dimensions.y / 2 };
  font.familyName = fileName;

  return font;
}

