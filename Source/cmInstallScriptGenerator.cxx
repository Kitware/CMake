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
::cmInstallScriptGenerator(const char* script, bool code,
                           const char* component) :
  cmInstallGenerator(0, std::vector<std::string>(), component),
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
  Indent indent;
  std::string component_test =
    this->CreateComponentTest(this->Component.c_str());
  os << indent << "IF(" << component_test << ")\n";

  if(this->Code)
    {
    os << indent.Next() << this->Script << "\n";
    }
  else
    {
    os << indent.Next() << "INCLUDE(\"" << this->Script << "\")\n";
    }

  os << indent << "ENDIF(" << component_test << ")\n\n";
}
