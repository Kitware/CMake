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
#include "cmITKWrapTclCommand.h"
#include "cmMakeDepend.h"

cmITKWrapTclCommand::cmITKWrapTclCommand():
  m_MakeDepend(new cmMakeDepend)
{
}

cmITKWrapTclCommand::~cmITKWrapTclCommand()
{
  delete m_MakeDepend;
}

// cmITKWrapTclCommand
bool cmITKWrapTclCommand::InitialPass(std::vector<std::string> const& argsIn)
{
  if(argsIn.size() < 2 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }
  std::vector<std::string> args;
  cmSystemTools::ExpandListArguments(argsIn, args);
  // keep the target name
  m_TargetName = args[0];
  
  // Prepare the dependency generator.
  m_MakeDepend->SetMakefile(m_Makefile);
  
  for(std::vector<std::string>::const_iterator i = (args.begin() + 1);
      i != args.end(); ++i)
    {
    if(!this->CreateCableRule((*i).c_str())) { return false; }
    }
  
  // Add the source list to the target.
  m_Makefile->GetTargets()[m_TargetName.c_str()].GetSourceLists().push_back(m_TargetName);

  return true;
}

bool cmITKWrapTclCommand::CreateCableRule(const char* configFile)
{
  std::string tclFile =
    cmSystemTools::GetFilenameNameWithoutExtension(configFile);
  tclFile += "_tcl";
  
  std::string inFile = m_Makefile->GetCurrentDirectory();
  inFile += "/";
  inFile += configFile;
  
  std::string outDir = m_Makefile->GetCurrentOutputDirectory();
  
  // Generate the rule to run cable to generate wrappers.
  std::string command = this->GetCableFromCache();
  std::vector<std::string> depends;
  
  // Special case for CMake's wrapping test.  Don't add dependency if
  // it is a dummy executable.
  if(command != "echo")
    {
    depends.push_back(command);
    }
  
  std::vector<std::string> commandArgs;
  commandArgs.push_back(inFile);
  commandArgs.push_back("-tcl");  
  std::string tmp = tclFile+".cxx";
  commandArgs.push_back(tmp);
#if !defined(_WIN32) || defined(__CYGWIN__)
  tmp = "${CMAKE_CXX_COMPILER}";
  m_Makefile->ExpandVariablesInString(tmp);
  if(tmp.length() > 0)
    {
    commandArgs.push_back("--gccxml-compiler");
    commandArgs.push_back(tmp);
    }
#endif
  tmp = "-I";
  tmp += m_Makefile->GetStartDirectory();
  commandArgs.push_back(tmp);
  const std::vector<std::string>& includes = 
    m_Makefile->GetIncludeDirectories();
  for(std::vector<std::string>::const_iterator i = includes.begin();
      i != includes.end(); ++i)
    {
    tmp = "-I";
    tmp += i->c_str();
    m_Makefile->ExpandVariablesInString(tmp);
    commandArgs.push_back(tmp);
    }
  
  // Get the dependencies.
  const cmDependInformation* info =
    m_MakeDepend->FindDependencies(inFile.c_str());
  if (info)
    {
    for(cmDependInformation::DependencySet::const_iterator d = 
          info->m_DependencySet.begin();
        d != info->m_DependencySet.end(); ++d)
      {
      // Make sure the full path is given.  If not, the dependency was
      // not found.
      if((*d)->m_FullPath != "")
        {
        depends.push_back((*d)->m_FullPath);
        }
      }
    }
  
  std::vector<std::string> outputs;
  outputs.push_back(outDir+"/"+tclFile+".cxx");
  
  m_Makefile->AddCustomCommand(inFile.c_str(),
                               command.c_str(),
                               commandArgs, depends,
                               outputs, m_TargetName.c_str());
  
  // Add the generated source to the package's source list.
  cmSourceFile file;
  file.SetName(tclFile.c_str(), outDir.c_str(), "cxx", false);
  // Set dependency hints.
  file.GetDepends().push_back(inFile.c_str());
  file.GetDepends().push_back("CableTclFacility/ctCalls.h");
  m_Makefile->AddSource(file, m_TargetName.c_str());
  
  return true;
}

/**
 * Get the "CABLE" cache entry value.  If there is no cache entry for CABLE,
 * one will be created and initialized to NOTFOUND.
 */
std::string cmITKWrapTclCommand::GetCableFromCache() const
{
  const char* cable =
    m_Makefile->GetDefinition("CABLE");
  if(cable)
    { return cable; }

  m_Makefile->AddCacheDefinition("CABLE",
                                 "NOTFOUND",
                                 "Path to CABLE executable.",
                                 cmCacheManager::FILEPATH);
  return "NOTFOUND";
}
