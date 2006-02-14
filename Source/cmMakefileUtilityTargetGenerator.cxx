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
#include "cmMakefileUtilityTargetGenerator.h"

#include "cmGeneratedFileStream.h"
#include "cmGlobalGenerator.h"
#include "cmLocalUnixMakefileGenerator3.h"
#include "cmMakefile.h"
#include "cmSourceFile.h"
#include "cmTarget.h"


//----------------------------------------------------------------------------
void cmMakefileUtilityTargetGenerator::WriteRuleFiles()
{
  this->CreateRuleFile();

  *this->BuildFileStream
    << "# Utility rule file for " << this->Target->GetName() << ".\n\n";

  // write the custom commands for this target
  this->WriteCustomCommandsForTarget();

  // Collect the commands and dependencies.
  std::vector<std::string> commands;
  std::vector<std::string> depends;
  const char* sym = this->Makefile->GetDefinition("CMAKE_MAKE_SYMBOLIC_RULE");
  if(sym)
    {
    depends.push_back(sym);
    }

  // Utility targets store their rules in pre- and post-build commands.
  this->LocalGenerator->AppendCustomDepends
    (depends, this->Target->GetPreBuildCommands());
  this->LocalGenerator->AppendCustomDepends
    (depends, this->Target->GetPostBuildCommands());
  this->LocalGenerator->AppendCustomCommands
    (commands, this->Target->GetPreBuildCommands());
  this->LocalGenerator->AppendCustomCommands
    (commands, this->Target->GetPostBuildCommands());

  // Add dependencies on targets that must be built first.
  this->AppendTargetDepends(depends);
  
  // Add a dependency on the rule file itself.
  std::string relPath = this->LocalGenerator->GetHomeRelativeOutputPath();
  std::string objTarget = relPath;
  objTarget += this->BuildFileName;
  this->LocalGenerator->AppendRuleDepend(depends, objTarget.c_str());

  // Write the rule.
  this->LocalGenerator->WriteMakeRule(*this->BuildFileStream, 0,
                      this->Target->GetName(), depends, commands);

  // Write convenience targets.
  std::string dir = this->Makefile->GetStartOutputDirectory();
  dir += "/";
  dir += this->LocalGenerator->GetTargetDirectory(*this->Target);
  std::string buildTargetRuleName = dir;
  buildTargetRuleName += "/build";
  buildTargetRuleName = 
    this->LocalGenerator->Convert(buildTargetRuleName.c_str(),
                                  cmLocalGenerator::HOME_OUTPUT,
                                  cmLocalGenerator::MAKEFILE);
  this->LocalGenerator->WriteConvenienceRule(*this->BuildFileStream, 
                                             this->Target->GetName(),
                                             buildTargetRuleName.c_str());

  // Write clean target
  this->WriteTargetCleanRules();

  // close the streams
  this->CloseFileStreams();
}

