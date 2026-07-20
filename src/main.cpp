#include "app.hpp"

CustomIDE::Application* CustomIDE::Application::s_instance = nullptr;

int main() {
  CustomIDE::Application* app = CustomIDE::Application::GetInstance();
  app->InitApplication();
  app->RunApplication();
  app->EndApplication();
}
