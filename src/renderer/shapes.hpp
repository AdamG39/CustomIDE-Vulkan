#ifndef CUSTOM_SHAPES_H
#define CUSTOM_SHAPES_H

#include <array>
#include <type_traits>
#include <utility>
#include <vector>
#include <cmath>
#include <cstdint>

// Standard colours
#define COLOUR_CLEAR  Colour(0xFFFFFF, 0.f)
#define COLOUR_BLACK  Colour(0x000000, 1.f)
#define COLOUR_WHITE  Colour(0xFFFFFF, 1.f)
#define COLOUR_RED    Colour(0xFF0000, 1.f)
#define COLOUR_GREEN  Colour(0x00FF00, 1.f)
#define COLOUR_BLUE   Colour(0x0000FF, 1.f)

// Theme colours
  // Dark Theme
    #define THEME_DARK_COLOUR_0 Colour(0x3B1C32, 1.f)
    #define THEME_DARK_COLOUR_1 Colour(0x1A1A1D, 1.f)

namespace CustomIDE {

template <typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
struct Vector2 {
  T x{};
  T y{};

  Vector2() = default;

  constexpr Vector2(T X, T Y) : x(X), y(Y) {}

  constexpr Vector2 operator+(const Vector2& Other) const {
    return { this->x + Other.x, this->y + Other.y };
  }
  constexpr Vector2 operator-(const Vector2& Other) const {
    return { this->x - Other.x, this->y -= Other.y };
  }
  constexpr Vector2 operator*(const Vector2& Other) const {
    return { this->x * Other.x, this->y * Other.y };
  }
  constexpr Vector2 operator/(const Vector2& Other) const {
    return { this->x / Other.x, this->y / Other.y };
  }

  constexpr bool operator==(const Vector2& Other) const {
    return (this->x == Other.x) && (this->y == Other.y);
  }
  constexpr bool operator!=(const Vector2& Other) const {
    return (this->x != Other.x) || (this->y != Other.y);
  }

  template <typename U, typename = std::enable_if_t<std::is_arithmetic_v<U> && std::is_convertible_v<U, T>>>
  constexpr Vector2 operator+(U Other) const {
    return { this->x + static_cast<T>(Other), this->y + static_cast<T>(Other) };
  }
  template <typename U, typename = std::enable_if_t<std::is_arithmetic_v<U> && std::is_convertible_v<U, T>>>
  constexpr Vector2 operator-(U Other) const {
    return { this->x - static_cast<T>(Other), this->y - static_cast<T>(Other) };
  }
  template <typename U, typename = std::enable_if_t<std::is_arithmetic_v<U> && std::is_convertible_v<U, T>>>
  constexpr Vector2 operator*(U Other) const {
    return { this->x * static_cast<T>(Other), this->y * static_cast<T>(Other) };
  }
  template <typename U, typename = std::enable_if_t<std::is_arithmetic_v<U> && std::is_convertible_v<U, T>>>
  constexpr Vector2 operator/(U Other) const {
    return { this->x / static_cast<T>(Other), this->y / static_cast<T>(Other) };
  }
};

using Vector2D = Vector2<float>;

template <typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
struct Vector3 {
  T x{};
  T y{};
  T z{};

  Vector3() = default;

  constexpr Vector3(T X, T Y, T Z) : x(X), y(Y), z(Z) {}

  constexpr Vector3(const Vector2<T>& That) : x(That.x), y(That.y), z(0) {}

  constexpr Vector3 operator+(const Vector2<T>& Other) const {
    return { this->x + Other.x, this->y + Other.y, this->z };
  }
  constexpr Vector3 operator+(const Vector3& Other) const {
    return { this->x + Other.x, this->y + Other.y, this->z + Other.z };
  }
  constexpr Vector3 operator-(const Vector2<T>& Other) const {
    return { this->x - Other.x, this->y - Other.y, this->z };
  }
  constexpr Vector3 operator-(const Vector3& Other) const {
    return { this->x - Other.x, this->y - Other.y, this->z - Other.z };
  }
  constexpr Vector3 operator*(const Vector2<T>& Other) const {
    return { this->x * Other.x, this->y * Other.y, this->z };
  }
  constexpr Vector3 operator*(const Vector3& Other) const {
    return { this->x * Other.x, this->y * Other.y, this->z * Other.z };
  }
  constexpr Vector3 operator/(const Vector2<T>& Other) const {
    return { this->x / Other.x, this->y / Other.y, this->z };
  }
  constexpr Vector3 operator/(const Vector3& Other) const {
    return { this->x / Other.x, this->y / Other.y, this->z / Other.z };
  }

  constexpr bool operator==(const Vector3& Other) const {
    return (this->x == Other.x) && (this->y == Other.y) && (this->z == Other.z);
  }
  constexpr bool operator!=(const Vector3& Other) const {
    return (this->x != Other.x) && (this->y != Other.y) && (this->z != Other.z);
  }

  template <typename U, typename = std::enable_if_t<std::is_arithmetic_v<U> && std::is_convertible_v<U, T>>>
  constexpr Vector3 operator+(U Other) const {
    return { this->x + static_cast<T>(Other), this->y + static_cast<T>(Other), this->z + static_cast<T>(Other) };
  }
  template <typename U, typename = std::enable_if_t<std::is_arithmetic_v<U> && std::is_convertible_v<U, T>>>
  constexpr Vector3 operator-(U Other) const {
    return { this->x - static_cast<T>(Other), this->y - static_cast<T>(Other), this->z - static_cast<T>(Other) };
  }
  template <typename U, typename = std::enable_if_t<std::is_arithmetic_v<U> && std::is_convertible_v<U, T>>>
  constexpr Vector3 operator*(U Other) const {
    return { this->x * static_cast<T>(Other), this->y * static_cast<T>(Other), this->z * static_cast<T>(Other) };
  }
  template <typename U, typename = std::enable_if_t<std::is_arithmetic_v<U> && std::is_convertible_v<U, T>>>
  constexpr Vector3 operator/(U Other) const {
    return { this->x / static_cast<T>(Other), this->y / static_cast<T>(Other), this->z / static_cast<T>(Other) };
  }
};

using Vector3D = Vector3<float>;

template <typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
struct Vector4 {
  T x{};
  T y{};
  T z{};
  T w{};

  Vector4() = default;

  constexpr Vector4(T X, T Y, T Z, T W) : x(X), y(Y), z(Z), w(W) {}

  constexpr Vector4(const Vector2<T>& That) : x(That.x), y(That.y), z(0), w(0) {}

  constexpr Vector4(const Vector3<T>& That) : x(That.x), y(That.y), z(That.z), w(0) {}
};

using Vector4D = Vector4<float>;

struct Colour {
  float r, g, b, a;

  Colour() : r(0), g(0), b(0), a(1) {}

  Colour(int HexColour, float Alpha)
  : r(((HexColour & 0xFF0000) >> 0x10) / 255.0f),
    g(((HexColour & 0x00FF00) >> 0x8) / 255.0f),
    b((HexColour & 0x0000FF) / 255.0f),
    a(Alpha) {}

  Colour(float R, float G, float B, float A) : r(R), g(G), b(B), a(A) {}

  Colour ConvertSRGBToLinear() const {
    return Colour(
        (r < 0.04045f) ? r * 0.773993808f : pow(r * 0.9478672986f + 0.0521327014f, 2.4f),
        (g < 0.04045f) ? g * 0.773993808f : pow(g * 0.9478672986f + 0.0521327014f, 2.4f),
        (b < 0.04045f) ? b * 0.773993808f : pow(b * 0.9478672986f + 0.0521327014f, 2.4f),
        (a < 0.04045f) ? a * 0.773993808f : pow(a * 0.9478672986f + 0.0521327014f, 2.4f));
  }
};

template <typename T>
struct Vertex {
  Vector3<T> position;
  Colour colour;
  Vector2<T> textureCoords;

  Vertex() : position(), colour(), textureCoords() {}
  
  Vertex(const Vector2<T>& Position, const Colour& Colour)
  : position(Vector3(Position)), colour(Colour) {}

  Vertex(const Vector3<T>& Position, const Colour& Colour)
  : position(Position), colour(Colour) {}

  template <typename Ut, typename = std::enable_if_t<!std::is_same_v<T, Ut>>>
  Vertex(const Vertex<Ut>& That)
  : position(That.position), colour(That.colour) {}

  void SetTextureCoords(const Vector2<T>& TextureCoords) {
    textureCoords = TextureCoords;
  }
};

using TextureID = int;

struct Rect2D {
  int32_t xOffset, yOffset;
  uint32_t width, height;

  bool operator==(const Rect2D& Other) {
    return this->xOffset == Other.xOffset && this->yOffset == Other.yOffset &&
           this->width == Other.width && this->height == Other.height;
  }

  bool PointIntersection(Vector2D Point) {
    int top = yOffset - (height / 2);
    int bottom = yOffset + (height / 2);
    int left = xOffset - (width / 2);
    int right = xOffset + (width / 2);

    return Point.x >= left && Point.x <= right &&
           Point.y >= top && Point.y <= bottom;
  }
};

struct UVRect2D {
  float xOffset, yOffset;
  float width, height;

  bool operator==(const UVRect2D& Other) {
    return this->xOffset == Other.xOffset && this->yOffset == Other.yOffset &&
           this->width == Other.width && this->height == Other.height;
  }
};

struct ClipRect {
  bool clippingEnabled;
  Rect2D rect;

  bool operator==(const ClipRect& Other) {
    return this->clippingEnabled == Other.clippingEnabled && this->rect == Other.rect;
  }
};

} // namespace CustomIDE

namespace Vulkan {

struct DrawCommand {
  CustomIDE::TextureID texture;
  CustomIDE::Rect2D transformRect;
  CustomIDE::UVRect2D uvRect;
  CustomIDE::Colour colour;

  CustomIDE::ClipRect clipRect;
  int zIndex;
};

using DrawBatch = std::vector<DrawCommand>;

} // namespace Renderer

#endif

