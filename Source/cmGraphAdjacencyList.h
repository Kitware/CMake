/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef cmGraphAdjacencyList_h
#define cmGraphAdjacencyList_h

#include "cmStandardIncludes.h"

struct cmGraphNodeList: public std::vector<int> {};
struct cmGraphAdjacencyList: public std::vector<cmGraphNodeList> {};

#endif
