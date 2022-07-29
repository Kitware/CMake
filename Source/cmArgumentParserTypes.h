/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#if defined(__SUNPRO_CC)

#  include <string>
#  include <vector>

namespace ArgumentParser {

template <typename T>
struct Maybe;
template <>
struct Maybe<std::string> : public std::string
{
  using std::string::basic_string;
};

template <typename T>
struct MaybeEmpty;
template <typename T>
struct MaybeEmpty<std::vector<T>> : public std::vector<T>
{
  using std::vector<T>::vector;
};

template <typename T>
struct NonEmpty;
template <typename T>
struct NonEmpty<std::vector<T>> : public std::vector<T>
{
  using std::vector<T>::vector;
};
template <>
struct NonEmpty<std::string> : public std::string
{
  using std::string::basic_string;
};

} // namespace ArgumentParser

#else

namespace ArgumentParser {

template <typename T>
struct Maybe : public T
{
  using T::T;
};

template <typename T>
struct MaybeEmpty : public T
{
  using T::T;
};

template <typename T>
struct NonEmpty : public T
{
  using T::T;
};

} // namespace ArgumentParser

#endif
