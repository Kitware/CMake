/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#if defined(__SUNPRO_CC) || defined(__EDG__)

#  include <string>
#  include <vector>

namespace ArgumentParser {

template <typename T>
struct Maybe;
template <>
struct Maybe<std::string> : public std::string
{
  using std::string::basic_string;
  using std::string::operator=;
};

template <typename T>
struct MaybeEmpty;
#  if defined(__LCC__) && (__EDG_VERSION__ >= 603)
template <>
struct MaybeEmpty<std::vector<std::string>> : public std::vector<std::string>
{
  using std::vector<std::string>::vector;
  using std::vector<std::string>::operator=;
};
#  endif
template <typename T>
struct MaybeEmpty<std::vector<T>> : public std::vector<T>
{
  using std::vector<T>::vector;
  using std::vector<T>::operator=;
};

template <typename T>
struct NonEmpty;
template <typename T>
struct NonEmpty<std::vector<T>> : public std::vector<T>
{
  using std::vector<T>::vector;
  using std::vector<T>::operator=;
};
template <>
struct NonEmpty<std::string> : public std::string
{
  using std::string::basic_string;
  using std::string::operator=;
};

} // namespace ArgumentParser

#else

namespace ArgumentParser {

template <typename T>
struct Maybe : public T
{
  using T::T;
  using T::operator=;
};

template <typename T>
struct MaybeEmpty : public T
{
  using T::T;
  using T::operator=;
};

template <typename T>
struct NonEmpty : public T
{
  using T::T;
  using T::operator=;
};

} // namespace ArgumentParser

#endif
