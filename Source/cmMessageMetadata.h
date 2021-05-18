/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmsys/Terminal.h"

struct cmMessageMetadata
{
  const char* title = nullptr;
  int desiredColor = cmsysTerminal_Color_Normal;
};
