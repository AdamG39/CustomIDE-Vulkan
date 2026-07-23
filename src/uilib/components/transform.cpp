#include "transform.hpp"

namespace CustomIDE {

extern int framebufferWidth;
extern int framebufferHeight;

int UI::ECS::Transform::GetType() { return TypeValue(); }

void UI::ECS::Transform::GetParentGlobalRect(Vector2D& ParentPosition, Vector2D& ParentSize) const {
  if (m_parentTransform) {
    ParentPosition = m_parentTransform->GetGlobalPosition();
    ParentSize = m_parentTransform->GetGlobalSize();
  }
  else {
    ParentPosition = { static_cast<float>(framebufferWidth / 2), static_cast<float>(framebufferHeight / 2) };
    ParentSize = { static_cast<float>(framebufferWidth), static_cast<float>(framebufferHeight) };
  }
}

Vector2D UI::ECS::Transform::CalculateGlobalPosition() const {
  Vector2D parentGlobalPosition{};
  Vector2D parentGlobalSize{};

  GetParentGlobalRect(parentGlobalPosition, parentGlobalSize);

  // m_anchor.Min == Bottom Left
  // m_anchor.Max == Top Right
  // x = % from Left -> Right where 0 == left and 1 == right
  // y = % from Bottom -> Top where 0 == bottom and 1 == top

  float left = parentGlobalPosition.x - (parentGlobalSize.x / 2.f);
  float bottom = parentGlobalPosition.y + (parentGlobalSize.y / 2.f);

  // { x1, y1 }
  Vector2D anchorBottomLeft { left + (m_anchor.Min.x * parentGlobalSize.x),
                              bottom - (m_anchor.Min.y * parentGlobalSize.y) };
  // { x2, y2 }
  Vector2D anchorTopRight { left + (m_anchor.Max.x * parentGlobalSize.x),
                            bottom - (m_anchor.Max.y * parentGlobalSize.y) };
  // { (x1 + x2) / 2, (y1, y2) / 2 }
  Vector2D anchorCentralPoint { (anchorTopRight.x + anchorBottomLeft.x) / 2.f,
                                (anchorBottomLeft.y + anchorTopRight.y) / 2.f };

  return anchorCentralPoint + m_position;
}

Vector2D UI::ECS::Transform::CalculateGlobalSize() const {
  bool stretchHorizontal = ((m_anchor.Min.y == m_anchor.Max.y) && m_anchor.Min != m_anchor.Max) || m_anchor == AnchorPresets::Presets[AnchorPresets::STRETCH_ALL]; // is this shape stretching horizontally
  bool stretchVertical = ((m_anchor.Min.x == m_anchor.Max.x) && m_anchor.Min != m_anchor.Max) || m_anchor == AnchorPresets::Presets[AnchorPresets::STRETCH_ALL]; // is this shape stretching vertically

  if (!stretchHorizontal && !stretchVertical) return m_size; // transform doesnt stretch so no futher calculation needed

  Vector2D parentGlobalPosition{};
  Vector2D parentGlobalSize{};

  GetParentGlobalRect(parentGlobalPosition, parentGlobalSize);

  // m_anchor.Min == Bottom Left
  // m_anchor.Max == Top Right
  // x = % from Left -> Right where 0 == left and 1 == right
  // y = % from Bottom -> Top where 0 == bottom and 1 == top

  float left = parentGlobalPosition.x - (parentGlobalSize.x / 2.f);
  float bottom = parentGlobalPosition.y + (parentGlobalSize.y / 2.f);

  // { x1, y1 }
  Vector2D anchorBottomLeft { left + (m_anchor.Min.x * parentGlobalSize.x),
                              bottom - (m_anchor.Min.y * parentGlobalSize.y) };
  // { x2, y2 }
  Vector2D anchorTopRight { left + (m_anchor.Max.x * parentGlobalSize.x),
                            bottom - (m_anchor.Max.y * parentGlobalSize.y) };

  Vector2D calculatedSize{m_size};

  // Padding is in the following format:
  // x = Left padding
  // y = Top padding
  // z = Right padding
  // w = Bottom padding
  if (stretchHorizontal) {
    calculatedSize.x = parentGlobalSize.x - (m_padding.x + m_padding.z);
  }
  if (stretchVertical) {
    calculatedSize.y = parentGlobalSize.y - (m_padding.y + m_padding.w);
  }

  return calculatedSize;
}

UI::ECS::Transform::Transform(const Vector2D& Size, const Vector2D& Position, Transform* ParentTransform)
  : m_size(Size), m_position(Position), m_parentTransform(ParentTransform), m_recalculateGlobalRect({true, true}) {
  SetAnchorPreset(AnchorPresets::CENTER_CENTER);
}

UI::ECS::Transform::Transform(const Vector4D& Padding, const Vector2D& Position, Transform* ParentTransform)
  : m_padding(Padding), m_position(Position), m_parentTransform(ParentTransform), m_recalculateGlobalRect({true, true}) {
  SetAnchorPreset(AnchorPresets::CENTER_CENTER);
}

UI::ECS::Transform::Transform(const Vector4D& Padding, const Vector2D& Size, const Vector2D& Position, Transform* ParentTransform)
  : m_padding(Padding), m_size(Size), m_position(Position), m_parentTransform(ParentTransform), m_recalculateGlobalRect({true, true}) {
  SetAnchorPreset(AnchorPresets::CENTER_CENTER);
}

Vector2D UI::ECS::Transform::GetLocalPosition() const {
  return m_position;
}

Vector2D UI::ECS::Transform::GetGlobalPosition() {
  if (!m_recalculateGlobalRect.x) return m_globalPosition;

  m_globalPosition = CalculateGlobalPosition();
  m_recalculateGlobalRect.x = false;
  return m_globalPosition;
}

Vector2D UI::ECS::Transform::GetGlobalPosition() const {
  return CalculateGlobalPosition();
}

void UI::ECS::Transform::SetLocalPosition(const Vector2D& Position) {
  m_position = Position;
}

Vector2D UI::ECS::Transform::GetLocalSize() const {
  return m_size;
}

Vector2D UI::ECS::Transform::GetGlobalSize() {
  if (!m_recalculateGlobalRect.y) return m_globalSize;

  m_globalSize = CalculateGlobalSize();
  m_recalculateGlobalRect.y = false;
  return m_globalSize;
}

Vector2D UI::ECS::Transform::GetGlobalSize() const {
  return CalculateGlobalSize();
}

void UI::ECS::Transform::SetLocalSize(const Vector2D& Size) {
  m_size = Size;
}

Vector4D UI::ECS::Transform::GetPadding() const {
  return m_padding;
}

void UI::ECS::Transform::SetPadding(const Vector4D& Padding) {
  m_padding = Padding;
}

void UI::ECS::Transform::RecalculateTransform() {
  m_recalculateGlobalRect.x = true;
  m_recalculateGlobalRect.y = true;
}

Vector2D UI::ECS::Transform::GetPivot() const {
  return m_pivot;
}

void UI::ECS::Transform::SetPivot(const Vector2D& Pivot) {
  m_pivot = Pivot;
}

UI::Anchor UI::ECS::Transform::GetAnchor() const {
  return m_anchor;
}

void UI::ECS::Transform::SetAnchorPreset(const AnchorPresets::PresetID& Preset) {
  auto& anchorPreset = AnchorPresets::Presets[Preset];
  m_anchor.Min = anchorPreset.Min;
  m_anchor.Max = anchorPreset.Max;
}

void UI::ECS::Transform::SetCustomAnchor(const UI::Anchor& Anchor) {
  m_anchor = Anchor;
}

} // namespace CustomIDE

