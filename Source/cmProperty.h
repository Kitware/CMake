/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

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
