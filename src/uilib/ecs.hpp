#ifndef CUSTOM_ENTITY_COMPONENT_SYSTEM_H
#define CUSTOM_ENTITY_COMPONENT_SYSTEM_H

#include <vector>
#include <memory>
#include <stack>
#include "entity.hpp"
#include "../renderer/renderer.hpp"

namespace CustomIDE::UI::ECS {

class EntityManager {
private:
  std::vector<std::shared_ptr<Entity>> m_entityTree;
  std::stack<ClipRect> m_clipStack;
  std::weak_ptr<Vulkan::Renderer> m_renderer;

public:
  EntityManager(std::weak_ptr<Vulkan::Renderer> Renderer);

  std::weak_ptr<Vulkan::Renderer> GetRenderer();

  std::vector<std::shared_ptr<Entity>>& GetEntityTree();

  std::shared_ptr<Entity> GetLastEntity() const;

  const std::list<std::shared_ptr<Entity>> GetAllEntities() const;

  std::stack<ClipRect>& GetClipStack();

  Entity& AddEntity();

  Entity& AddEntity(const Vector2D& Size, const Vector2D& Position, const Vector4D& Padding = {});

  Entity& AddEntity(const Entity& _Entity);

  void RemoveEntity(size_t Index);

  void RenderTree();

  void RecalculateTree();
};

} // namespace UI::ECS

#endif

