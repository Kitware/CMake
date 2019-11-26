/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cm_static_string_view_hxx
#define cm_static_string_view_hxx

#include "cmConfigure.h" // IWYU pragma: keep

#include <cstddef>

#include <cm/string_view>

namespace cm {

/** A string_view that only binds to static storage.
 *
 * This is used together with the `""_s` user-defined literal operator
 * to construct a type-safe abstraction of a string_view that only views
 * statically allocated strings.  These strings are const and available
 * for the entire lifetime of the program.
 */
class static_string_view : public string_view
{
  static_string_view(string_view v)
    : string_view(v)
  {
  }

  friend static_string_view operator"" _s(const char* data, size_t size);
};

/** Create a static_string_view using `""_s` literal syntax.  */
inline static_string_view operator"" _s(const char* data, size_t size)
{
  return string_view(data, size);
}

} // namespace cm

using cm::operator"" _s;

#endif
