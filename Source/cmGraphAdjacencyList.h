/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmGraphAdjacencyList_h
#define cmGraphAdjacencyList_h

#include "cmStandardIncludes.h"

struct cmGraphNodeList: public std::vector<int> {};
struct cmGraphAdjacencyList: public std::vector<cmGraphNodeList> {};

#endif
