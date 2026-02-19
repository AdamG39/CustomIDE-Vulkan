#include "app.hpp"
#include "io/io.hpp"
#include "helpers/errors/errors.hpp"
#include "renderer/vulkanCore.hpp"
#include <GLFW/glfw3.h>

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

  CreateRenderer(m_applicationName);
  glfwSetWindowIcon(m_renderer->GetWindow(), appIcon.size(), appIcon.data());
  framebufferWidth = m_windowWidth;
  framebufferHeight = m_windowHeight;

  m_tree = new EntityManager(*m_renderer);
  m_eventManager = new EventManager();
  //m_root->WindowFlags ^= WINDOW_FLAG_MAXIMISED;

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
    m_root->HandleEvents();

    m_root->RenderAll();

    HandleResizing();

    UpdateCursorState();

    HandleDragging();
    */

    m_eventManager->HandleEvents();

    m_tree->RenderTree();

    m_renderer->DrawFrame();
  }

  vkDeviceWaitIdle(m_renderer->GetDevice());
}

void CustomIDEApplication::EndApplication() {
  DestroyRenderer();
}

void CustomIDEApplication::CreateRenderer(std::string AppName) {
  m_renderer = new VulkanRenderer(AppName, THEME_DARK_COLOUR_0.ConvertSRGBToLinear());

  glfwGetFramebufferSize(m_renderer->GetWindow(), &m_windowWidth, &m_windowHeight);
};

void CustomIDEApplication::DestroyRenderer() {
  delete m_renderer;
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
  Entity& titleBar = m_tree->AddEntity(Vector2<UISize<float>>({1.0f, SizeMode::Proportional}, {40.0f}),
                                       Vector2<UISize<float>>({0.0f}, {20.0f}));

  titleBar.GetComponent<Transform>()->SetAnchor(UIAnchorType::Top);

  titleBar.AddComponent<StaticColour>(THEME_DARK_COLOUR_1);

  Entity& closeButton = m_tree->AddEntity(Vector2<UISize<float>>({50.f}, {40.f}),
                                          Vector2<UISize<float>>({-25.f}, {20.f}));

  closeButton.GetComponent<Transform>()->SetAnchor(UIAnchorType::TopRight);

  closeButton.AddComponent<StaticColour>(COLOUR_RED);
  closeButton.AddComponent<Button>();
  closeButton.GetComponent<Button>()->SetOnRelease(&glfwSetWindowShouldClose, m_renderer->GetWindow(), GLFW_TRUE);

  closeButton.AddChild(Entity(Vector2<UISize<float>>({40.f}, {40.f}),
                              Vector2<UISize<float>>({0.f, SizeMode::Proportional}, {0.f, SizeMode::Proportional})));

  std::shared_ptr closeButtonCross = closeButton.GetChild(0);

  closeButtonCross->AddComponent<Texture>(2);


  Entity& titleLabel = m_tree->AddEntity(Vector2<UISize<float>>({600.f, 40.f}),
                                         Vector2<UISize<float>>({20.f, 20.f}));

  titleLabel.GetComponent<Transform>()->SetAnchor(UIAnchorType::TopLeft);

  Font font = CreateFont("../assets/unscii-alt-font-16.png", COLOUR_WHITE);

  titleLabel.AddComponent<Label>(font, "CustomIDE | File | Edit");
  
  Entity& textBox = m_tree->AddEntity(Vector2<UISize<float>>({600.f, 600.f}),
                                      Vector2<UISize<float>>({20.f, 60.f}));

  textBox.GetComponent<Transform>()->SetAnchor(UIAnchorType::TopLeft);
  
  textBox.AddComponent<TextBox>(font, "some test text");
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
    .EntityManager = CustomIDEApplication::GetInstance()->GetEntityManager(),
    .MouseInfo = {
      .Position = Vector2<float>((float)xPos, (float)yPos),
      .Button = Button,
      .Action = Action,
      .Modifications = Mods
    },
  };
  CustomIDEApplication::GetInstance()->GetEventManager()->AddEvent(EventType::Mouse, &info);
}

// FIXME transition from old uimanager to new ecs manager
/*
void MouseButtonCallback(GLFWwindow* Window, int Button, int Action, int Mods) {
  if (Button != GLFW_MOUSE_BUTTON_LEFT) return;

  if (Action == GLFW_RELEASE) {
    if (resizing) {
      resizing = false;
      return;
    }

    if (!resizeHover) {
      // Left mouse pressed
      double xPos, yPos;
      glfwGetCursorPos(Window, &xPos, &yPos);
      if (CustomIDEApplication::s_instance) {
        CustomIDEApplication::s_instance->GetUIManager()->AddEvent(std::make_shared<UIMouseEvent>(
                                                      UIMouseEvent(Vector2((float)xPos, (float)yPos),
                                                                   Button, Action, Mods)));
        return;
      }
    }
  }

  else if (Action == GLFW_PRESS) {
    if (resizeHover) {
      resizing = true;
      double xPos, yPos;
      glfwGetCursorPos(Window, &xPos, &yPos);
      if (CustomIDEApplication::s_instance) {
        auto event = UIMouseEvent(Vector2((float)xPos, (float)yPos), Button, Action, Mods);
        event.SetEventType(UIEventType::WINDOW_RESIZE);
        CustomIDEApplication::s_instance->GetUIManager()->AddEvent(std::make_shared<UIMouseEvent>(event));
        return;
      }
    }

    if (!resizeHover) {
      // Left mouse pressed
      double xPos, yPos;
      glfwGetCursorPos(Window, &xPos, &yPos);
      if (CustomIDEApplication::s_instance) {
        CustomIDEApplication::s_instance->GetUIManager()->AddEvent(std::make_shared<UIMouseEvent>(
                                                      UIMouseEvent(Vector2((float)xPos, (float)yPos),
                                                                   Button, Action, Mods)));
        return;
      }
    }
  }
}*/

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
    .EntityManager = CustomIDEApplication::GetInstance()->GetEntityManager(),
    .Window = Window,
    .KeyboardInfo {
      .Key = Key,
      .Scancode = Scancode,
      .Action = Action,
      .Modifications = Mods
    },
  };

  if (glfwGetKeyName(Key, 0) == nullptr)
    CustomIDEApplication::GetInstance()->GetEventManager()->AddEvent(EventType::Keyboard, &info);
}

void CharacterCallback(GLFWwindow *Window, unsigned int Codepoint) {
  EventInfo info {
    .EntityManager = CustomIDEApplication::GetInstance()->GetEntityManager(),
    .Window = Window,
    .CharacterInfo {
      .Codepoint = Codepoint
    },
  };

  CustomIDEApplication::GetInstance()->GetEventManager()->AddEvent(EventType::Character, &info);
}

Font CreateFont(const std::string& Filepath, const Colour<float>& FontColour) {
  Font font;

  font.colour = FontColour;

  auto renderer = CustomIDEApplication::GetInstance()->GetRenderer();
  renderer->LoadImage(Filepath, true);
  std::string fileName = GetFileNameFromPath(Filepath);
  Vector2<int> dimensions = renderer->GetImageDimensions(renderer->GetImageIndexFromName(fileName));
  font.size = { dimensions.x / 64, dimensions.y / 2 };
  font.familyName = fileName;

  return font;
}

