/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2014 Kitware, Inc.

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#ifndef cmWIXFilesShortcut_h
#define cmWIXFilesShortcut_h

#include <string>

struct cmWIXShortcut
{
  cmWIXShortcut()
    :desktop(false)
    {}

  std::string textLabel;
  std::string workingDirectoryId;
  bool desktop;
};

#endif
