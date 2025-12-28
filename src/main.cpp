#include "app.hpp"

CustomIDEApplication* CustomIDEApplication::s_instance = nullptr;

int main() {
  CustomIDEApplication* app = CustomIDEApplication::GetInstance();
  app->InitApplication();
  app->RunApplication();
  app->EndApplication();
}
