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
  if(args.size() < 2 || args.size() > 3)
    {
    this->SetError("called with incorrect number of arguments");
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
