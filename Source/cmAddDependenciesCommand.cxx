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
bool cmAddDependenciesCommand::InitialPass(std::vector<std::string> const& args)
{
  if(args.size() < 2 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }
  
  std::string target_name = args[0];
  m_Makefile->ExpandVariablesInString(target_name);

  cmTargets &tgts = m_Makefile->GetTargets();
  if (tgts.find(target_name) != tgts.end())
    {
    std::vector<std::string>::const_iterator s = args.begin();
    ++s;
    std::string depend_target;
    for (; s != args.end(); ++s)
      {
      depend_target = *s;
      m_Makefile->ExpandVariablesInString(depend_target);
      tgts[target_name].AddUtility(depend_target.c_str());
      }
    }
  else
    {
    std::cerr << "existing targets are:";
    
    for(cmTargets::iterator i = tgts.begin();
        i != tgts.end(); ++i)
      {
      std::cerr << i->first << std::endl;
      }
    
    std::string error = "Adding dependency to non-existent target: ";
    error += target_name;
    this->SetError(error.c_str());
    return false;
    }
  

  return true;
}

