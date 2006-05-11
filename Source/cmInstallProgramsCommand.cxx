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
bool cmInstallProgramsCommand
::InitialPass(std::vector<std::string> const& args)
{
  if(args.size() < 2)
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }

  // Create an INSTALL_PROGRAMS target specifically for this path.
  this->TargetName = "INSTALL_PROGRAMS_"+args[0];
  cmTarget& target = this->Makefile->GetTargets()[this->TargetName];
  target.SetInAll(false);
  target.SetType(cmTarget::INSTALL_PROGRAMS, this->TargetName.c_str());
  target.SetInstallPath(args[0].c_str());

  std::vector<std::string>::const_iterator s = args.begin();
  for (++s;s != args.end(); ++s)
    {
    this->FinalArgs.push_back(*s);
    }  
  
  return true;
}

void cmInstallProgramsCommand::FinalPass() 
{
  std::vector<std::string>& targetSourceLists =
    this->Makefile->GetTargets()[this->TargetName].GetSourceLists();

  bool files_mode = false;
  if(!this->FinalArgs.empty() && this->FinalArgs[0] == "FILES")
    {
    files_mode = true;
    }
  
  // two different options
  if (this->FinalArgs.size() > 1 || files_mode)
    {
    // for each argument, get the programs 
    std::vector<std::string>::iterator s = this->FinalArgs.begin();
    if(files_mode)
      {
      // Skip the FILES argument in files mode.
      ++s;
      }
    for(;s != this->FinalArgs.end(); ++s)
      {
      // add to the result
      targetSourceLists.push_back(this->FindInstallSource(s->c_str()));
      }
    }
  else     // reg exp list
    {
    std::vector<std::string> programs;
    cmSystemTools::Glob(this->Makefile->GetCurrentDirectory(),
                        this->FinalArgs[0].c_str(), programs);
    
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
std::string cmInstallProgramsCommand
::FindInstallSource(const char* name) const
{
  if(cmSystemTools::FileIsFullPath(name))
    {
    // This is a full path.
    return name;
    }
  
  // This is a relative path.
  std::string tb = this->Makefile->GetCurrentOutputDirectory();
  tb += "/";
  tb += name;
  std::string ts = this->Makefile->GetCurrentDirectory();
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
