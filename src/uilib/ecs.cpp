#include <stack>
#include "ecs.hpp"

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
      Label* label = top->GetComponent<Label>();
      // An entity requires a renderable component and a transform to be renderered
      if (transform == nullptr) continue; // Just skip this entity since it cant be renderered

      if (staticColour != nullptr) {
        geometries.emplace_back(transform->GetPixelSize(), transform->GetPixelPosition(), staticColour->GetColour());
      } else if (texture != nullptr) {
        geometries.emplace_back(transform->GetPixelSize(), transform->GetPixelPosition(), COLOUR_WHITE);
        geometries.back().SetTextureCoords(texture->GetTextureCoords());
        geometries.back().SetTextureIndex(texture->GetTextureIndex());
      } else if (label != nullptr) {
        // calculate size of each character based on font
        Font font = label->GetFont();
        Vector2 labelPos = transform->GetPixelPosition();
        Vector2<float> charSize {(float)font.size, (float)font.size};
        Vector2<float> fontAtlasSize {64, 2};
        std::string content = label->GetContent();
        int charsPerLine = transform->GetPixelSize().x / charSize.x;
        // create a rect for each character
        for (size_t i = 0; i < content.size(); i++) {
          Vector2<float> charPosition {
            labelPos.x + (charSize.x * (i % charsPerLine)),
            labelPos.y + (((i >= charsPerLine) ? int(i / charsPerLine) + 1 : 1) * charSize.y)
          };
          geometries.emplace_back(charSize, charPosition, COLOUR_WHITE);
          geometries.back().SetTextureIndex(4);
          auto textureCoords = label->CalculateCharTextureCoords(fontAtlasSize, content[i]);
          geometries.back().SetTextureCoords(textureCoords);
        }
      }
      // Dont do anything if either condition isnt met
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

