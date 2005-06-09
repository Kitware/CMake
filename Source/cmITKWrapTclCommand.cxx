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
bool cmITKWrapTclCommand::InitialPass(std::vector<std::string> const& args)
{
  // deprecated
  const char* versionValue
    = m_Makefile->GetDefinition("CMAKE_BACKWARDS_COMPATIBILITY");
  if (atof(versionValue) > 2.2)
    {
    this->SetError("The ITK_WRAP_TCL command was deprecated in CMake version 2.2 and will be removed in later versions of CMake. You should modify your CMakeLists.txt files to use the MACRO command or use CMake 2.2 or earlier for this project.\n");
    return false;
    }
  if (atof(versionValue) > 2.0)
    {
    cmSystemTools::Message("The ITK_WRAP_TCL command was deprecated in CMake version 2.2 and will be removed in later versions. You should modify your CMakeLists.txt files to use a MACRO or use CMake 2.2 or earlier.\n","Warning");
    }

  if(args.size() < 2 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }
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

  cmCustomCommandLine commandLine;
  commandLine.push_back(command);
  commandLine.push_back(inFile);
  commandLine.push_back("-tcl");  
  std::string tmp = tclFile+".cxx";
  commandLine.push_back(tmp);
#if !defined(_WIN32) || defined(__CYGWIN__)
  tmp = "${CMAKE_CXX_COMPILER}";
  m_Makefile->ExpandVariablesInString(tmp);
  if(tmp.length() > 0)
    {
    commandLine.push_back("--gccxml-compiler");
    commandLine.push_back(tmp);
    tmp = "${CMAKE_CXX_FLAGS}";
    m_Makefile->ExpandVariablesInString(tmp);
    commandLine.push_back("--gccxml-cxxflags");
    commandLine.push_back(tmp);
    }
#else
  const char* genName = m_Makefile->GetDefinition("CMAKE_GENERATOR");
  if (genName)
    {
    std::string gen = genName;
    if(gen == "Visual Studio 6")
      {
      commandLine.push_back("--gccxml-compiler");
      commandLine.push_back("msvc6");
      tmp = "${CMAKE_CXX_FLAGS}";
      m_Makefile->ExpandVariablesInString(tmp);
      commandLine.push_back("--gccxml-cxxflags");
      commandLine.push_back(tmp);
      }
    else if(gen == "Visual Studio 7")
      {
      commandLine.push_back("--gccxml-compiler");
      commandLine.push_back("msvc7");
      tmp = "${CMAKE_CXX_FLAGS}";
      m_Makefile->ExpandVariablesInString(tmp);
      commandLine.push_back("--gccxml-cxxflags");
      commandLine.push_back(tmp);
      }
    else if(gen == "NMake Makefiles")
      {
      tmp = "${CMAKE_CXX_COMPILER}";
      m_Makefile->ExpandVariablesInString(tmp);
      commandLine.push_back("--gccxml-compiler");
      commandLine.push_back(tmp);
      tmp = "${CMAKE_CXX_FLAGS}";
      m_Makefile->ExpandVariablesInString(tmp);
      commandLine.push_back("--gccxml-cxxflags");
      commandLine.push_back(tmp);      
      }
    }
#endif
  const char* gccxml = m_Makefile->GetDefinition("ITK_GCCXML_EXECUTABLE");
  if(gccxml)
    {
    commandLine.push_back("--gccxml");
    commandLine.push_back(gccxml);
    }
  tmp = "-I";
  tmp += m_Makefile->GetStartDirectory();
  commandLine.push_back(tmp);
  const std::vector<std::string>& includes = 
    m_Makefile->GetIncludeDirectories();
  for(std::vector<std::string>::const_iterator i = includes.begin();
      i != includes.end(); ++i)
    {
    tmp = "-I";
    tmp += i->c_str();
    m_Makefile->ExpandVariablesInString(tmp);
    commandLine.push_back(tmp);
    }
  
  // Get the dependencies.
  const cmDependInformation* info =
    m_MakeDepend->FindDependencies(inFile.c_str());
  if (info)
    {
    std::set<cmDependInformation const*> visited;
    this->AddDependencies(info, depends, visited);
    }
  
  std::string output;
  output = outDir+"/"+tclFile+".cxx";
  
  // Add the source to the makefile.
  cmSourceFile file;
  file.SetName(tclFile.c_str(), outDir.c_str(), "cxx", false);
  // Set dependency hints.
  file.GetDepends().push_back(inFile.c_str());
  file.GetDepends().push_back("CableTclFacility/ctCalls.h");
  m_Makefile->AddSource(file);

  const char* no_comment = 0;
  cmCustomCommandLines commandLines;
  commandLines.push_back(commandLine);
  m_Makefile->AddCustomCommandToOutput(output.c_str(), depends,
                                       inFile.c_str(), commandLines,
                                       no_comment);
  
  // Add the generated source to the package's source list.
  m_Target->GetSourceLists().push_back(output);
  
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
                                 "CABLE-NOTFOUND",
                                 "Path to CABLE executable.",
                                 cmCacheManager::FILEPATH);
  return "CABLE-NOTFOUND";
}
