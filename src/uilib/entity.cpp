#include "entity.hpp"
#include "ecs.hpp"

namespace CustomIDE {

UI::ECS::Entity::Entity(Vector2D Size, Vector2D Position, Vector4D Padding, Entity* Parent)
: m_parent(Parent) {
  // By default every object will have a transform component
  // This can be manually removed if preferred
  AddComponent<Transform>(Padding, Size, Position, ((Parent != nullptr) ? Parent->GetComponent<Transform>() : nullptr));
}

void UI::ECS::Entity::RenderEntity(EntityManager& Manager, Transform* Transform, IRenderable* Renderable) {
  if (Transform == nullptr || Renderable == nullptr)
    return;

  Renderable->Render(Manager, Transform);
}

void UI::ECS::Entity::RenderEntityAndChildren(EntityManager& Manager, Transform* TransformComponent, IRenderable* Renderable) {
  RenderEntity(Manager, TransformComponent, Renderable);

  for (auto entity : m_children) {
    Transform* transform = entity->GetComponent<Transform>();
    IRenderable* renderableComponent = entity->GetRenderableComponent();
    Mask* maskComponent = entity->GetComponent<Mask>();

    if (maskComponent != nullptr)
      Manager.GetClipStack().push(maskComponent->GetClipArea());

    entity->RenderEntityAndChildren(Manager, transform, renderableComponent);

    if (maskComponent != nullptr)
      Manager.GetClipStack().pop();
  }
}

void UI::ECS::Entity::RecalculateEntity() {
  Transform* transform = GetComponent<Transform>();

  if (transform) transform->RecalculateTransform();

  for (auto& entity : m_children) {
    transform = entity->GetComponent<Transform>();

    if (transform) transform->RecalculateTransform();
  }
}

} // namespace CustomIDE
