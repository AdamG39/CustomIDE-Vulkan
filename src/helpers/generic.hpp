#ifndef CUSTOM_HELPERS_GENERIC_H
#define CUSTOM_HELPERS_GENERIC_H

#include <type_traits>

namespace CustomIDE {

template <typename T, typename = std::enable_if_t<std::is_enum_v<T>>>
constexpr auto to_underlying(const T& t) {
  return static_cast<std::underlying_type_t<T>>(t);
}

} // namespace CustomIDE

#endif
