#ifndef CUSTOM_ENTITY_COMPONENT_SYSTEM_H
#define CUSTOM_ENTITY_COMPONENT_SYSTEM_H

#include <vector>
#include <memory>
#include "entity.hpp"
#include "../renderer/renderer.hpp"

class EntityManager {
private:
  std::vector<std::shared_ptr<Entity>> m_entityTree;
  VulkanRenderer& m_renderer;

public:
  EntityManager(VulkanRenderer& Renderer) : m_renderer(Renderer) {}

  EntityManager& operator = (const EntityManager& other) {
    m_entityTree = other.m_entityTree;
    m_renderer = other.m_renderer;
    return *this;
  }

  std::vector<std::shared_ptr<Entity>>& GetEntityTree();

  Entity& AddEntity();

  Entity& AddEntity(const Vector2<UISize<float>>& Size, const Vector2<UISize<float>>& Position);

  void RemoveEntity(size_t Index);

  void RenderTree();
};

#endif

