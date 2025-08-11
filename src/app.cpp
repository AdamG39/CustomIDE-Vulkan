#include "app.h"
#include <GLFW/glfw3.h>

const uint32_t BORDER_THICKNESS = 5;

bool framebufferResized = false;
int framebufferWidth = 0;
int framebufferHeight = 0;

bool maximisedState = true;
bool resizeHover = false;
bool resizing = false;
bool resizeDirection;

bool dragging = false;

void CustomIDEApplication::StartApplication() {
  CreateRenderer(ApplicationName);
  framebufferWidth = m_windowWidth;
  framebufferHeight = m_windowHeight;

  m_root = new UIManager<float, float>();
  m_root->BindRenderer(*m_renderer);

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

    if (resizeHover && m_cursorState == CURSOR_STATE_DEFAULT) {
      if (resizeDirection == RESIZE_HORIZONTAL) {
        SetCursorState(CURSOR_STATE_HRESIZE);
        glfwSetCursor(m_renderer->GetWindow(), GetCursorObject("HRESIZE"));
      } else {
        SetCursorState(CURSOR_STATE_VRESIZE);
        glfwSetCursor(m_renderer->GetWindow(), GetCursorObject("VRESIZE"));
      }
    }

    if (!resizeHover && m_cursorState != CURSOR_STATE_DEFAULT) {
      SetCursorState(CURSOR_STATE_DEFAULT);
      glfwSetCursor(m_renderer->GetWindow(), GetCursorObject("DEFAULT"));
    }

    if (resizing) {
      double xpos, ypos;
      GLFWwindow* window = m_renderer->GetWindow();
      glfwGetCursorPos(window, &xpos, &ypos);
      if (resizeDirection == RESIZE_HORIZONTAL) {
        glfwSetWindowSize(window, static_cast<int>(xpos), m_windowHeight);
      } else {
        glfwSetWindowSize(window, m_windowWidth, static_cast<int>(ypos));
      }
    }

    m_renderer->DrawFrame();
  }

  vkDeviceWaitIdle(m_renderer->GetDevice());
}

void CustomIDEApplication::EndApplication() {
  DestroyRenderer();
  delete m_root;
}

void CustomIDEApplication::CreateRenderer(std::string AppName) {
  m_renderer = new VulkanRenderer(AppName);

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

void CustomIDEApplication::CreateUIElements() {
  // TODO: Implement anchoring so offsets can have more flexibility
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

bool CursorAtHorizontalBorder(double xpos) {
  return (xpos >= -BORDER_THICKNESS && xpos <= BORDER_THICKNESS ||
          xpos >= framebufferWidth - BORDER_THICKNESS && xpos <= framebufferWidth + BORDER_THICKNESS)
    ? true : false;
}

bool CursorAtVerticalBorder(double ypos) {
  return (ypos >= -BORDER_THICKNESS && ypos <= BORDER_THICKNESS ||
          ypos >= framebufferHeight - BORDER_THICKNESS && ypos <= framebufferHeight + BORDER_THICKNESS)
    ? true : false;
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

    if (dragging) {
      dragging = false;
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
  if (CursorAtHorizontalBorder(xpos)) {
    resizeHover = true;
    resizeDirection = RESIZE_HORIZONTAL;
  } else if (CursorAtVerticalBorder(ypos)) {
    resizeHover = true;
    resizeDirection = RESIZE_VERITCAL;
  } else {
    resizeHover = false;
  }
}

void ToggleMaximiseCallback(GLFWwindow* Window) {
  if (glfwGetWindowAttrib(Window, GLFW_MAXIMIZED)) {
    // Un-maximise window if already maximised
    glfwRestoreWindow(Window);
    maximisedState = false;
  } else {
    // Maximise window if already un-maximised
    glfwMaximizeWindow(Window);
    maximisedState = true;
  }
}

void MinimiseCallback(GLFWwindow* Window) {
  glfwIconifyWindow(Window);
}

