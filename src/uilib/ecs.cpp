#include <stack>
#include "ecs.hpp"

std::vector<std::shared_ptr<Entity>>& EntityManager::GetEntityTree() {
  return m_entityTree;
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

void EntityManager::RenderTree(float framebufferWidth, float framebufferHeight) {
  for (size_t i = 0; i < m_entityTree.size(); i++) {
    std::stack<Entity*> entities;
    entities.push(m_entityTree[i].get());

    while (!entities.empty()) {
      Entity* top = entities.top();
      entities.pop();
      for (auto entity : top->GetChildren()) {
        entities.push(entity.get());
      }

      Transform* transform = top->GetComponent<Transform>();
      auto renderableComponent = top->GetRenderableComponent();

      renderableComponent->Render(*this, transform, Vector2<float>(framebufferWidth, framebufferHeight));
    }
  }

  std::vector<Triangle<float, float>> tris;

  for (auto it = m_geometries.begin(); it != m_geometries.end(); it++) {
    std::array<Triangle<float, float>, 2> temp = it->GetTris();
    tris.push_back(temp[0]);
    tris.push_back(temp[1]);
  }

  m_geometries.clear();

  std::vector<TextureArrayBounds> textureIndexArrayBounds;
  // Order each triangle based on its zIndex then convert each triangle into its vertices
  auto vertices = TriVectorToSortedVertexVector(tris, textureIndexArrayBounds);

  if (vertices.size() != 0)
    m_renderer.FillVertexBuffer(vertices, textureIndexArrayBounds);
}

