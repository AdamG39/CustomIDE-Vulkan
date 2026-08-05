#ifndef CUSTOM_COMPONENT_TRANSFORM_H
#define CUSTOM_COMPONENT_TRANSFORM_H

#include "component.hpp"
#include "../../renderer/shapes.hpp"
#include "../ui.hpp"

namespace CustomIDE::UI::ECS {

class Transform : public IComponent {
private:
  Transform* m_parentTransform;

  Vector2D m_position;
  Vector2D m_size;
  Vector4D m_padding;
  Vector2D m_pivot;
  UI::Anchor m_anchor;

  Vector2<bool> m_recalculateGlobalRect;
  Vector2D m_globalPosition;
  Vector2D m_globalSize;

  void GetParentGlobalRect(Vector2D& ParentPosition, Vector2D& ParentSize) const;

  Vector2D CalculateGlobalPosition() const;
  Vector2D CalculateGlobalSize() const;

public:
  static int TypeValue() { return TypeTransform; }
  int GetType() override;

  Transform(const Vector2D& Size, const Vector2D& Position, Transform* ParentTransform);
  Transform(const Vector4D& Padding, const Vector2D& Position, Transform* ParentTransform);
  Transform(const Vector4D& Padding, const Vector2D& Size, const Vector2D& Position, Transform* ParentTransform);

  Vector2D GetLocalPosition() const;
  Vector2D GetGlobalPosition();
  Vector2D GetGlobalPosition() const;
  void SetLocalPosition(const Vector2D& Position);

  Vector2D GetLocalSize() const;
  Vector2D GetGlobalSize();
  Vector2D GetGlobalSize() const;
  void SetLocalSize(const Vector2D& Size);

  Vector4D GetPadding() const;
  void SetPadding(const Vector4D& Padding);

  void RecalculateTransform();

  Vector2D GetPivot() const;
  void SetPivot(const Vector2D& Pivot);

  UI::Anchor GetAnchor() const;
  void SetAnchorPreset(const AnchorPresets::PresetID& Preset);
  void SetCustomAnchor(const UI::Anchor& Anchor);

  void SetParentTransform(Transform* ParentTransform);
};

} // namespace UI::ECS

#endif

