#ifndef CUSTOM_ENTITY_H
#define CUSTOM_ENTITY_H

#include <vector>
#include <memory>
#include "components.hpp"

class Entity {
private:
  std::vector<std::shared_ptr<IComponent>> m_components;

public:
  Entity() {
    // By default every object will have a transform component
    // This can be manually removed if preferred
    AddComponent<Transform>();
  }

  Entity(Vector2<UISize<float>> Size, Vector2<UISize<float>> Position) {
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
};

#endif

