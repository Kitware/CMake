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
cmDependsJava::cmDependsJava():
  m_SourceFile()
{
}

//----------------------------------------------------------------------------
cmDependsJava::cmDependsJava(const char* sourceFile):
  m_SourceFile(sourceFile)
{
}

//----------------------------------------------------------------------------
cmDependsJava::~cmDependsJava()
{
}

//----------------------------------------------------------------------------
bool cmDependsJava::WriteDependencies(std::ostream&)
{
  // Make sure this is a scanning instance.
  if(m_SourceFile == "")
    {
    cmSystemTools::Error("Cannot scan dependencies without an source file.");
    return false;
    }

  return true;
}

//----------------------------------------------------------------------------
bool cmDependsJava::CheckDependencies(std::istream&)
{
  return true;
}
