#include "ecs.hpp"

std::vector<std::unique_ptr<Entity>>& EntityManager::GetEntityTree() {
  return m_entityTree;
}

Entity& EntityManager::AddEntity() {
  m_entityTree.emplace_back(std::make_unique<Entity>());

  return *m_entityTree.back();
}

Entity& EntityManager::AddEntity(const Vector2<UISize<float>>& Size, const Vector2<UISize<float>>& Position) {
  m_entityTree.emplace_back(std::make_unique<Entity>(Size, Position));

  return *m_entityTree.back();
}

void EntityManager::RemoveEntity(const size_t Index) {
  m_entityTree.erase(m_entityTree.begin() + Index);
}

void EntityManager::RenderTree() {

}

