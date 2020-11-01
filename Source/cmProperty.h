/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>

class cmProperty
{
public:
  enum ScopeType
  {
    TARGET,
    SOURCE_FILE,
    DIRECTORY,
    GLOBAL,
    CACHE,
    TEST,
    VARIABLE,
    CACHED_VARIABLE,
    INSTALL
  };
};

using cmProp = const std::string*;

inline const char* cmToCStr(cmProp p)
{
  return p ? p->c_str() : nullptr;
}

inline const char* cmToCStrSafe(cmProp p)
{
  return p ? p->c_str() : "";
}
