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
#include "cmRemoveCommand.h"

// cmRemoveCommand
bool cmRemoveCommand::InitialPass(std::vector<std::string> const& args)
{
  if(args.size() < 1)
    {
    return true;
    }

  const char* variable = args[0].c_str(); // VAR is always first
  // get the old value
  const char* cacheValue
    = m_Makefile->GetDefinition(variable);

  // if there is no old value then return
  if (!cacheValue)
    {
    return true;
    }
  
  // expand the variable
  std::vector<std::string> varArgsExpanded;
  std::vector<std::string> temp;
  temp.push_back(std::string(cacheValue));
  cmSystemTools::ExpandListArguments(temp, varArgsExpanded);
  
  // expand the args
  // check for REMOVE(VAR v1 v2 ... vn) 
  std::vector<std::string> argsExpanded;
  std::vector<std::string> temp2;
  for(unsigned int j = 1; j < args.size(); ++j)
    {
    temp2.push_back(args[j]);
    }
  cmSystemTools::ExpandListArguments(temp2, argsExpanded);
  
  // now create the new value
  std::string value;
  for(unsigned int j = 1; j < varArgsExpanded.size(); ++j)
    {
    int found = 0;
    for(unsigned int k = 1; k < argsExpanded.size(); ++k)
      {
      if (varArgsExpanded[j] == argsExpanded[k])
        {
        found = 1;
        break;
        }
      }
    if (!found)
      {
      if (value.size())
        {
        value += ";";
        }
      value += varArgsExpanded[j];
      }
    }
  
  // add the definition
  m_Makefile->AddDefinition(variable, value.c_str());

  return true;
}

