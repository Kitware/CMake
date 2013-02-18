/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmGlobalBorlandMakefileGenerator.h"
#include "cmLocalUnixMakefileGenerator3.h"
#include "cmMakefile.h"
#include "cmake.h"

cmGlobalBorlandMakefileGenerator::cmGlobalBorlandMakefileGenerator()
{
  this->EmptyRuleHackDepends = "NUL";
  this->FindMakeProgramFile = "CMakeBorlandFindMake.cmake";
  this->ForceUnixPaths = false;
  this->ToolSupportsColor = true;
  this->UseLinkScript = false;
}


void cmGlobalBorlandMakefileGenerator
::EnableLanguage(std::vector<std::string>const& l,
                 cmMakefile *mf,
                 bool optional)
{
  std::string outdir = this->CMakeInstance->GetStartOutputDirectory();
  mf->AddDefinition("BORLAND", "1");
  mf->AddDefinition("CMAKE_GENERATOR_CC", "bcc32");
  mf->AddDefinition("CMAKE_GENERATOR_CXX", "bcc32");
  this->cmGlobalUnixMakefileGenerator3::EnableLanguage(l, mf, optional);
}

///! Create a local generator appropriate to this Global Generator
cmLocalGenerator *cmGlobalBorlandMakefileGenerator::CreateLocalGenerator()
{
  cmLocalUnixMakefileGenerator3* lg = new cmLocalUnixMakefileGenerator3;
  lg->SetIncludeDirective("!include");
  lg->SetWindowsShell(true);
  lg->SetDefineWindowsNULL(true);
  lg->SetMakefileVariableSize(32);
  lg->SetPassMakeflags(true);
  lg->SetGlobalGenerator(this);
  lg->SetUnixCD(false);
  lg->SetMakeCommandEscapeTargetTwice(true);
  lg->SetBorlandMakeCurlyHack(true);
  return lg;
}


//----------------------------------------------------------------------------
void cmGlobalBorlandMakefileGenerator
::GetDocumentation(cmDocumentationEntry& entry)
{
  entry.Name = cmGlobalBorlandMakefileGenerator::GetActualName();
  entry.Brief = "Generates Borland makefiles.";
  entry.Full = "";
}
