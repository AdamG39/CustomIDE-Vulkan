#include "entity.hpp"
#include "ecs.hpp"
#include "components.hpp"

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
  auto last = Manager.GetLastGeometry();

  for (auto entity : m_children) {
    Transform* transform = entity->GetComponent<Transform>();
    IRenderable* renderableComponent = entity->GetRenderableComponent();

    entity->RenderEntityAndChildren(Manager, transform, renderableComponent, last.GetSize(), last.GetPosition());
  }
}
