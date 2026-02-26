#include "ecs.hpp"

std::vector<std::shared_ptr<Entity>>& EntityManager::GetEntityTree() {
  return m_entityTree;
}

const std::list<std::shared_ptr<Entity>> EntityManager::GetAllEntities() const {
  std::list<std::shared_ptr<Entity>> result;

  for (auto& entity : m_entityTree) {
    result.push_back(entity);
  }

  auto it = result.begin();
  while (it != result.end()) {
    if ((*it)->GetChildCount() > 0) {
      for (auto& entity : (*it)->GetChildren())
        result.push_back(entity);
    }
    it++;
  }

  return result;
}

Entity& EntityManager::AddEntity() {
  return *m_entityTree.emplace_back(std::make_shared<Entity>());
}

Entity& EntityManager::AddEntity(const Vector2<UISize<float>>& Size, const Vector2<UISize<float>>& Position) {
  m_entityTree.emplace_back(std::make_shared<Entity>(Size, Position));

  return *m_entityTree.back();
}

void EntityManager::RemoveEntity(const size_t Index) {
  m_entityTree.erase(m_entityTree.begin() + Index);
}

void EntityManager::AddGeometry(Rect<float, float> Geometry) {
  m_geometries.push_back(Geometry);
}

Rect<float, float> EntityManager::GetLastGeometry() {
  return m_geometries.back();
}

void EntityManager::RenderTree(float framebufferWidth, float framebufferHeight) {
  for (auto entity : m_entityTree) {
    Transform* transform = entity->GetComponent<Transform>();
    IRenderable* renderableComponent = entity->GetRenderableComponent();
    entity->RenderEntityAndChildren(*this, transform, renderableComponent, Vector2(framebufferWidth, framebufferHeight), {0, 0});
  }

  std::list<DrawBatch> drawBatches;
  // Order each triangle based on its zIndex then convert each triangle into its vertices
  auto vertices = RectVectorToSortedVertexVector(m_geometries, drawBatches, Vector2<uint32_t>(
        static_cast<uint32_t>(framebufferWidth), static_cast<uint32_t>(framebufferHeight)));

  m_geometries.clear();

  if (vertices.size() != 0)
    m_renderer.FillVertexBuffer(vertices, drawBatches);
}

