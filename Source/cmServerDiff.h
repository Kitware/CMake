/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2015 Stephen Kelly <steveire@gmail.com>

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#pragma once

#include "cmStandardIncludes.h"

#if defined(CMAKE_BUILD_WITH_CMAKE)
#include "cm_jsoncpp_value.h"
#endif

struct Chunk
{
  long OrigStart = 1;
  long NewStart = 1;
  long NumCommon = 0;
  long NumRemoved = 0;
  long NumAdded = 0;
};

struct DifferentialFileContent
{
  std::vector<std::string> OrigLines;
  std::vector<std::string> EditorLines;
  std::vector<Chunk> Chunks;
};

class cmServerDiff
{
public:
  static DifferentialFileContent GetDiff(Json::Value value);
};
