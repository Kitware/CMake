/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmGlobalMSYSMakefileGenerator.h"
#include "cmLocalUnixMakefileGenerator3.h"
#include "cmMakefile.h"
#include "cmake.h"

cmGlobalMSYSMakefileGenerator::cmGlobalMSYSMakefileGenerator()
{
  this->FindMakeProgramFile = "CMakeMSYSFindMake.cmake";
  this->ForceUnixPaths = true;
  this->ToolSupportsColor = true;
  this->UseLinkScript = false;
}

std::string 
cmGlobalMSYSMakefileGenerator::FindMinGW(std::string const& makeloc)
{
  std::string fstab = makeloc;
  fstab += "/../etc/fstab";
  std::ifstream fin(fstab.c_str());
  std::string path;
  std::string mount;
  std::string mingwBin;
  while(fin)
    {
    fin >> path;
    fin >> mount;
    if(mount == "/mingw")
      {
      mingwBin = path;
      mingwBin += "/bin";
      }
    }
  return mingwBin;
}

void cmGlobalMSYSMakefileGenerator
::EnableLanguage(std::vector<std::string>const& l, 
                 cmMakefile *mf, 
                 bool optional)
{
  this->FindMakeProgram(mf);
  std::string makeProgram = mf->GetRequiredDefinition("CMAKE_MAKE_PROGRAM");
  std::vector<std::string> locations;
  std::string makeloc = cmSystemTools::GetProgramPath(makeProgram.c_str());
  locations.push_back(this->FindMinGW(makeloc));
  locations.push_back(makeloc);
  locations.push_back("/mingw/bin");
  locations.push_back("c:/mingw/bin");
  std::string tgcc = cmSystemTools::FindProgram("gcc", locations);
  std::string gcc = "gcc.exe";
  if(tgcc.size())
    {
    gcc = tgcc;
    }
  std::string tgxx = cmSystemTools::FindProgram("g++", locations);
  std::string gxx = "g++.exe";
  if(tgxx.size())
    {
    gxx = tgxx;
    }
  mf->AddDefinition("MSYS", "1");
  mf->AddDefinition("CMAKE_GENERATOR_CC", gcc.c_str());
  mf->AddDefinition("CMAKE_GENERATOR_CXX", gxx.c_str());
  this->cmGlobalUnixMakefileGenerator3::EnableLanguage(l, mf, optional);

  if(!mf->IsSet("CMAKE_AR") &&
      !this->CMakeInstance->GetIsInTryCompile() &&
      !(1==l.size() && l[0]=="NONE"))
    {
    cmSystemTools::Error
      ("CMAKE_AR was not found, please set to archive program. ",
       mf->GetDefinition("CMAKE_AR"));
    }
}

///! Create a local generator appropriate to this Global Generator
cmLocalGenerator *cmGlobalMSYSMakefileGenerator::CreateLocalGenerator()
{
  cmLocalUnixMakefileGenerator3* lg = new cmLocalUnixMakefileGenerator3;
  lg->SetWindowsShell(false);
  lg->SetMSYSShell(true);
  lg->SetGlobalGenerator(this);
  lg->SetIgnoreLibPrefix(true);
  lg->SetPassMakeflags(false);
  lg->SetUnixCD(true);
  return lg;
}

//----------------------------------------------------------------------------
void cmGlobalMSYSMakefileGenerator
::GetDocumentation(cmDocumentationEntry& entry) const
{
  entry.Name = this->GetName();
  entry.Brief = "Generates MSYS makefiles.";
  entry.Full = "The makefiles use /bin/sh as the shell.  "
    "They require msys to be installed on the machine.";
}
