#ifndef CUSTOM_ENTITY_COMPONENT_SYSTEM_H
#define CUSTOM_ENTITY_COMPONENT_SYSTEM_H

#include <vector>
#include <memory>
#include "entity.hpp"

class EntityManager {
private:
  std::vector<std::unique_ptr<Entity>> m_entityTree;

public:
  std::vector<std::unique_ptr<Entity>>& GetEntityTree();

  Entity& AddEntity();

  Entity& AddEntity(const Vector2<UISize<float>>& Size, const Vector2<UISize<float>>& Position);

  void RemoveEntity(size_t Index);

  void RenderTree();
};

#endif

