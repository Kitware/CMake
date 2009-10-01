/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmDependsJava.h"

#include "cmDependsJavaParserHelper.h"
#include "cmSystemTools.h"

//----------------------------------------------------------------------------
cmDependsJava::cmDependsJava()
{
}

//----------------------------------------------------------------------------
cmDependsJava::~cmDependsJava()
{
}

//----------------------------------------------------------------------------
bool cmDependsJava::WriteDependencies(const char *src, const char *,
  std::ostream&, std::ostream&)
{
  // Make sure this is a scanning instance.
  if(!src || src[0] == '\0')
    {
    cmSystemTools::Error("Cannot scan dependencies without an source file.");
    return false;
    }

  return true;
}

bool cmDependsJava::CheckDependencies(std::istream&,
                             std::map<std::string, DependencyVector >&)
{
  return true;
}
