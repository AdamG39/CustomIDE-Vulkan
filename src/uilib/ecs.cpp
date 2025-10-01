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

void EntityManager::RenderTree() {
  std::vector<Rect<float, float>> geometries;

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
      StaticColour* staticColour = top->GetComponent<StaticColour>();
      Texture* texture = top->GetComponent<Texture>();
      // An entity requires a colour/texture component and a transform to be renderered
      if (transform == nullptr) continue; // Just skip this entity since it cant be renderered

      if (staticColour != nullptr) {
        geometries.emplace_back(transform->GetPixelSize(), transform->GetPixelPosition(), staticColour->GetColour());
      } else if (texture != nullptr) {
        geometries.emplace_back(transform->GetPixelSize(), transform->GetPixelPosition(), COLOUR_CLEAR);
        geometries.back().SetTextureCoords(texture->GetTextureCoords());
      }
      // Dont do anything if either condition isnt met
    }
  }

  std::vector<Triangle<float, float>> tris;

  for (size_t i = 0; i < geometries.size(); i++) {
    std::array<Triangle<float, float>, 2> temp = geometries[i].GetTris();
    tris.push_back(temp[0]);
    tris.push_back(temp[1]);
  }

  // Order each triangle based on its zIndex then convert each triangle into its vertices
  auto vertices = TriVectorToSortedVertexVector(tris);

  if (vertices.size() != 0)
    m_renderer.FillVertexBuffer(vertices);
}

