/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmProperty_h
#define cmProperty_h

namespace cmProperty
{
  enum ScopeType { TARGET, SOURCE_FILE, DIRECTORY, GLOBAL, CACHE,
                   TEST, VARIABLE, CACHED_VARIABLE };
}

#endif
