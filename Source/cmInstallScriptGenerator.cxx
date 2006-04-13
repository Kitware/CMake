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
#include "cmInstallScriptGenerator.h"

//----------------------------------------------------------------------------
cmInstallScriptGenerator
::cmInstallScriptGenerator(const char* script, bool code):
  Script(script), Code(code)
{
}

//----------------------------------------------------------------------------
cmInstallScriptGenerator
::~cmInstallScriptGenerator()
{
}

//----------------------------------------------------------------------------
void cmInstallScriptGenerator::GenerateScript(std::ostream& os)
{
  if(this->Code)
    {
    os << this->Script << "\n";
    }
  else
    {
    os << "INCLUDE(\"" << this->Script << "\")\n";
    }
}
