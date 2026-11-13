// -*-c++-*-
// vim: set ft=cpp:

/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */

#pragma once

#include <iterator> // IWYU pragma: keep

#if __cplusplus < 201703L || defined(_MSVC_LANG) && _MSVC_LANG < 201703L
#  include <initializer_list>
#endif
#if __cplusplus < 202002L || defined(_MSVC_LANG) && _MSVC_LANG < 202002L
#  include <cstddef>
#  include <type_traits>
#endif

namespace cm {

using std::begin;
using std::end;

#if __cplusplus < 201703L || defined(_MSVC_LANG) && _MSVC_LANG < 201703L

template <typename C>
inline constexpr auto size(C const& c) noexcept(noexcept(c.size()))
  -> decltype(c.size())
{
  return c.size();
}

template <typename T, std::size_t N>
inline constexpr std::size_t size(T const (&)[N]) noexcept
{
  return N;
}

template <typename C>
inline constexpr auto empty(C const& c) noexcept(noexcept(c.empty()))
  -> decltype(c.empty())
{
  return c.empty();
}

template <typename T, std::size_t N>
inline constexpr bool empty(T const (&)[N]) noexcept
{
  return false;
}

template <typename E>
inline constexpr bool empty(std::initializer_list<E> il) noexcept
{
  return il.size() == 0;
}

template <typename C>
inline constexpr auto data(C& c) noexcept(noexcept(c.data()))
  -> decltype(c.data())
{
  return c.data();
}

template <typename C>
inline constexpr auto data(C const& c) noexcept(noexcept(c.data()))
  -> decltype(c.data())
{
  return c.data();
}

template <typename T, std::size_t N>
inline constexpr T* data(T (&array)[N]) noexcept
{
  return array;
}

template <typename E>
inline constexpr E const* data(std::initializer_list<E> il) noexcept
{
  return il.begin();
}

#else

using std::size;
using std::empty;
using std::data;

#endif

#if __cplusplus < 202002L || defined(_MSVC_LANG) && _MSVC_LANG < 202002L

template <typename C>
inline constexpr auto ssize(C const& c)
  -> std::common_type_t<std::ptrdiff_t, std::make_signed_t<decltype(c.size())>>
{
  using signed_type = std::make_signed_t<decltype(c.size())>;
  using result_type = std::common_type_t<std::ptrdiff_t, signed_type>;

  return static_cast<result_type>(c.size());
}

template <typename T, std::ptrdiff_t N>
inline constexpr std::ptrdiff_t ssize(T const (&)[N]) noexcept
{
  return N;
}

#else

using std::ssize;

#endif

} // namespace cm
