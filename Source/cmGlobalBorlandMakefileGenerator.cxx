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
}


void cmGlobalBorlandMakefileGenerator
::EnableLanguage(std::vector<std::string>const& l, cmMakefile *mf)
{
  std::string outdir = this->CMakeInstance->GetStartOutputDirectory();
  mf->AddDefinition("BORLAND", "1");
  mf->AddDefinition("CMAKE_GENERATOR_CC", "bcc32");
  mf->AddDefinition("CMAKE_GENERATOR_CXX", "bcc32"); 
  this->cmGlobalUnixMakefileGenerator3::EnableLanguage(l, mf);
}

///! Create a local generator appropriate to this Global Generator
cmLocalGenerator *cmGlobalBorlandMakefileGenerator::CreateLocalGenerator()
{
  cmLocalUnixMakefileGenerator3* lg = new cmLocalUnixMakefileGenerator3;
  lg->SetEchoNeedsQuote(false);
  lg->SetIncludeDirective("!include");
  lg->SetWindowsShell(true);
  lg->SetDefineWindowsNULL(true);
  lg->SetMakefileVariableSize(32);
  lg->SetPassMakeflags(true);
  lg->SetGlobalGenerator(this);
  lg->SetUnixCD(false);
  return lg;
}


//----------------------------------------------------------------------------
void cmGlobalBorlandMakefileGenerator
::GetDocumentation(cmDocumentationEntry& entry) const
{
  entry.name = this->GetName();
  entry.brief = "Generates Borland makefiles.";
  entry.full = "";
}
