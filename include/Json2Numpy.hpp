#pragma once

#include <type_traits>

#include <nlohmann/json.hpp>

struct type {
  template<typename T>
  using base_type = typename std::remove_cv<typename std::remove_reference<T>::type>::type;
  template<typename T>
  static constexpr const char *dtype() {
    if constexpr(std::is_same<base_type<T>, char>::value) { return "u1"; }
    if constexpr(std::is_same<base_type<T>, std::uint8_t>::value) { return "u1"; }
    else if constexpr(std::is_same<base_type<T>, std::uint16_t>::value) { return "u2"; }
    else if constexpr(std::is_same<base_type<T>, std::uint32_t>::value) { return "u4"; }
    else if constexpr(std::is_same<base_type<T>, std::uint64_t>::value) { return "u8"; }
    else if constexpr(std::is_same<base_type<T>, std::int8_t>::value) { return "i1"; }
    else if constexpr(std::is_same<base_type<T>, std::int16_t>::value) { return "i2"; }
    else if constexpr(std::is_same<base_type<T>, std::int32_t>::value) { return "i4"; }
    else if constexpr(std::is_same<base_type<T>, std::int64_t>::value) { return "i8"; }
    else if constexpr(std::is_same<base_type<T>, float>::value) { return "f4"; }
    else if constexpr(std::is_same<base_type<T>, double>::value) { return "f8"; }
    else if constexpr(std::is_same<base_type<T>, long double>::value) { return "f8"; }
    else { return "unknown type"; }
  }
};

