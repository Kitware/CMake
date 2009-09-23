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
