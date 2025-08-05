#include "app.h"
#include <GLFW/glfw3.h>

void CloseWindowCallback(GLFWwindow* Window){
  glfwDestroyWindow(Window);
}

void MouseButtonCallback(GLFWwindow* Window, int Button, int Action, int Mods) {
  if (Button == GLFW_MOUSE_BUTTON_LEFT && Action == GLFW_PRESS) {
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

void CustomIDEApplication::StartApplication() {
  CreateRenderer(ApplicationName);
  m_root = new UIManager<int, float>();
  m_root->BindRenderer(*m_renderer);

  Panel background = Panel("Background",
                           Vector2(m_windowWidth, m_windowHeight),
                           Vector2(m_windowWidth / 2, m_windowHeight / 2),
                           Colour(0x3B1C32, 1.0f));

  Panel titleBar = Panel("TitleBar",
                         Vector2(m_windowWidth, 40),
                         Vector2(m_windowWidth / 2, 20),
                         Colour(0x1A1A1D, 1.0f));

  Panel box = Panel("ExitButtonPanel",
                    Vector2(50, 40),
                    Vector2(m_windowWidth - 25, 20),
                    Colour(0xFF0000, 1.0f));

  Button exitButton = Button<int, float>(Vector2(50, 40),
                                         Vector2(m_windowWidth - 25, 20),
                                         CloseWindowCallback, m_renderer->GetWindow());

  box.AddChild(std::make_shared<Button<int, float>>(exitButton));

  titleBar.GetGeometry().SetZIndex(1);
  box.GetGeometry().SetZIndex(1);

  m_root->AddElement(std::make_shared<Panel<int, float>>(background));
  m_root->AddElement(std::make_shared<Panel<int, float>>(titleBar));
  m_root->AddElement(std::make_shared<Panel<int, float>>(box));
}

void CustomIDEApplication::MainLoop() {
  while (!glfwWindowShouldClose(m_renderer->GetWindow())) {
    glfwWaitEventsTimeout(0.5f);

    m_root->HandleEvents();

    m_root->RenderAll();

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

