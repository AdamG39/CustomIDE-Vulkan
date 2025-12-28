#ifndef CUSTOM_SHAPES_H
#define CUSTOM_SHAPES_H

#include <array>
#include <type_traits>
#include <utility>

// Standard colours
#define COLOUR_CLEAR  Colour(0xFFFFFF, 0.0f)
#define COLOUR_BLACK  Colour(0x000000, 1.0f)
#define COLOUR_WHITE  Colour(0xFFFFFF, 1.0f)
#define COLOUR_RED    Colour(0xFF0000, 1.0f)
#define COLOUR_GREEN  Colour(0x00FF00, 1.0f)
#define COLOUR_BLUE   Colour(0x0000FF, 1.0f)

// Theme colours
  // Dark Theme
    #define THEME_DARK_COLOUR_0 Colour(0x3B1C32, 1.0f)
    #define THEME_DARK_COLOUR_1 Colour(0x1A1A1D, 1.0f)

template <typename T>
struct Vector2 {
  T x{};
  T y{};

  Vector2() = default;

  template <typename U1, typename U2,
            typename = std::enable_if_t<std::is_convertible_v<T, U1> &&
                                        std::is_convertible_v<T, U2>>>
  Vector2(U1&& X, U2&& Y)
  : x(std::forward<U1>(X)), y(std::forward<U2>(Y)) {}

  template<typename U>
  Vector2(const Vector2<U>& That)
  : x(static_cast<T>(That.x)), y(static_cast<T>(That.y)) {}

  Vector2(T X, T Y) : x(X), y(Y) {}
};

template <typename T>
struct Vector3 {
  T x, y, z;

  Vector3(): x(0), y(0), z(0) {}

  Vector3(const Vector2<T>& that) : x(that.x), y(that.y), z(0) {}

  template<typename U, typename = std::enable_if_t<!std::is_same_v<T, U>>>
  Vector3(const Vector3<U>& That)
  : x(static_cast<T>(That.x)), y(static_cast<T>(That.y)), z(static_cast<T>(That.z)) {}

  Vector3(T X, T Y, T Z) : x(X), y(Y), z(Z) {}
};

template <typename T>
struct Colour {
  T r, g, b, a;

  Colour() : r(0), g(0), b(0), a(1) {}

  Colour(int HexColour, T Alpha)
  : r(static_cast<T>(static_cast<float>((HexColour & 0xFF0000) >> 0x10) / 255.0f)),
    g(static_cast<T>(static_cast<float>((HexColour & 0x00FF00) >> 0x8) / 255.0f)),
    b(static_cast<T>(static_cast<float>(HexColour & 0x0000FF) / 255.0f)), a(Alpha) {}

  template<typename U, typename = std::enable_if_t<!std::is_same_v<T, U>>>
  Colour(const Colour<U>& That)
  : r(static_cast<T>(That.r)),
    g(static_cast<T>(That.g)),
    b(static_cast<T>(That.b)),
    a(static_cast<T>(That.a)) {}

  Colour(T R, T G, T B, T A) : r(R), g(G), b(B), a(A) {}

  Colour<float> ConvertSRGBToLinear() const {
    return Colour<float>(
        (r < 0.04045f) ? r * 0.773993808f : pow(r * 0.9478672986f + 0.0521327014f, 2.4f),
        (g < 0.04045f) ? g * 0.773993808f : pow(g * 0.9478672986f + 0.0521327014f, 2.4f),
        (b < 0.04045f) ? b * 0.773993808f : pow(b * 0.9478672986f + 0.0521327014f, 2.4f),
        (a < 0.04045f) ? a * 0.773993808f : pow(a * 0.9478672986f + 0.0521327014f, 2.4f));
  }
};

template <typename T, typename C>
struct Vertex {
  Vector3<T> position;
  Colour<C> colour;
  Vector2<T> textureCoords = {-1, -1};

  Vertex() : position(), colour(), textureCoords() {}
  
  Vertex(const Vector2<T>& Position, const Colour<C>& Colour)
  : position(Vector3(Position)), colour(Colour) {}

  Vertex(const Vector3<T>& Position, const Colour<C>& Colour)
  : position(Position), colour(Colour) {}

  template <typename Ut, typename Uc, typename = std::enable_if_t<!std::is_same_v<T, Ut> || !std::is_same_v<C, Uc>>>
  Vertex(const Vertex<Ut, Uc>& That)
  : position(That.position), colour(That.colour) {}

  void SetTextureCoords(const Vector2<T>& TextureCoords) {
    textureCoords = TextureCoords;
  }
};

template <typename T, typename C>
struct Triangle {
  std::array<Vertex<T, C>, 3> vertices;
  int zIndex;

  Triangle() : vertices(), zIndex(0) {}

  Triangle(std::array<Vertex<T, C>, 3> Vertices, int Z_Index)
  : vertices(Vertices), zIndex(Z_Index) {}

  Triangle(const Vertex<T, C>& V0, const Vertex<T, C>& V1, const Vertex<T, C>& V2, int Z_Index)
  : vertices{ V0, V1, V2 }, zIndex(Z_Index) {}

  bool operator < (const Triangle<T, C>& That) {
    return (zIndex < That.zIndex);
  }
};

template <typename T, typename C>
class Rect {
private:
  Vector2<T> m_position;
  Vector2<T> m_size;
  Colour<C> m_colour;
  int m_zIndex;

  std::array<Vertex<T, C>, 4> m_vertices;

public:
  Rect(Vector3<T> TopLeft, Vector3<T> BotRight, Colour<C> RectColour = COLOUR_BLACK, int zIndex = 0)
  : m_position(Vector2<T>((BotRight.x - TopLeft.x) / 2, (BotRight.y - TopLeft.y) / 2)),
    m_size(Vector2<T>(BotRight.x - TopLeft.x, BotRight.y - TopLeft.y)),
    m_colour(RectColour),
    m_zIndex(0),
    m_vertices{ Vertex<T, C>(TopLeft, RectColour),
                Vertex<T, C>(BotRight.x, TopLeft.y, RectColour), 
                Vertex<T, C>(TopLeft.x, BotRight.y, RectColour), 
                Vertex<T, C>(BotRight, RectColour) } {}

  Rect(Vector2<T> Size, Vector2<T> Position, Colour<C> RectColour = COLOUR_BLACK, int zIndex = 0)
  : m_position(Position),
    m_size(Size),
    m_colour(RectColour),
    m_zIndex(zIndex),
    m_vertices{ Vertex<T, C>(Vector2<T>(Position.x - (Size.x / 2), Position.y - (Size.y / 2)), RectColour),
                Vertex<T, C>(Vector2<T>(Position.x + (Size.x / 2), Position.y - (Size.y / 2)), RectColour),
                Vertex<T, C>(Vector2<T>(Position.x - (Size.x / 2), Position.y + (Size.y / 2)), RectColour),
                Vertex<T, C>(Vector2<T>(Position.x + (Size.x / 2), Position.y + (Size.y / 2)), RectColour) } {}

  void SetTextureCoords(const std::array<Vector2<T>, 4>& TextureCoords) {
    for (size_t i = 0; i < TextureCoords.size(); i++) {
      m_vertices[i].SetTextureCoords(TextureCoords[i]);
    }
  }

  std::array<Vertex<T, C>, 6> GetVertices() const {
    std::array<Vertex<T, C>, 6> vertices;

    // Triangle 1
    vertices[0] = m_vertices[0];
    vertices[1] = m_vertices[1];
    vertices[2] = m_vertices[2];

    // Triangle 2
    vertices[3] = m_vertices[1];
    vertices[4] = m_vertices[2];
    vertices[5] = m_vertices[3];

    return vertices;
  }

  std::array<Triangle<T, C>, 2> GetTris() const {
    std::array<Triangle<T, C>, 2> tris;

    tris[0] = Triangle<T, C>(m_vertices[0], m_vertices[1], m_vertices[2], m_zIndex);
    tris[1] = Triangle<T, C>(m_vertices[1], m_vertices[2], m_vertices[3], m_zIndex);

    return tris;
  }

  Vector2<T> GetSize() const { return m_size; }

  Vector2<T> GetPosition() const { return m_position; }
};

struct ArrayBounds {
  size_t start;
  size_t count;
};

struct TextureArrayBounds {
  ArrayBounds bounds;
  int textureIndex;
};

#endif

