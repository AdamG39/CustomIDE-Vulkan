#include "app.h"
#include <GLFW/glfw3.h>

const uint32_t BORDER_THICKNESS = 5;

bool framebufferResized = false;
int framebufferWidth = 0;
int framebufferHeight = 0;

bool resizeHover = false;
bool resizing = false;
bool resizeDirection;

void CustomIDEApplication::StartApplication() {
  CreateRenderer(ApplicationName);
  framebufferWidth = m_windowWidth;
  framebufferHeight = m_windowHeight;

  m_root = new UIManager<int, float>();
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

      // FIXME: DO NOT REMOVE AND RECREATE ALL ELEMENTS
      // instead store if an element uses fixed or percentage size
      // then propogate through the list and update the ones that are percentage based
      m_root->RemoveAllElements();
      CreateUIElements();
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
      puts("resizing");
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
  Panel background = Panel("Background",
                           Vector2(m_windowWidth, m_windowHeight),
                           Vector2(m_windowWidth / 2, m_windowHeight / 2),
                           Colour(0x3B1C32, 1.0f));

  Panel titleBar = Panel("TitleBar",
                         Vector2(m_windowWidth, 40),
                         Vector2(m_windowWidth / 2, 20),
                         Colour(0x1A1A1D, 1.0f));

  Panel exitButtonPanel = Panel("ExitButtonPanel",
                    Vector2(50, 40),
                    Vector2(m_windowWidth - 25, 20),
                    Colour(0xFF0000, 1.0f));

  Panel maximiseButtonPanel = Panel("MaximiseButtonPanel",
                    Vector2(50, 40),
                    Vector2(m_windowWidth - 75, 20),
                    Colour(0x00FF00, 1.0f));

  Panel minimiseButtonPanel = Panel("MinimiseButtonPanel",
                    Vector2(50, 40),
                    Vector2(m_windowWidth - 125, 20),
                    Colour(0x0000FF, 1.0f));

  Button exitButton = Button<int, float>(Vector2(50, 40),
                                         Vector2(m_windowWidth - 25, 20),
                                         CloseWindowCallback, m_renderer->GetWindow());

  Button maximiseButton = Button<int, float>(Vector2(50, 40),
                                             Vector2(m_windowWidth - 75, 20),
                                             ToggleMaximiseCallback, m_renderer->GetWindow());

  Button minimiseButton = Button<int, float>(Vector2(50, 40),
                                             Vector2(m_windowWidth - 125, 20),
                                             MinimiseCallback, m_renderer->GetWindow());

  exitButtonPanel.AddChild(std::make_shared<Button<int, float>>(exitButton));
  maximiseButtonPanel.AddChild(std::make_shared<Button<int, float>>(maximiseButton));
  minimiseButtonPanel.AddChild(std::make_shared<Button<int, float>>(minimiseButton));

  titleBar.GetGeometry().SetZIndex(1);
  exitButtonPanel.GetGeometry().SetZIndex(1);
  maximiseButtonPanel.GetGeometry().SetZIndex(1);
  minimiseButtonPanel.GetGeometry().SetZIndex(1);

  m_root->AddElement(std::make_shared<Panel<int, float>>(background));
  m_root->AddElement(std::make_shared<Panel<int, float>>(titleBar));
  m_root->AddElement(std::make_shared<Panel<int, float>>(exitButtonPanel));
  m_root->AddElement(std::make_shared<Panel<int, float>>(maximiseButtonPanel));
  m_root->AddElement(std::make_shared<Panel<int, float>>(minimiseButtonPanel));
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
  if (Button == GLFW_MOUSE_BUTTON_LEFT && Action == GLFW_RELEASE && resizing) resizing = false;
  if (resizeHover && (Button == GLFW_MOUSE_BUTTON_LEFT && Action == GLFW_PRESS)) resizing = true;
  if (!resizeHover && (Button == GLFW_MOUSE_BUTTON_LEFT && Action == GLFW_PRESS || Action == GLFW_RELEASE)) {
    // Left mouse pressed
    double xPos, yPos;
    glfwGetCursorPos(Window, &xPos, &yPos);
    if (CustomIDEApplication::s_instance) {
      CustomIDEApplication::s_instance->GetUIManager()->AddEvent(std::make_shared<UIMouseEvent>(
                                                    UIMouseEvent(Vector2((float)xPos, (float)yPos),
                                                                 Button, Action, Mods)));
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
  } else {
    // Maximise window is already un-maximised
    glfwMaximizeWindow(Window);
  }
}

void MinimiseCallback(GLFWwindow* Window) {
  glfwIconifyWindow(Window);
}

