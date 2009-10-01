/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmInstallProgramsCommand.h"
#include "cmInstallFilesGenerator.h"
// cmExecutableCommand
bool cmInstallProgramsCommand
::InitialPass(std::vector<std::string> const& args, cmExecutionStatus &)
{
  if(args.size() < 2)
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }

  // Enable the install target.
  this->Makefile->GetLocalGenerator()
    ->GetGlobalGenerator()->EnableInstallTarget();

  this->Destination = args[0];

  std::vector<std::string>::const_iterator s = args.begin();
  for (++s;s != args.end(); ++s)
    {
    this->FinalArgs.push_back(*s);
    }  
  
  this->Makefile->GetLocalGenerator()->GetGlobalGenerator()
                       ->AddInstallComponent("Unspecified");

  return true;
}

void cmInstallProgramsCommand::FinalPass() 
{
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
      this->Files.push_back(this->FindInstallSource(s->c_str()));
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
      this->Files.push_back(this->FindInstallSource(s->c_str()));
      }
    }

  // Construct the destination.  This command always installs under
  // the prefix.  We skip the leading slash given by the user.
  std::string destination = this->Destination.substr(1);
  cmSystemTools::ConvertToUnixSlashes(destination);
  if(destination.empty())
    {
    destination = ".";
    }

  // Use a file install generator.
  const char* no_permissions = "";
  const char* no_rename = "";
  const char* no_component = "Unspecified";
  std::vector<std::string> no_configurations;
  this->Makefile->AddInstallGenerator(
    new cmInstallFilesGenerator(this->Files,
                                destination.c_str(), true,
                                no_permissions, no_configurations,
                                no_component, no_rename));
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
