/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "cmOptionCommand.h"

// cmOptionCommand
bool cmOptionCommand::InitialPass(std::vector<std::string> const& args)
{
  bool argError = false;
  if(args.size() < 2)
    {
    argError = true;
    }
  // for VTK 4.0 we have to support the option command with more than 3 arguments
  // if CMAKE_MINIMUM_REQUIRED_VERSION is not defined, if CMAKE_MINIMUM_REQUIRED_VERSION
  // is defined, then we can have stricter checking.
  if(m_Makefile->GetDefinition("CMAKE_MINIMUM_REQUIRED_VERSION"))
    {
    if(args.size() > 3)
      {
      argError = true;
      }
    }
  if(argError)
    {
    std::string m = "called with incorrect number of arguments: ";
    for(size_t i =0; i < args.size(); ++i)
      {
      m += args[i];
      m += " ";
      }
    this->SetError(m.c_str());
    return false;
    }
  
  // Now check and see if the value has been stored in the cache
  // already, if so use that value and don't look for the program
  const char* cacheValue
    = m_Makefile->GetDefinition(args[0].c_str());
  if(!cacheValue)
    {
    std::string initialValue = "Off";
    if(args.size() == 3)
      {
      initialValue = args[2];
      }
    m_Makefile->AddCacheDefinition(args[0].c_str(),
                                   cmSystemTools::IsOn(initialValue.c_str()),
                                   args[1].c_str());
    }
  return true;
}
