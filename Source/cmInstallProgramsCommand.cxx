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
#include "cmInstallProgramsCommand.h"
#include "cmCacheManager.h"

// cmExecutableCommand
bool cmInstallProgramsCommand::InitialPass(std::vector<std::string> const& args)
{
  if(args.size() < 2)
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }

  // Create an INSTALL_PROGRAMS target specifically for this path.
  m_TargetName = "INSTALL_PROGRAMS_"+args[0];
  cmTarget target;
  target.SetInAll(false);
  target.SetType(cmTarget::INSTALL_PROGRAMS);
  target.SetInstallPath(args[0].c_str());
  m_Makefile->GetTargets().insert(cmTargets::value_type(m_TargetName, target));

  std::vector<std::string>::const_iterator s = args.begin();
  for (++s;s != args.end(); ++s)
    {
    m_FinalArgs.push_back(*s);
    }  
  
  return true;
}

void cmInstallProgramsCommand::FinalPass() 
{
  std::vector<std::string>& targetSourceLists =
    m_Makefile->GetTargets()[m_TargetName].GetSourceLists();
  
  // two different options
  if (m_FinalArgs.size() > 1)
    {
    // for each argument, get the programs 
    for (std::vector<std::string>::iterator s = m_FinalArgs.begin();
         s != m_FinalArgs.end(); ++s)
      {
      // replace any variables
      std::string temps = *s;
      m_Makefile->ExpandVariablesInString(temps);
      // add to the result
      targetSourceLists.push_back(temps);
      }
    }
  else     // reg exp list
    {
    std::vector<std::string> programs;
    cmSystemTools::Glob(m_Makefile->GetCurrentDirectory(),
                        m_FinalArgs[0].c_str(), programs);
    
    std::vector<std::string>::iterator s = programs.begin();
    // for each argument, get the programs 
    for (;s != programs.end(); ++s)
      {
      targetSourceLists.push_back(*s);
      }
    }
}

      
