#include <stack>
#include "ecs.hpp"
#include "../helpers/errors/errors.hpp"

std::vector<std::shared_ptr<Entity>>& EntityManager::GetEntityTree() {
  return m_entityTree;
}

Entity& EntityManager::AddEntity() {
  return *m_entityTree.emplace_back(std::make_shared<Entity>());
}

Entity& EntityManager::AddEntity(const Vector2<UISize<float>>& Size, const Vector2<UISize<float>>& Position) {
  m_entityTree.emplace_back(std::make_shared<Entity>(Size, Position));

  return *m_entityTree.back();
}

void EntityManager::RemoveEntity(const size_t Index) {
  m_entityTree.erase(m_entityTree.begin() + Index);
}

void EntityManager::RenderTree() {
  std::vector<Rect<float, float>> geometries;

  for (size_t i = 0; i < m_entityTree.size(); i++) {
    std::stack<Entity*> entities;
    entities.push(m_entityTree[i].get());

    while (!entities.empty()) {
      Entity* top = entities.top();
      entities.pop();
      for (auto entity : top->GetChildren()) {
        entities.push(entity.get());
      }

      Transform* transform = top->GetComponent<Transform>();
      // TODO: merge colour and texture into one component 
      //auto renderableComponents = top->GetRenderableComponents();
      StaticColour* staticColour = top->GetComponent<StaticColour>();
      Texture* texture = top->GetComponent<Texture>();
      bool isTextBox = false;
      IText* textObj = top->GetComponent<Label>();
      if (textObj == nullptr) {
        textObj = top->GetComponent<TextBox>();
        isTextBox = true;
      }

      // An entity requires a renderable component and a transform to be renderered
      if (transform == nullptr) continue; // Just skip this entity since it cant be renderered

      if (staticColour != nullptr) {
        geometries.emplace_back(transform->GetPixelSize(), transform->GetPixelPosition(),
                                staticColour->GetColour(), staticColour->GetDrawDepth());
      } else if (texture != nullptr) {
        geometries.emplace_back(transform->GetPixelSize(), transform->GetPixelPosition(),
                                COLOUR_WHITE, texture->GetDrawDepth());
        geometries.back().SetTextureCoords(texture->GetTextureCoords());
        geometries.back().SetTextureIndex(texture->GetTextureIndex());
      } else if (textObj != nullptr) {
        // calculate size of each character based on font
        Font font = textObj->GetFont();
        Vector2 textObjPos = transform->GetPixelPosition();
        Vector2 fontAtlasSize {64.f, 2.f};
        std::string content = textObj->GetContent();

        int charsPerLine = int(transform->GetPixelSize().x) / font.size.x;
        // create a rect for each character
        int linePosition = 0;
        int lineCount = 0;
        for (size_t i = 0; i < content.size(); i++) {
          switch (content[i]) {
          case '\n':
            lineCount++;
            linePosition = 0;
            continue;
          case ' ':
            linePosition++;
            if (textObj->GetWordWrap()) {
              if (linePosition > charsPerLine) { 
                lineCount++;
                linePosition = 0;
              }
            }
            continue;
          case '\t':
            linePosition += 4;
            if (textObj->GetWordWrap()) {
              if (linePosition > charsPerLine) { 
                lineCount++;
                linePosition = 0;
              }
            }
            continue;
          }

          Vector2<float> charPosition {
            textObjPos.x + (font.size.x * linePosition),
            textObjPos.y + (lineCount * font.size.y)
          };

          linePosition++;

          if (textObj->GetWordWrap()) {
            if (linePosition > charsPerLine) { 
              lineCount++;
              linePosition = 0;
            }
          }

          auto temp = dynamic_cast<IRenderable*>(textObj);
          geometries.emplace_back(font.size, charPosition, font.colour, temp->GetDrawDepth());
          auto imageIndex = m_renderer.GetImageIndexFromName(font.familyName);
          if (imageIndex < 0) ExitWithError("No image with that name found", -35);
          geometries.back().SetTextureIndex(imageIndex);
          auto textureCoords = textObj->CalculateCharTextureCoords(fontAtlasSize, content[i]);
          geometries.back().SetTextureCoords(textureCoords);
        }
        // If a text box render the cursor
        if (isTextBox) {
          TextBox* textBoxObj = dynamic_cast<TextBox*>(textObj);
          int cursorIndexPos = textBoxObj->GetCursorPosition();
          Vector2<float> finalCursorPosition {
            textObjPos.x + (font.size.x * (cursorIndexPos % charsPerLine)),
            textObjPos.y + (lineCount * font.size.y)
          };
          geometries.emplace_back(font.size, finalCursorPosition,
                                  textBoxObj->GetCursorColour(), textBoxObj->GetDrawDepth() + 1);
        }
      } 
    }
  }

  std::vector<Triangle<float, float>> tris;

  for (size_t i = 0; i < geometries.size(); i++) {
    std::array<Triangle<float, float>, 2> temp = geometries[i].GetTris();
    tris.push_back(temp[0]);
    tris.push_back(temp[1]);
  }

  std::vector<TextureArrayBounds> textureIndexArrayBounds;
  // Order each triangle based on its zIndex then convert each triangle into its vertices
  auto vertices = TriVectorToSortedVertexVector(tris, textureIndexArrayBounds);

  if (vertices.size() != 0)
    m_renderer.FillVertexBuffer(vertices, textureIndexArrayBounds);
}

