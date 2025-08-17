#include "uilib.h"
#include <stdlib.h>

UIElement* CreateUIElement(enum UIType Type, UIElement* Parent, UIElement** Children, uint32_t ChildCount) {
  UIElement* temp = (UIElement*)malloc(sizeof(UIElement));
  temp->type = Type;
  temp->parent = Parent;
  temp->children = Children;
  temp->childCount = ChildCount;
  return temp;
}

void DestroyUIElement(UIElement* Element) {
  free(Element->parent);
  for (uint_fast32_t i = 0; i < Element->childCount; i++) {
    free(Element->children[i]);
  }
  free(Element->children);
}

