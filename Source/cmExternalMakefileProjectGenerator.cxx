/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2007 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include <assert.h>

#include "cmExternalMakefileProjectGenerator.h"

std::string cmExternalMakefileProjectGenerator::CreateFullGeneratorName(
                                                   const char* globalGenerator,
                                                   const char* extraGenerator)
{
  std::string fullName;
  if (globalGenerator)
    {
    if (extraGenerator && *extraGenerator)
      {
      fullName = extraGenerator;
      fullName += " - ";
      }
    fullName += globalGenerator;
    }
  return fullName;
}

const char* cmExternalMakefileProjectGenerator::GetGlobalGeneratorName(
                                                          const char* fullName)
{
  // at least one global generator must be supported
  assert(!this->SupportedGlobalGenerators.empty());

  if (fullName==0)
    {
    return 0;
    }

  std::string currentName = fullName;
  // if we get only the short name, take the first global generator as default
  if (currentName == this->GetName())
    {
    return this->SupportedGlobalGenerators[0].c_str();
    }

  // otherwise search for the matching global generator
  for (std::vector<std::string>::const_iterator 
       it = this->SupportedGlobalGenerators.begin();
       it != this->SupportedGlobalGenerators.end();
       ++it)
    {
      if (this->CreateFullGeneratorName(it->c_str(), this->GetName())
                                                                == currentName)
      {
        return it->c_str();
      }
    }
  return 0;
}
