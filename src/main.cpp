#include "app.h"

CustomIDEApplication* CustomIDEApplication::s_instance = nullptr;

int main() {
  CustomIDEApplication app;
  CustomIDEApplication::SetInstance(&app);

  app.StartApplication();
  app.MainLoop();

  app.EndApplication();
}
