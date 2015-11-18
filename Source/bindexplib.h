/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#ifndef bindexplib_h
#define bindexplib_h

#include "cmStandardIncludes.h"


class bindexplib
{
public:
  bindexplib() {}
  bool AddObjectFile(const char* filename);
  void WriteFile(FILE* file);
private:
  std::set<std::string> Symbols;
  std::set<std::string> DataSymbols;
};
#endif
