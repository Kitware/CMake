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

#include "cmSystemTools.h"

cmDependsJava::cmDependsJava()
{
}

cmDependsJava::~cmDependsJava()
{
}

bool cmDependsJava::WriteDependencies(const std::set<std::string>& sources,
                                      const std::string& /*obj*/,
                                      std::ostream& /*makeDepends*/,
                                      std::ostream& /*internalDepends*/)
{
  // Make sure this is a scanning instance.
  if (sources.empty() || sources.begin()->empty()) {
    cmSystemTools::Error("Cannot scan dependencies without an source file.");
    return false;
  }

  return true;
}

bool cmDependsJava::CheckDependencies(
  std::istream& /*internalDepends*/, const char* /*internalDependsFileName*/,
  std::map<std::string, DependencyVector>& /*validDeps*/)
{
  return true;
}
