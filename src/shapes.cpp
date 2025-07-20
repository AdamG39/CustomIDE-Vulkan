#include "shapes.h"

template <typename T>
Vector2::Vector2(T X, T Y) : x(X), y(Y) {}

template <typename T>
Vector3::Vector3(T X, T Y, T Z) : x(X), y(Y), z(Z) {}
Vector3::Vector3(const Vector2<T>& that) : x(that.x), y(that.y) {}
Vector3::Vector3& operator=(const Vector2<T>& that) {
  x = that.x;
  y = that.y;
  return *this;
}
Vector3::Vector3(const Vector3<T>& that) : x(that.X), y(that.Y), z(that.Z) {}
Vector3::Vector3& operator=(const Vector3<T>& that) {
  x = that.x;
  y = that.y;
  z = that.z;
  return *this;
}

template <typename T>
Colour::Colour(T R, T G, T B) : r(R), g(G), b(B) {}

template <typename T, typename C>
Vertex::Vertex(T Loc, Colour<C> Colour) : location(Loc), colour(Colour) {}
