#ifndef CUSTOM_UI_H
#define CUSTOM_UI_H

namespace CustomIDE::UI {

enum class SizeMode { Fixed, Proportional };

template <typename T>
struct Size {
  T Value{};
  SizeMode Mode{SizeMode::Fixed};

  Size() = default;

  Size(T Value, SizeMode Mode = SizeMode::Fixed)
  : Value(Value), Mode(Mode) {}
};

enum class AnchorType {
  Center,
  Top,
  Bottom,
  Left,
  Right,
  TopLeft,
  TopRight,
  BottomLeft,
  BottomRight
};

} // namespace UI

#endif

