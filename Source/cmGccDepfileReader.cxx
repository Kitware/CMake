/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmGccDepfileReader.h"

#include <type_traits>
#include <utility>

#include <cm/optional>

#include "cmGccDepfileLexerHelper.h"

cm::optional<cmGccDepfileContent> cmReadGccDepfile(const char* filePath)
{
  cmGccDepfileLexerHelper helper;
  if (helper.readFile(filePath)) {
    return cm::make_optional(std::move(helper).extractContent());
  }
  return cm::nullopt;
}
