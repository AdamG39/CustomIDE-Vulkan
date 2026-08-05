#include "ecs.hpp"

namespace CustomIDE {

UI::ECS::EntityManager::EntityManager(std::weak_ptr<Vulkan::Renderer> Renderer) {
  m_renderer = Renderer;
}

std::weak_ptr<Vulkan::Renderer> UI::ECS::EntityManager::GetRenderer() {
  return m_renderer;
}

std::vector<std::shared_ptr<UI::ECS::Entity>>& UI::ECS::EntityManager::GetEntityTree() {
  return m_entityTree;
}

std::shared_ptr<UI::ECS::Entity> UI::ECS::EntityManager::GetLastEntity() const {
  return m_entityTree.back();
}

const std::list<std::shared_ptr<UI::ECS::Entity>> UI::ECS::EntityManager::GetAllEntities() const {
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

std::stack<ClipRect>& UI::ECS::EntityManager::GetClipStack() {
  return m_clipStack;
}

UI::ECS::Entity& UI::ECS::EntityManager::AddEntity() {
  return *m_entityTree.emplace_back(std::make_shared<Entity>());
}

UI::ECS::Entity& UI::ECS::EntityManager::AddEntity(const Vector2D& Size, const Vector2D& Position, const Vector4D& Padding) {
  m_entityTree.emplace_back(std::make_shared<Entity>(Size, Position, Padding));

  return *m_entityTree.back();
}

void UI::ECS::EntityManager::RemoveEntity(const size_t Index) {
  m_entityTree.erase(m_entityTree.begin() + Index);
}

void UI::ECS::EntityManager::RenderTree() {
  for (auto entity : m_entityTree) {
    Transform* transform = entity->GetComponent<Transform>();
    IRenderable* renderableComponent = entity->GetRenderableComponent();
    Mask* maskComponent = entity->GetComponent<Mask>();

    if (maskComponent != nullptr)
      m_clipStack.push(maskComponent->GetClipArea());

    entity->RenderEntityAndChildren(*this, transform, renderableComponent);

    if (maskComponent != nullptr)
      m_clipStack.pop();
  }
}

void UI::ECS::EntityManager::RecalculateTree() {
  for (auto& entity : m_entityTree) {
    entity->RecalculateEntity();
  }
}

} // namespace CustomIDE

