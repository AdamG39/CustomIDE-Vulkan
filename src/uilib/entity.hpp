#ifndef CUSTOM_ENTITY_H
#define CUSTOM_ENTITY_H

#include <vector>
#include <memory>
#include "components.hpp"

class Entity {
private:
  std::vector<std::shared_ptr<IComponent>> m_components;
  std::vector<std::shared_ptr<Entity>> m_children;
  std::shared_ptr<Entity> m_parent;

public:
  Entity(Vector2<UISize<float>> Size = {}, Vector2<UISize<float>> Position = {}, Entity* Parent = nullptr)
  : m_parent(Parent) {
    // By default every object will have a transform component
    // This can be manually removed if preferred
    AddComponent<Transform>(Size, Position);
  }

  template <class ComponentType, typename... Args>
  void AddComponent(Args&&... Parameters) {
    m_components.push_back(std::make_shared<ComponentType>(std::forward<Args>(Parameters)...));
  }

  template <class ComponentType>
  bool RemoveComponent() {
    if (m_components.empty()) return false;

    bool success = false;
    auto index = m_components.begin();
    for (size_t i = 0; i < m_components.size(); i++) {
      if (m_components[i]->GetType() == ComponentType::TypeValue()) {
        success = true;
        index += i;
      }
    }

    if (success) m_components.erase(index);

    return success;
  }

  template <class ComponentType>
  ComponentType* GetComponent() {
    for (auto&& component : m_components) {
      if (component->GetType() == ComponentType::TypeValue()) {
        return static_cast<ComponentType*>(component.get());
      }
    }

    return nullptr;
  }

  std::vector<IRenderable*> GetRenderableComponents() {
    std::vector<IRenderable*> renderableComps;
    for (auto&& component : m_components) {
      auto renderable = dynamic_cast<IRenderable*>(component.get());
      if (renderable != nullptr) {
        renderableComps.push_back(renderable);
      }
    }

    return renderableComps;
  }

  std::shared_ptr<Entity> GetParent() {
    return m_parent;
  }

  void SetParent(Entity* Parent) {
    m_parent = std::shared_ptr<Entity>(Parent);
  }

  void AddChild(const Entity& Child) {
    m_children.push_back(std::make_shared<Entity>(std::move(Child)));
    m_children.back().get()->SetParent(this);
  }

  void RemoveChild(size_t Index) {
    m_children.erase(m_children.begin() + Index);
  }

  const std::vector<std::shared_ptr<Entity>>& GetChildren() {
    return m_children;
  }

  std::shared_ptr<Entity> GetChild(size_t Index) {
    return m_children[Index];
  }

  size_t GetChildCount() {
    return m_children.size();
  }
};

#endif

