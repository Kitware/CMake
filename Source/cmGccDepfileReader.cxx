/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmGccDepfileReader.h"

#include <type_traits>
#include <utility>

#include "cmGccDepfileLexerHelper.h"

cmGccDepfileContent cmReadGccDepfile(const char* filePath)
{
  cmGccDepfileContent result;
  cmGccDepfileLexerHelper helper;
  if (helper.readFile(filePath)) {
    result = std::move(helper).extractContent();
  }
  return result;
}
