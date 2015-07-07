/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2015 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmGlobalCommonGenerator_h
#define cmGlobalCommonGenerator_h

#include "cmGlobalGenerator.h"

/** \class cmGlobalCommonGenerator
 * \brief Common infrastructure for Makefile and Ninja global generators.
 */
class cmGlobalCommonGenerator : public cmGlobalGenerator
{
public:
  cmGlobalCommonGenerator(cmake* cm);
  ~cmGlobalCommonGenerator();
};

#endif
