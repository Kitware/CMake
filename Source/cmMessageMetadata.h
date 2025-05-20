/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmStdIoTerminal.h"

struct cmMessageMetadata
{
  char const* title = nullptr;
  cm::StdIo::TermAttrSet attrs = cm::StdIo::TermAttr::Normal;
};
