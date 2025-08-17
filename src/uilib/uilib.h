#ifndef CUSTOM_UI_LIB_H
#define CUSTOM_UI_LIB_H
#endif

#include <stdint.h>

/* Elements to create: 
 * Panel
 * Button
 * Label
 * Dropdown
 */

/* Geometry types to create:
 * Triangle
 * Rectangle
 * Text
 */

enum UIType { Panel, Button, Label, Dropdown };

typedef struct Vector2 {
  float x, y;
} Vector2;

typedef struct UIElement UIElement;

struct UIElement {
  enum UIType type;
  UIElement* parent;
  UIElement** children;
  uint32_t childCount;
};

UIElement* CreateUIElement(enum UIType Type, UIElement* Parent, UIElement** Children, uint32_t ChildCount);

void DestroyUIElement(UIElement* Element);

