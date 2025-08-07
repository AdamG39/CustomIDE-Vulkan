#pragma once

#include "shapes.h"
#include "renderer.h"
#include <memory>

#define RESULT_SUCCESS 0
#define RESULT_FAIL 1

enum class UIEventType { MOUSE_PRESS, MOUSE_RELEASE };

struct UIEvent {
  UIEventType Type;

  UIEvent(UIEventType Type) : Type(Type) {}

  virtual ~UIEvent() = default;
};

struct UIMouseEvent : public UIEvent {
  Vector2<float> CursorPos;
  int Button;
  int Mods;

  UIMouseEvent(Vector2<float> CursorPos, int Button, int Action, int Mods)
  : UIEvent((Action == GLFW_PRESS) ? UIEventType::MOUSE_PRESS : UIEventType::MOUSE_RELEASE),
    CursorPos(CursorPos),
    Button(Button),
    Mods(Mods) {}
};

template <typename T>
bool CursorOverlap(Vector2<float> CursorPos, Vector2<T> Size, Vector2<T> Position) {
  Vector2 min = Vector2(Position.x - (Size.x / 2), Position.y - (Size.y / 2));
  Vector2 max = Vector2(Position.x + (Size.x / 2), Position.y + (Size.y / 2));

  if (CursorPos.x >= min.x && CursorPos.x <= max.x &&
      CursorPos.y >= min.y && CursorPos.y <= max.y) return true;

  return false;
}

template <typename T, typename C>
class UIManager {
public:
  void AddElement(std::shared_ptr<UIElement<T, C>> Element) {
    m_treeObjects.push_back(Element);
  }

  void RemoveElement(UIElement<T, C>* Element) {
    int index = -1;
    for (size_t i = 0; i < m_treeObjects.size(); i++) {
      if (m_treeObjects[i].get() == Element) {
        index = i;
        break;
      }
    }

    if (index == -1) {
      ExitWithError("Attempted to remove panel from tree when panel doesnt exist", -2);
    }

    // TODO remove child panels as well

    if (m_treeObjects[index]->GetChildCount() > 0) {
      // Delete children
      m_treeObjects[index]->RemoveChildren();
    }
    
    m_treeObjects.erase(m_treeObjects.begin() + index);
  }

  void RemoveAllElements() {
    for (size_t i = 0; i < m_treeObjects.size(); i++) {
      if (m_treeObjects[i]->GetChildCount() > 0) {
        m_treeObjects[i]->RemoveChildren();
      }
    }
    m_treeObjects.clear();
  }

  UIElement<T, C>* GetElementFromIndex(int Index) {
    if (Index > m_treeObjects.size()) {
      ExitWithError("Index out of range of element list", -2);
    }

    return m_treeObjects[Index].get();
  }

  UIElement<T, C>* GetElementFromName(std::string Name) {
    int index = -1;

    for (size_t i = 0; i < m_treeObjects.size(); i++) {
      if (Name == m_treeObjects[i]->Label) {
        index = i;
        break;
      }
    }

    if (index == -1) {
      ExitWithError("No element found with that label", -2);
    }

    return GetPanelFromIndex(index);
  }

  void RenderAll() {
    // Follow the tree and add all geometry to a list
    // Order list via zIndex
    // Send list to renderer

    // Gather all rect geometries for each object in the tree
    std::vector<Rect<T, C>> rects;

    for (size_t i = 0; i < m_treeObjects.size(); i++) {
      if (!(m_treeObjects[i]->IsVisible())) continue;
      std::vector<Rect<T, C>> temp = m_treeObjects[i]->GetTotalGeometries();
      for (size_t j = 0; j < temp.size(); j++) {
        rects.push_back(temp[j]);
      }
    }

    // Convert each rect into its triangles
    std::vector<Triangle<T, C>> tris;

    for (size_t i = 0; i < rects.size(); i++) {
      tris.push_back(rects[i].GetTris()[0]);
      tris.push_back(rects[i].GetTris()[1]);
    }

    // Order each triangle based on its zIndex then convert each triangle into its vertices
    auto verts = TriVectorToSortedVertexVector(tris);

    // Pass the list of vertices to the vertex buffer converting if nessessary
    m_renderer->FillVertexBuffer(
        ConvertVertexVector<int, float, float, float>(TriVectorToSortedVertexVector(tris)));
  }

  void AddEvent(std::shared_ptr<UIEvent> Event) { 
    m_events.push_back(std::move(Event));
  }

  void HandleEvents() {
    // Loop through m_events handling them respectively
    
    if (m_events.size() == 0) { return; }

    int result = 0;

    for (size_t i = 0; i < m_events.size(); i++) {
      switch (m_events[i]->Type) {
        case UIEventType::MOUSE_PRESS:
        case UIEventType::MOUSE_RELEASE:
          HandleMouseEvent(std::dynamic_pointer_cast<UIMouseEvent>(m_events[i]));
          result++;
          break;
      }
    }

    if (result > 0 && result == m_events.size()) {
      m_events.clear();
    }
  }

  void BindRenderer(VulkanRenderer& Renderer) {
    m_renderer = &Renderer;
  }

private:
  std::vector<std::shared_ptr<UIElement<T, C>>> m_treeObjects;

  std::vector<std::shared_ptr<UIEvent>> m_events;

  VulkanRenderer* m_renderer = nullptr;

  std::vector<std::shared_ptr<UIElement<int, float>>> GetTreeElements() {
    std::vector<std::shared_ptr<UIElement<int, float>>> ret;
    
    for (size_t i = 0; i < m_treeObjects.size(); i++) {
      ret.push_back(m_treeObjects[i]);
      if (m_treeObjects[i]->GetChildCount() == 0) continue;

      std::vector<std::shared_ptr<UIElement<int, float>>> temp = m_treeObjects[i]->GetChildren();
      for (size_t j = 0; j < temp.size(); j++) {
        ret.push_back(temp[j]);
      }
    }

    return ret;
  }

  int HandleMouseEvent(std::shared_ptr<UIMouseEvent> Event) {
    switch (Event->Type) {
      case UIEventType::MOUSE_RELEASE:
      case UIEventType::MOUSE_PRESS:
        std::vector<std::shared_ptr<UIElement<int, float>>> treeObjects = GetTreeElements();
        for (size_t i = 0; i < treeObjects.size(); i++) {
          if (treeObjects[i]->GetType() != UIType::Button) continue;

          auto button = std::dynamic_pointer_cast<Button<int, float>>(treeObjects[i]);
          if (CursorOverlap(Event->CursorPos, button->GetSize(), button->GetPositon())) button->OnClick();
        }
    }
    return RESULT_SUCCESS;
  }

};

class CustomIDEApplication {
public:
  static CustomIDEApplication* s_instance;

  std::string ApplicationName = "CustomIDE";

  static void SetInstance(CustomIDEApplication* App) {
    s_instance = App;
  }

  void StartApplication();

  void MainLoop();

  void EndApplication();

  VulkanRenderer* GetRenderer() const { return m_renderer; }

  UIManager<int, float>* GetUIManager() const { return m_root; }

  void CreateUIElements();

private:
  const uint32_t WIDTH = 1920;
  const uint32_t HEIGHT = 1080;

  int m_windowWidth;
  int m_windowHeight;

  VulkanRenderer* m_renderer;

  UIManager<int, float>* m_root;

  void CreateRenderer(std::string AppName);

  void DestroyRenderer();
};

