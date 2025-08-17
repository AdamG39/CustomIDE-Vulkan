#include "app.hpp"
#include "io.hpp"
#include <GLFW/glfw3.h>

/*\ ---- TODO: ----
 *  [X] Have a basic vulkan implementation to draw a flat colour for the window
 *  [X] Draw a rectangle to represent new title bar
 *  [-] Render quads for custom buttons with textures
 *  [X] Ensure new title bar doesnt interfere with the rest of the windows ui
 *  [X] Implement window dragging
 *  [X] Implement window resizing
 *  [X] Implement minimise, maximise and close buttons
 *  [ ] Dim or change colour of title bar when window is unfocused
 *  [ ] Update button class to contain additional functions such as OnHover, OnRelease etc
 *  [ ] Create a dropdown UI element
 *  [ ] Allow for text rendering
 *  [ ] Create a text field UI element
\*/

const Colour<float> DEFAULT_BACKGROUND_COLOUR = Colour(0x3B1C32, 1.0f);
const int32_t BORDER_THICKNESS = 10;

bool framebufferResized = false;
int framebufferWidth = 0;
int framebufferHeight = 0;

bool maximisedState = true;
bool resizeHover = false;
bool resizing = false;

ResizeSide resizeSide;

void CustomIDEApplication::StartApplication() {
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
  CreateRenderer(ApplicationName);
  glfwSetWindowIcon(m_renderer->GetWindow(), appIcon.size(), appIcon.data());
  framebufferWidth = m_windowWidth;
  framebufferHeight = m_windowHeight;

  m_root = new UIManager<float, float>();
  m_root->BindRenderer(*m_renderer);
  m_root->WindowFlags ^= WINDOW_FLAG_MAXIMISED;

  CreateUIElements();

  m_cursorObjects["DEFAULT"] = nullptr;
  m_cursorObjects["HRESIZE"] = glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR);
  m_cursorObjects["VRESIZE"] = glfwCreateStandardCursor(GLFW_VRESIZE_CURSOR);
  if (m_cursorObjects["HRESIZE"] == nullptr ||
      m_cursorObjects["VRESIZE"] == nullptr) {
    ExitWithError("Failed to create cursor objects!", -1);
  }
}

void CustomIDEApplication::MainLoop() {
  while (!glfwWindowShouldClose(m_renderer->GetWindow())) {
    glfwWaitEventsTimeout(0.5f);

    m_root->HandleEvents();

    m_root->RenderAll();

    HandleResizing();

    UpdateCursorState();

    HandleDragging();

    m_renderer->DrawFrame();
  }

  vkDeviceWaitIdle(m_renderer->GetDevice());
}

void CustomIDEApplication::EndApplication() {
  DestroyRenderer();
  delete m_root;
}

void CustomIDEApplication::CreateRenderer(std::string AppName) {
  m_renderer = new VulkanRenderer(AppName, DEFAULT_BACKGROUND_COLOUR.ConvertSRGBToLinear());

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

    m_root->RecalculateUILayout(framebufferWidth, framebufferHeight);
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
}

void CustomIDEApplication::CreateUIElements() {
  Panel background = Panel(Vector2<UISize<float>>({1.0f, SizeMode::Proportional}, {1.0f, SizeMode::Proportional}),
                           Vector2<UISize<float>>({0.0f}, {0.0f}),
                           Colour(0x3B1C32, 1.0f));

  Panel titleBar = Panel(Vector2<UISize<float>>({1.0f, SizeMode::Proportional}, {40.0f}),
                         Vector2<UISize<float>>({0.0f}, {0.0f}),
                         Colour(0x1A1A1D, 1.0f));

  titleBar.SetAnchor(UIAnchor(Vector2<UISize<float>>({0.0f}, {20.0f}), UIAnchorType::Top));

  PanelButton exitButton = PanelButton(Vector2<UISize<float>>({50.0f}, {40.0f}),
                                       Vector2<UISize<float>>({0.0f}, {0.0f}),
                                       Colour(0xFF0000, 1.0f),
                                       CloseWindowCallback, m_renderer->GetWindow());

  exitButton.SetAnchor(UIAnchor(Vector2<UISize<float>>({-25.0f}, {20.0f}), UIAnchorType::TopRight));

  PanelButton maximiseButton = PanelButton(Vector2<UISize<float>>({50.0f}, {40.0f}),
                                           Vector2<UISize<float>>({-50.0f,}, {0.0f}),
                                           Colour(0x00FF00, 1.0f),
                                           ToggleMaximiseCallback, m_renderer->GetWindow());

  maximiseButton.SetAnchor(UIAnchor(Vector2<UISize<float>>({-25.0f}, {20.0f}), UIAnchorType::TopRight));

  PanelButton minimiseButton = PanelButton(Vector2<UISize<float>>({50.0f}, {40.0f}),
                                           Vector2<UISize<float>>({-100.0f}, {0.0f}),
                                           Colour(0x0000FF, 1.0f),
                                           MinimiseCallback, m_renderer->GetWindow());

  minimiseButton.SetAnchor(UIAnchor(Vector2<UISize<float>>({-25.0f}, {20.0f}), UIAnchorType::TopRight));

  titleBar.GetGeometry().SetZIndex(1);
  exitButton.GetGeometry().SetZIndex(1);
  maximiseButton.GetGeometry().SetZIndex(1);
  minimiseButton.GetGeometry().SetZIndex(1);

  m_root->AddElement(background);
  m_root->AddElement(titleBar);
  m_root->AddElement(exitButton);
  m_root->AddElement(maximiseButton);
  m_root->AddElement(minimiseButton);

  m_root->RecalculateUILayout(framebufferWidth, framebufferHeight);
}

bool CursorAtHorizontalBorder(double xpos, ResizeSide* side) {
  if (xpos >= -BORDER_THICKNESS && xpos <= BORDER_THICKNESS) {
    *side = ResizeSide::Left;
    return true;
  } else if(xpos >= framebufferWidth - BORDER_THICKNESS && xpos <= framebufferWidth + BORDER_THICKNESS) {
    *side = ResizeSide::Right;
    return true;
  } else return false;
}

bool CursorAtVerticalBorder(double ypos, ResizeSide* side) {
  if (ypos >= -BORDER_THICKNESS && ypos <= BORDER_THICKNESS) {
    *side = ResizeSide::Top;
    return true;
  } else if (ypos >= framebufferHeight - BORDER_THICKNESS && ypos <= framebufferHeight + BORDER_THICKNESS) {
    *side = ResizeSide::Bottom;
    return true;
  } else return false;
}

void CloseWindowCallback(GLFWwindow* Window){
  glfwDestroyWindow(Window);
}

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
  if (CursorAtHorizontalBorder(xpos, &resizeSide)) {
    resizeHover = true;
  } else if (CursorAtVerticalBorder(ypos, &resizeSide)) {
    resizeHover = true;
  } else {
    resizeHover = false;
  }
}

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
}

void MinimiseCallback(GLFWwindow* Window) {
  glfwIconifyWindow(Window);
}

