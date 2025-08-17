#ifndef CUSTOM_APP_H
#define CUSTOM_APP_H

#include "shapes.hpp"
#include "renderer.hpp"
#include <memory>
#include <map>

#define RESULT_SUCCESS 0
#define RESULT_FAIL 1

#define CURSOR_STATE_DEFAULT 0
#define CURSOR_STATE_HRESIZE 1
#define CURSOR_STATE_VRESIZE 2

#define EVENT_FLAG_DRAGGING 0x00000001

#define WINDOW_FLAG_MAXIMISED 0x00000001

enum class UIEventType { MOUSE_PRESS, MOUSE_RELEASE, WINDOW_MAXIMISE, WINDOW_RESTORE, WINDOW_RESIZE };

enum class ResizeSide { Top, Left, Right, Bottom };

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

  void SetEventType(const UIEventType& Type) {
    UIEvent::Type = Type;
  }
};

template <typename T>
bool CursorOverlap(Vector2<float> CursorPos, Vector2<T> Size, Vector2<T> Position) {
  Vector2 min = Vector2(Position.x - (Size.x / 2), Position.y - (Size.y / 2));
  Vector2 max = Vector2(Position.x + (Size.x / 2), Position.y + (Size.y / 2));

  if (CursorPos.x >= min.x && CursorPos.x <= max.x &&
      CursorPos.y >= min.y && CursorPos.y <= max.y) return true;

  return false;
}

bool CursorAtHorizonalBorder(double xpos, ResizeSide* side);

bool CursorAtVerticalBorder(double ypos, ResizeSide* side);

template <typename T, typename C>
class UIManager {
public:
  uint32_t EventFlags = 0;
  uint32_t WindowFlags = 0;
  Vector2<double> MousePressPosition;

  template <typename Ty>
  void AddElement(Ty&& Element) {
    m_treeObjects.push_back(std::make_shared<std::decay_t<Ty>>(std::forward<Ty>(Element)));
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
    /*m_renderer->FillVertexBuffer(
        ConvertVertexVector<int, float, float, float>(TriVectorToSortedVertexVector(tris)));*/

    m_renderer->FillVertexBuffer(TriVectorToSortedVertexVector(tris));
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
        case UIEventType::WINDOW_MAXIMISE:
          WindowFlags ^= WINDOW_FLAG_MAXIMISED;
          result++;
          break;
        case UIEventType::WINDOW_RESTORE:
          WindowFlags -= WINDOW_FLAG_MAXIMISED;
          result++;
          break;
        case UIEventType::WINDOW_RESIZE:
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

  void RecalculateUILayout(int framebufferWidth, int framebufferHeight) {
    for (size_t i = 0; i < m_treeObjects.size(); i++) {
      m_treeObjects[i]->RecalculateGeometry(framebufferWidth, framebufferHeight);
    }
  }

private:
  std::vector<std::shared_ptr<UIElement<T, C>>> m_treeObjects;

  std::vector<std::shared_ptr<UIEvent>> m_events;

  VulkanRenderer* m_renderer = nullptr;

  std::vector<std::shared_ptr<UIElement<float, float>>> GetTreeElements() {
    std::vector<std::shared_ptr<UIElement<float, float>>> ret;
    
    for (size_t i = 0; i < m_treeObjects.size(); i++) {
      ret.push_back(m_treeObjects[i]);
      if (m_treeObjects[i]->GetChildCount() == 0) continue;

      std::vector<std::shared_ptr<UIElement<float, float>>> temp = m_treeObjects[i]->GetChildren();
      for (size_t j = 0; j < temp.size(); j++) {
        ret.push_back(temp[j]);
      }
    }

    return ret;
  }

  int HandleMouseEvent(std::shared_ptr<UIMouseEvent> Event) {
    std::vector<std::shared_ptr<UIElement<float, float>>> treeObjects = GetTreeElements();
    bool eventSuccessful = false;
    switch (Event->Type) {
      case UIEventType::MOUSE_RELEASE:
        if ((EventFlags & EVENT_FLAG_DRAGGING) != 0) EventFlags -= EVENT_FLAG_DRAGGING;
        break;
      case UIEventType::MOUSE_PRESS:
        for (size_t i = 0; i < treeObjects.size(); i++) {
          if (treeObjects[i]->GetType() != UIType::Button) continue;

          auto button = std::dynamic_pointer_cast<Button<float, float>>(treeObjects[i]);
          if (CursorOverlap(Event->CursorPos, button->GetSize(), button->GetPosition())) { 
            button->OnClick();
            eventSuccessful = true;
            break;
          }
        }
        if (eventSuccessful || WindowFlags & WINDOW_FLAG_MAXIMISED) break;
        if (CursorOverlap(Event->CursorPos, treeObjects[1]->GetSize(), treeObjects[1]->GetPosition())) {
          EventFlags ^= EVENT_FLAG_DRAGGING;
          MousePressPosition = Event->CursorPos;
        }
        break;
      default:
        return -1;
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

  UIManager<float, float>* GetUIManager() const { return m_root; }

  void CreateUIElements();

private:
  const int MIN_WIDTH = 800;
  const int MIN_HEIGHT = 600;

  const GLFWimage WINDOW_ICON = GLFWimage();

  uint32_t m_cursorState = CURSOR_STATE_DEFAULT;

  int m_windowWidth;
  int m_windowHeight;

  VulkanRenderer* m_renderer;

  UIManager<float, float>* m_root;

  std::map<std::string, GLFWcursor*> m_cursorObjects;

  void CreateRenderer(std::string AppName);

  void DestroyRenderer();

  GLFWcursor* GetCursorObject(std::string Index);

  void SetCursorState(int State);

  void HandleResizing();

  void UpdateCursorState();

  void HandleDragging();
};

#endif

