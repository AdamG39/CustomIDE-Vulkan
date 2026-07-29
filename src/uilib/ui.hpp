#ifndef CUSTOM_UI_H
#define CUSTOM_UI_H

#include <utility>
#include "../renderer/shapes.hpp"

namespace CustomIDE::UI {

struct Anchor {
  Vector2D Min;
  Vector2D Max;

  constexpr bool operator==(const Anchor& Other) const {
    return (this->Min == Other.Min) && (this->Max == Other.Max);
  }
};

namespace AnchorPresets {

  using PresetID = size_t;
  constexpr PresetID TOP_LEFT         = 0;
  constexpr PresetID TOP_CENTER       = 1;
  constexpr PresetID TOP_RIGHT        = 2;
  constexpr PresetID CENTER_LEFT      = 3;
  constexpr PresetID CENTER_CENTER    = 4;
  constexpr PresetID CENTER_RIGHT     = 5;
  constexpr PresetID BOTTOM_LEFT      = 6;
  constexpr PresetID BOTTOM_CENTER    = 7;
  constexpr PresetID BOTTOM_RIGHT     = 8;

  constexpr PresetID STRETCH_TOP      = 9;
  constexpr PresetID STRETCH_LEFT     = 10;
  constexpr PresetID STRETCH_RIGHT    = 11;
  constexpr PresetID STRETCH_BOTTOM   = 12;
  constexpr PresetID STRETCH_CENTER_H = 13;
  constexpr PresetID STRETCH_CENTER_V = 14;
  constexpr PresetID STRETCH_ALL      = 15;

  constexpr Anchor Presets[] {
    // Fixed size presets
    Anchor{{ 0.0f, 1.0f }, { 0.0f, 1.0f }}, // TOP_LEFT
    Anchor{{ 0.5f, 1.0f }, { 0.5f, 1.0f }}, // TOP_CENTER
    Anchor{{ 1.0f, 1.0f }, { 1.0f, 1.0f }}, // TOP_RIGHT
    Anchor{{ 0.0f, 0.5f }, { 0.0f, 0.5f }}, // CENTER_LEFT
    Anchor{{ 0.5f, 0.5f }, { 0.5f, 0.5f }}, // CENTER_CENTER
    Anchor{{ 1.0f, 0.5f }, { 1.0f, 0.5f }}, // CENTER_RIGHT
    Anchor{{ 0.0f, 0.0f }, { 0.0f, 0.0f }}, // BOTTOM_LEFT
    Anchor{{ 0.5f, 0.0f }, { 0.5f, 0.0f }}, // BOTTOM_CENTER
    Anchor{{ 1.0f, 0.0f }, { 1.0f, 0.0f }}, // BOTTOM_RIGHT

    // Stretch presets
    Anchor{{ 0.0f, 1.0f }, { 1.0f, 1.0f }}, // STRETCH_TOP
    Anchor{{ 0.0f, 0.0f }, { 0.0f, 1.0f }}, // STRETCH_LEFT
    Anchor{{ 1.0f, 1.0f }, { 1.0f, 0.0f }}, // STRETCH_RIGHT
    Anchor{{ 0.0f, 0.0f }, { 1.0f, 0.0f }}, // STRETCH_BOTTOM
    Anchor{{ 0.0f, 0.5f }, { 1.0f, 0.5f }}, // STRETCH_CENTER_H
    Anchor{{ 0.5f, 0.0f }, { 0.5f, 1.0f }}, // STRETCH_CENTER_V
    Anchor{{ 0.0f, 0.0f }, { 1.0f, 1.0f }}, // STRETCH_ALL
  };

} // namespace AnchorPresets

} // namespace UI

#endif

