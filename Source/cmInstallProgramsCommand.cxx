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
#include "cmInstallProgramsCommand.h"

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
  cmTarget& target = m_Makefile->GetTargets()[m_TargetName];
  target.SetInAll(false);
  target.SetType(cmTarget::INSTALL_PROGRAMS, m_TargetName.c_str());
  target.SetInstallPath(args[0].c_str());

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
      // add to the result
      targetSourceLists.push_back(this->FindInstallSource(s->c_str()));
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
      targetSourceLists.push_back(this->FindInstallSource(s->c_str()));
      }
    }
}

/**
 * Find a file in the build or source tree for installation given a
 * relative path from the CMakeLists.txt file.  This will favor files
 * present in the build tree.  If a full path is given, it is just
 * returned.
 */
std::string cmInstallProgramsCommand::FindInstallSource(const char* name) const
{
  if(cmSystemTools::FileIsFullPath(name))
    {
    // This is a full path.
    return name;
    }
  
  // This is a relative path.
  std::string tb = m_Makefile->GetCurrentOutputDirectory();
  tb += "/";
  tb += name;
  std::string ts = m_Makefile->GetCurrentDirectory();
  ts += "/";
  ts += name;
  
  if(cmSystemTools::FileExists(tb.c_str()))
    {
    // The file exists in the binary tree.  Use it.
    return tb;
    }
  else if(cmSystemTools::FileExists(ts.c_str()))
    {
    // The file exists in the source tree.  Use it.
    return ts;
    }
  else
    {
    // The file doesn't exist.  Assume it will be present in the
    // binary tree when the install occurs.
    return tb;
    }
}
