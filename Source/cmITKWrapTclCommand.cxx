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



void 
cmITKWrapTclCommand::AddDependencies(cmDependInformation const *info,
                                     std::vector<std::string>& depends,
                                     std::set<cmDependInformation const*>& visited)
{
  if(!info)
    {
    return;
    }
  // add info to the visited set
  visited.insert(info);
    
  // add this dependency and the recurse
  // now recurse with info's dependencies
  for(cmDependInformation::DependencySet::const_iterator d = 
        info->m_DependencySet.begin();
      d != info->m_DependencySet.end(); ++d)
    {
    if (visited.find(*d) == visited.end())
      { 
      if((*d)->m_FullPath != "")
        {
        depends.push_back((*d)->m_FullPath);
        }
      this->AddDependencies(*d,depends,visited);
      }
    }
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
  m_Target = &m_Makefile->GetTargets()[m_TargetName.c_str()];
  
  // Prepare the dependency generator.
  m_MakeDepend->SetMakefile(m_Makefile);
  
  for(std::vector<std::string>::const_iterator i = (args.begin() + 1);
      i != args.end(); ++i)
    {
    if(!this->CreateCableRule((*i).c_str())) { return false; }
    }
  
  return true;
}

bool cmITKWrapTclCommand::CreateCableRule(const char* configFile)
{
  std::string tclFile =
    cmSystemTools::GetFilenameWithoutExtension(configFile);
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
    tmp = "${CMAKE_CXX_FLAGS}";
    m_Makefile->ExpandVariablesInString(tmp);
    commandArgs.push_back("--gccxml-cxxflags");
    commandArgs.push_back(tmp);
    }
#else
  const char* genName = m_Makefile->GetDefinition("CMAKE_GENERATOR");
  if (genName)
    {
    std::string gen = genName;
    if(gen == "Visual Studio 6")
      {
      commandArgs.push_back("--gccxml-compiler");
      commandArgs.push_back("msvc6");
      tmp = "${CMAKE_CXX_FLAGS}";
      m_Makefile->ExpandVariablesInString(tmp);
      commandArgs.push_back("--gccxml-cxxflags");
      commandArgs.push_back(tmp);
      }
    else if(gen == "Visual Studio 7")
      {
      commandArgs.push_back("--gccxml-compiler");
      commandArgs.push_back("msvc7");
      tmp = "${CMAKE_CXX_FLAGS}";
      m_Makefile->ExpandVariablesInString(tmp);
      commandArgs.push_back("--gccxml-cxxflags");
      commandArgs.push_back(tmp);
      }
    else if(gen == "NMake Makefiles")
      {
      tmp = "${CMAKE_CXX_COMPILER}";
      m_Makefile->ExpandVariablesInString(tmp);
      commandArgs.push_back("--gccxml-compiler");
      commandArgs.push_back(tmp);
      tmp = "${CMAKE_CXX_FLAGS}";
      m_Makefile->ExpandVariablesInString(tmp);
      commandArgs.push_back("--gccxml-cxxflags");
      commandArgs.push_back(tmp);      
      }
    }
#endif
  const char* gccxml = m_Makefile->GetDefinition("ITK_GCCXML_EXECUTABLE");
  if(gccxml)
    {
    commandArgs.push_back("--gccxml");
    commandArgs.push_back(gccxml);
    }
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
    std::set<cmDependInformation const*> visited;
    this->AddDependencies(info, depends, visited);
    }
  
  std::vector<std::string> outputs;
  outputs.push_back(outDir+"/"+tclFile+".cxx");
  
  m_Makefile->AddCustomCommand(inFile.c_str(),
                               command.c_str(),
                               commandArgs, depends,
                               outputs, m_TargetName.c_str());
  
  // Add the source to the makefile.
  cmSourceFile file;
  file.SetName(tclFile.c_str(), outDir.c_str(), "cxx", false);
  // Set dependency hints.
  file.GetDepends().push_back(inFile.c_str());
  file.GetDepends().push_back("CableTclFacility/ctCalls.h");
  m_Makefile->AddSource(file);
  
  // Add the generated source to the package's source list.
  std::string srcname = file.GetSourceName() + ".cxx";
  m_Target->GetSourceLists().push_back(srcname);
  
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
