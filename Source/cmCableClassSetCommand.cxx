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
#include "cmCableClassSetCommand.h"
#include "cmCacheManager.h"
#include "cmTarget.h"

// cmCableClassSetCommand
bool cmCableClassSetCommand::InitialPass(std::vector<std::string> const& argsIn)
{
  if(argsIn.size() < 2)
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }
  std::vector<std::string> args = argsIn;
  // First, we want to expand all CMAKE variables in all arguments.
  for(std::vector<std::string>::iterator a = args.begin();
      a != args.end(); ++a)
    {
    m_Makefile->ExpandVariablesInString(*a);
    }
  
  // The first argument is the name of the set.
  std::vector<std::string>::const_iterator arg = args.begin();
  m_ClassSetName = *arg++;
  
  // Create the new class set.
  cmCableClassSet* classSet = new cmCableClassSet(m_ClassSetName.c_str());
  
  // Add all the regular entries.
  for(; (arg != args.end()) && (*arg != "SOURCES_BEGIN"); ++arg)
    {
    classSet->ParseAndAddElement(arg->c_str(), m_Makefile);
    }
  
  // Add any sources that are associated with all the members.
  if(arg != args.end())
    {
    for(++arg; arg != args.end(); ++arg)
      {
      classSet->AddSource(arg->c_str());
      }
    }
  
  // Store the class set in the makefile.
  m_Makefile->RegisterData(classSet);
  
  return true;
}

