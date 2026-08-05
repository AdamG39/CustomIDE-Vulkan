#ifndef CUSTOM_ENTITY_H
#define CUSTOM_ENTITY_H

#include <vector>
#include <memory>
#include <optional>
#include <type_traits>
#include "components/component.hpp"

// Need to include all types of component headers to know that they inherit from IComponent
// so that AddComponent can create a new component pointer by pointing to the base IComponent class
#include "components/transform.hpp"
#include "components/image.hpp"
#include "components/button.hpp"
#include "components/label.hpp"
#include "components/textBox.hpp"
#include "components/mask.hpp"

#include "ui.hpp"

namespace CustomIDE::UI::ECS {

class Entity {
private:
  std::vector<std::shared_ptr<IComponent>> m_components;
  std::vector<std::shared_ptr<Entity>> m_children;
  Entity* m_parent;

  template <class ComponentType, typename... Args>
  void CreateComponent(Args&&... Parameters) {
    m_components.push_back(std::make_shared<ComponentType>(std::forward<Args>(Parameters)...));
  }

public:
  Entity(Vector2D Size = {}, Vector2D Position = {}, Vector4D Padding = {}, Entity* Parent = nullptr);
  Entity(const Entity& Other);
  Entity& operator=(Entity& Other);
  Entity(Entity&& Other);
  Entity& operator=(Entity&& Other);
  ~Entity();

  template <class ComponentType, typename... Args>
  void AddComponent(Args&&... Parameters) {
    if (std::is_base_of_v<IInteractable, ComponentType> && GetInteractableComponent() != nullptr)
      ReplaceComponent<IInteractable, ComponentType>(Parameters...);

    if (std::is_base_of_v<IRenderable, ComponentType> && GetRenderableComponent() != nullptr)
      ReplaceComponent<IRenderable, ComponentType>(Parameters...);

    CreateComponent<ComponentType>(Parameters...);
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

  void RemoveComponentByIndex(size_t Index) {
    if (Index >= m_components.size()) {
      printf("[Warning]: Attempt to remove component using out of bounds index, call ignored");
      return;
    }

    auto it = m_components.begin();
    it += Index;
    m_components.erase(it);
  }

  template <class PreviousComponent, class NewComponent, typename... NewComponentArgs>
  void ReplaceComponent(NewComponentArgs... Parameters) {
    std::optional<size_t> index = FindIndexOfComponentWithBase<PreviousComponent>();
    // If previous component exists then remove it
    if (index) RemoveComponentByIndex(index.value());
    CreateComponent<NewComponent>(Parameters...);
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

  template <class BaseType>
  std::optional<size_t> FindIndexOfComponentWithBase() {
    size_t index = 0;
    for (auto&& component : m_components) {
      if (dynamic_cast<BaseType*>(component.get())) {
        return index;
      }
      index++;
    }

    // No component found
    return std::nullopt;
  }

  IInteractable* GetInteractableComponent() {
    for (auto&& component : m_components) {
      auto interactable = dynamic_cast<IInteractable*>(component.get());
      if (interactable != nullptr) {
        return interactable ;
      }
    }

    return nullptr;
  }

  IRenderable* GetRenderableComponent() {
    for (auto&& component : m_components) {
      auto renderable = dynamic_cast<IRenderable*>(component.get());
      if (renderable != nullptr) {
        return renderable;
      }
    }

    return nullptr;
  }

  Entity* GetParent() {
    return m_parent;
  }

  void SetParent(Entity* Parent) {
    m_parent = Parent;
  }

  void AddChild(const Entity& Child) {
    m_children.push_back(std::make_shared<Entity>(std::move(Child)));
    auto child = m_children.back();
    child->SetParent(this);
    Transform* transform = child->GetComponent<Transform>();
    Transform* parentTransform = GetComponent<Transform>();

    transform->SetParentTransform(parentTransform);
  }

  void AddChild(std::shared_ptr<UI::ECS::Entity> ChildPtr) {
    m_children.push_back(ChildPtr);
    auto child = m_children.back();
    child->SetParent(this);
    Transform* transform = child->GetComponent<Transform>();
    Transform* parentTransform = GetComponent<Transform>();

    transform->SetParentTransform(parentTransform);
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

  std::shared_ptr<Entity> GetLastChild() {
    return m_children.back();
  }

  void RenderEntity(EntityManager& Manager, Transform* Transform, IRenderable* Renderable);

  void RenderEntityAndChildren(EntityManager& Manager, Transform* Transform, IRenderable* Renderable);

  void RecalculateEntity();
};

} // namespace UI::ECS

#endif

