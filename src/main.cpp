#include "app.hpp"

CustomIDE::Application* CustomIDE::Application::s_instance = nullptr;

int main(int argc, char* argv[]) {
  CustomIDE::Application* app = CustomIDE::Application::GetInstance();
  app->InitApplication();
  app->RunApplication();
  app->EndApplication();
}

#ifdef _WIN32
  #include <windows.h>

  int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    return main(__argc, __argv);
  }
#endif

