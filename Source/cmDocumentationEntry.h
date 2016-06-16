/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmDocumentationEntry_h
#define cmDocumentationEntry_h

#include <cmConfigure.h> // IWYU pragma: keep

#include <string>

/** Standard documentation entry for cmDocumentation's formatting.  */
struct cmDocumentationEntry
{
  std::string Name;
  std::string Brief;
  cmDocumentationEntry() {}
  cmDocumentationEntry(const char* doc[2])
  {
    if (doc[0]) {
      this->Name = doc[0];
    }
    if (doc[1]) {
      this->Brief = doc[1];
    }
  }
  cmDocumentationEntry(const char* n, const char* b)
  {
    if (n) {
      this->Name = n;
    }
    if (b) {
      this->Brief = b;
    }
  }
};

#endif
