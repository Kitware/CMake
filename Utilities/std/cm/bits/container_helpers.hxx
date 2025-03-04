// -*-c++-*-
// vim: set ft=cpp:

/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */

#pragma once

#include <iterator> // IWYU pragma: keep

#if __cplusplus < 201402L || defined(_MSVC_LANG) && _MSVC_LANG < 201402L
#  include <initializer_list>
#endif
#if __cplusplus < 202002L || defined(_MSVC_LANG) && _MSVC_LANG < 202002L
#  include <cstddef>
#  include <type_traits>
#endif

namespace cm {

using std::begin;
using std::end;

#if __cplusplus < 201402L || defined(_MSVC_LANG) && _MSVC_LANG < 201402L

template <typename C>
#  if defined(_MSC_VER) && _MSC_VER < 1900
inline auto cbegin(C const& c)
#  else
inline constexpr auto cbegin(C const& c) noexcept(noexcept(std::begin(c)))
#  endif
  -> decltype(std::begin(c))
{
  return std::begin(c);
}

template <typename C>
#  if defined(_MSC_VER) && _MSC_VER < 1900
inline auto cend(C const& c)
#  else
inline constexpr auto cend(C const& c) noexcept(noexcept(std::end(c)))
#  endif
  -> decltype(std::end(c))
{
  return std::end(c);
}

template <typename C>
#  if defined(_MSC_VER) && _MSC_VER < 1900
inline auto rbegin(C& c)
#  else
inline constexpr auto rbegin(C& c)
#  endif
  -> decltype(c.rbegin())
{
  return c.rbegin();
}
template <typename C>
#  if defined(_MSC_VER) && _MSC_VER < 1900
inline auto rbegin(C const& c)
#  else
inline constexpr auto rbegin(C const& c)
#  endif
  -> decltype(c.rbegin())
{
  return c.rbegin();
}
template <typename T, std::size_t N>
#  if defined(_MSC_VER) && _MSC_VER < 1900
inline std::reverse_iterator<T*> rbegin(T (&array)[N])
#  else
inline constexpr std::reverse_iterator<T*> rbegin(T (&array)[N]) noexcept
#  endif
{
  return std::reverse_iterator<T*>(array + N);
}
template <typename T>
#  if defined(_MSC_VER) && _MSC_VER < 1900
inline std::reverse_iterator<T const*> rbegin(std::initializer_list<T> il)
#  else
inline constexpr std::reverse_iterator<T const*> rbegin(
  std::initializer_list<T> il) noexcept
#  endif
{
  return std::reverse_iterator<T const*>(il.end());
}

template <typename C>
#  if defined(_MSC_VER) && _MSC_VER < 1900
inline auto rend(C& c)
#  else
inline constexpr auto rend(C& c)
#  endif
  -> decltype(c.rend())

{
  return c.rend();
}
template <typename C>
#  if defined(_MSC_VER) && _MSC_VER < 1900
inline auto rend(C const& c)
#  else
inline constexpr auto rend(C const& c)
#  endif
  -> decltype(c.rend())
{
  return c.rend();
}
template <typename T, std::size_t N>
#  if defined(_MSC_VER) && _MSC_VER < 1900
inline std::reverse_iterator<T*> rend(T (&array)[N])
#  else
inline constexpr std::reverse_iterator<T*> rend(T (&array)[N]) noexcept
#  endif
{
  return std::reverse_iterator<T*>(array);
}
template <typename T>
#  if defined(_MSC_VER) && _MSC_VER < 1900
inline std::reverse_iterator<T const*> rend(std::initializer_list<T> il)
#  else
inline constexpr std::reverse_iterator<T const*> rend(
  std::initializer_list<T> il) noexcept
#  endif
{
  return std::reverse_iterator<T const*>(il.begin());
}

template <typename C>
#  if defined(_MSC_VER) && _MSC_VER < 1900
inline auto crbegin(C const& c)
#  else
inline constexpr auto crbegin(C const& c)
#  endif
  -> decltype(cm::rbegin(c))
{
  return cm::rbegin(c);
}

template <typename C>
#  if defined(_MSC_VER) && _MSC_VER < 1900
inline auto crend(C const& c)
#  else
inline constexpr auto crend(C const& c)
#  endif
  -> decltype(cm::rend(c))
{
  return cm::rend(c);
}

#else

using std::cbegin;
using std::cend;

using std::rbegin;
using std::rend;

using std::crbegin;
using std::crend;

#endif

#if __cplusplus < 201703L || defined(_MSVC_LANG) && _MSVC_LANG < 201703L

template <typename C>
#  if defined(_MSC_VER) && _MSC_VER < 1900
inline auto size(C const& c)
#  else
inline constexpr auto size(C const& c) noexcept(noexcept(c.size()))
#  endif
  -> decltype(c.size())
{
  return c.size();
}

template <typename T, std::size_t N>
#  if defined(_MSC_VER) && _MSC_VER < 1900
inline std::size_t size(T const (&)[N])
#  else
inline constexpr std::size_t size(T const (&)[N]) noexcept
#  endif
{
  return N;
}

template <typename C>
#  if defined(_MSC_VER) && _MSC_VER < 1900
inline auto empty(C const& c)
#  else
inline constexpr auto empty(C const& c) noexcept(noexcept(c.empty()))
#  endif
  -> decltype(c.empty())
{
  return c.empty();
}

template <typename T, std::size_t N>
#  if defined(_MSC_VER) && _MSC_VER < 1900
inline bool empty(T const (&)[N])
#  else
inline constexpr bool empty(T const (&)[N]) noexcept
#  endif
{
  return false;
}

template <typename E>
#  if defined(_MSC_VER) && _MSC_VER < 1900
inline bool empty(std::initializer_list<E> il)
#  else
inline constexpr bool empty(std::initializer_list<E> il) noexcept
#  endif
{
  return il.size() == 0;
}

template <typename C>
#  if defined(_MSC_VER) && _MSC_VER < 1900
inline auto data(C& c) -> decltype(c.data())
#  else
inline constexpr auto data(C& c) noexcept(noexcept(c.data()))
#  endif
                         -> decltype(c.data())
{
  return c.data();
}

template <typename C>
#  if defined(_MSC_VER) && _MSC_VER < 1900
inline auto data(C const& c)
#  else
inline constexpr auto data(C const& c) noexcept(noexcept(c.data()))
#  endif
  -> decltype(c.data())
{
  return c.data();
}

template <typename T, std::size_t N>
#  if defined(_MSC_VER) && _MSC_VER < 1900
inline T* data(T (&array)[N])
#  else
inline constexpr T* data(T (&array)[N]) noexcept
#  endif
{
  return array;
}

template <typename E>
#  if defined(_MSC_VER) && _MSC_VER < 1900
inline E const* data(std::initializer_list<E> il)
#  else
inline constexpr E const* data(std::initializer_list<E> il) noexcept
#  endif
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
#  if defined(_MSC_VER) && _MSC_VER < 1900
inline auto ssize(C const& c)
#  else
inline constexpr auto ssize(C const& c)
#  endif
  -> typename std::common_type<
    std::ptrdiff_t, typename std::make_signed<decltype(c.size())>::type>::type
{
  using signed_type = typename std::make_signed<decltype(c.size())>::type;
  using result_type =
    typename std::common_type<std::ptrdiff_t, signed_type>::type;

  return static_cast<result_type>(c.size());
}

template <typename T, std::ptrdiff_t N>
#  if defined(_MSC_VER) && _MSC_VER < 1900
inline std::ptrdiff_t ssize(T const (&)[N])
#  else
inline constexpr std::ptrdiff_t ssize(T const (&)[N]) noexcept
#  endif
{
  return N;
}

#else

using std::ssize;

#endif

} // namespace cm
