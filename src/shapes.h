#pragma once

#include <array>

template <typename T>
class Vector2 {
  T x = 0;
  T y = 0;

  Vector2(T X, T Y);
};

template <typename T>
class Vector3 {
  T x = 0;
  T y = 0;
  T z = 0;

  Vector3(T X, T Y, T Z);
  Vector3(const Vector2<T>& that);
  Vector3& operator=(const Vector2<T>& that);
  Vector3(const Vector3<T>& that);
  Vector3& operator=(const Vector3<T>& that);
};

template <typename T>
class Colour {
  T r;
  T g;
  T b;

  Colour(T R, T G, T B);
};

template <typename T, typename C>
class Vertex {
  T* location = nullptr;
  Colour<C>* colour = nullptr;

  Vertex(T Loc, Colour<C> Colour);
};

template <typename T, typename C>
class Triangle {
  Vertex<T, C> points[3];

  Triangle(Vertex<T, C> p0, Vertex<T, C> p1, Vertex<T, C> p2) {
    points[0] = p0;
    points[1] = p1;
    points[2] = p2;
  }

  Triangle(Vector3<T> v0, Vector3<T> v1, Vector3<T> v2, Colour<C> colour) {
    points[0] = new Vertex<T, C>(v0, colour);
    points[1] = new Vertex<T, C>(v1, colour);
    points[2] = new Vertex<T, C>(v2, colour);
  }

  ~Triangle() {
    delete[] points;
  }
};

template <typename T, typename C>
class Rect {
  private:
  std::array<Vertex<T, C>, 4> points; // 0 = topLeft, 1 = topRight, 2 = botLeft, 3 = botRight

  public:
  std::array<Triangle<T, C>, 2> tris;

  Rect(Vector3<T> topLeft, Vector3<T> botRight, Colour<C> colour) {
    Vector3<T>* topRight = new Vector3<T>(botRight.X, topLeft.Y);
    Vector3<T>* botLeft = new Vector3<T>(topLeft.X, botRight.Y);

    points[0] = new Vertex<T, C>(topLeft, colour);
    points[1] = new Vertex<T, C>(*topRight, colour);
    points[2] = new Vertex<T, C>(*botLeft, colour);
    points[3] = new Vertex<T, C>(botRight, colour);

    ConstructTrisFromPoints();
  }

  Rect(T height, T width, Vector3<T> offset, Colour<C> colour) {
    points[0] = new Vertex<T, C>(offset, colour); 
    points[1] = new Vertex<T, C>(new Vector3<T>(offset.x + width, offset.y, offset.z), colour);
    points[2] = new Vertex<T, C>(new Vector3<T>(offset.x, offset.y + height, offset.z), colour);
    points[3] = new Vertex<T, C>(new Vector3<T>(offset.x + width, offset.y + height, offset.z), colour);

    ConstructTrisFromPoints();
  }

  std::array<Vertex<T, C>*, 4> GetPoints() {
    return points;
  }

  std::array<Vertex<T, C>*, 6> GetTriPoints() {
    std::array<Vertex<T, C>*, 6> triPoints;

    triPoints[0] = tris[0]->points[0];
    triPoints[1] = tris[0];
    triPoints[2] = tris[0];
    triPoints[3] = tris[1];
    triPoints[4] = tris[1];
    triPoints[5] = tris[1];
  }

  void ConstructTrisFromPoints() {
    tris[0] = Triangle<T, C>(points[0], points[1], points[2]);
    tris[1] = Triangle<T, C>(points[1], points[2], points[3]);
  }

  ~Rect() {
    delete[] points;
    delete[] tris;
  }
};
