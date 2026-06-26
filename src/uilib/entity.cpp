#include "entity.hpp"
#include "ecs.hpp"

void Entity::RenderEntity(EntityManager& Manager, Transform* Transform,
    IRenderable* Renderable, Vector2<float> DrawArea, Vector2<float> DrawAreaOffset) {
  if (Transform == nullptr || Renderable == nullptr)
    return;

  // NOTE: recalculation must be done in this order as position is dependent on entity size
  Transform->RecalculateEntitySize(DrawArea.x, DrawArea.y);
  Transform->RecalculateEntityPosition(DrawArea, DrawAreaOffset, Transform->GetAnchor());
  Renderable->Render(Manager, Transform);
}

void Entity::RenderEntityAndChildren(EntityManager& Manager, Transform* TransformComponent,
    IRenderable* Renderable, Vector2<float> DrawArea, Vector2<float> DrawAreaOffset) {
  RenderEntity(Manager, TransformComponent, Renderable, DrawArea, DrawAreaOffset);
  // Get reference to parent render geometry
  auto parentPos = TransformComponent->GetPixelPosition();
  auto parentSize = TransformComponent->GetPixelSize();

  for (auto entity : m_children) {
    Transform* transform = entity->GetComponent<Transform>();
    IRenderable* renderableComponent = entity->GetRenderableComponent();
    Mask* maskComponent = entity->GetComponent<Mask>();

    if (maskComponent != nullptr)
      Manager.GetClipStack().push(maskComponent->GetClipArea());

    entity->RenderEntityAndChildren(Manager, transform, renderableComponent, parentSize, parentPos);

    if (maskComponent != nullptr)
      Manager.GetClipStack().pop();
  }
}
