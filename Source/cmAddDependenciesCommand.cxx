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
#include "cmAddDependenciesCommand.h"
#include "cmCacheManager.h"

// cmDependenciesCommand
bool cmAddDependenciesCommand::InitialPass(std::vector<std::string> const& argsIn)
{
  if(argsIn.size() < 2 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }
  std::vector<std::string> args;
  cmSystemTools::ExpandListArguments(argsIn, args);

  std::string target_name = args[0];

  cmTargets &tgts = m_Makefile->GetTargets();
  if (tgts.find(target_name) != tgts.end())
    {
    std::vector<std::string>::const_iterator s = args.begin();
    ++s;
    std::string depend_target;
    for (; s != args.end(); ++s)
      {
      tgts[target_name].AddUtility(s->c_str());
      }
    }
  else
    {
    std::string error = "Adding dependency to non-existent target: ";
    error += target_name;
    this->SetError(error.c_str());
    return false;
    }
  

  return true;
}

