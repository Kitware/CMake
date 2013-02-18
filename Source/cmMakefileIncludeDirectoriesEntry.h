/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2012 Stephen Kelly <steveire@gmail.com>

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#ifndef cmMakefileIncludeDirectoriesEntry_h
#define cmMakefileIncludeDirectoriesEntry_h

#include <string>
#include "cmListFileCache.h"

struct cmMakefileIncludeDirectoriesEntry {
  cmMakefileIncludeDirectoriesEntry(const std::string &value,
                          const cmListFileBacktrace &bt)
    : Value(value), Backtrace(bt)
  {}
  std::string Value;
  cmListFileBacktrace Backtrace;
};

#endif
