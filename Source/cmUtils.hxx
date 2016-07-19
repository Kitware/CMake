/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2016 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmUtils_hxx
#define cmUtils_hxx

#include <cmsys/SystemTools.hxx>

// Use the make system's VERBOSE environment variable to enable
// verbose output. This can be skipped by also setting CMAKE_NO_VERBOSE
// (which is set by the Eclipse and KDevelop generators).
inline bool isCMakeVerbose()
{
  return (cmSystemTools::HasEnv("VERBOSE") &&
          !cmSystemTools::HasEnv("CMAKE_NO_VERBOSE"));
}

#endif
