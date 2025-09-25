/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>

/** Standard documentation entry for cmDocumentation's formatting.  */
struct cmDocumentationEntry
{
#if __cplusplus <= 201103L
  cmDocumentationEntry(std::string const& name, std::string const& brief)
    : Name{ name }
    , Brief{ brief }
  {
  }
#endif

  std::string Name;
  std::string Brief;
  char CustomNamePrefix = ' ';
};
