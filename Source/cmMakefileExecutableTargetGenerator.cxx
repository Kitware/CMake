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
#include "cmMakefileExecutableTargetGenerator.h"

#include "cmGeneratedFileStream.h"
#include "cmGlobalGenerator.h"
#include "cmLocalUnixMakefileGenerator3.h"
#include "cmMakefile.h"
#include "cmSourceFile.h"
#include "cmTarget.h"

//----------------------------------------------------------------------------
void cmMakefileExecutableTargetGenerator::WriteRuleFiles()
{
  // create the build.make file and directory, put in the common blocks
  this->CreateRuleFile();
  
  // Add in any rules for custom commands
  this->WriteCustomCommandsForTarget();

  // write in rules for object files
  this->WriteCommonCodeRules();

  // Write the dependency generation rule.
  this->WriteTargetDependRules();

  // write the link rules
  this->WriteExecutableRule(false);
  if(this->Target->NeedRelinkBeforeInstall())
    {
    // Write rules to link an installable version of the target.
    this->WriteExecutableRule(true);
    }

  // Write the requires target.
  this->WriteTargetRequiresRules();

  // Write clean target
  this->WriteTargetCleanRules();

  // close the streams
  this->CloseFileStreams();
}



//----------------------------------------------------------------------------
void cmMakefileExecutableTargetGenerator::WriteExecutableRule(bool relink)
{
  std::vector<std::string> commands;

  std::string relPath = this->LocalGenerator->GetHomeRelativeOutputPath();
  std::string objTarget;

  // Build list of dependencies.
  std::vector<std::string> depends;
  for(std::vector<std::string>::const_iterator obj = this->Objects.begin();
      obj != this->Objects.end(); ++obj)
    {
    objTarget = relPath;
    objTarget += *obj;
    depends.push_back(objTarget);
    }

  // Add dependencies on targets that must be built first.
  this->AppendTargetDepends(depends);

  // Add a dependency on the rule file itself.
  this->LocalGenerator->AppendRuleDepend(depends,
                                         this->BuildFileNameFull.c_str());
  
  for(std::vector<std::string>::const_iterator obj = 
        this->ExternalObjects.begin();
      obj != this->ExternalObjects.end(); ++obj)
    {
    depends.push_back(*obj);
    }

  // from here up is the same for exe or lib

  // Get the name of the executable to generate.
  std::string targetName;
  std::string targetNameReal;
  this->Target->GetExecutableNames(targetName, targetNameReal,
                                   this->LocalGenerator->m_ConfigurationName.c_str());

  // Construct the full path version of the names.
  std::string outpath = this->LocalGenerator->m_ExecutableOutputPath;
  if(outpath.length() == 0)
    {
    outpath = this->Makefile->GetStartOutputDirectory();
    outpath += "/";
    }
#ifdef __APPLE__
  if(this->Target->GetPropertyAsBool("MACOSX_BUNDLE"))
    {
    // Make bundle directories
    outpath += targetName;
    outpath += ".app/Contents/MacOS/";
    std::string f1 = 
      this->Makefile->GetModulesFile("MacOSXBundleInfo.plist.in");
    if ( f1.size() == 0 )
      {
      cmSystemTools::Error("could not find Mac OSX bundle template file.");
      }
    std::string macdir = 
      this->Makefile->GetSafeDefinition("EXECUTABLE_OUTPUT_PATH");
    if ( macdir.size() == 0 )
      {
      macdir = this->Makefile->GetCurrentOutputDirectory();
      }
    if(macdir.size() && macdir[macdir.size()-1] != '/')
      {
      macdir += "/";
      }
    macdir += targetName;
    macdir += ".app/Contents/";

    // Configure the Info.plist file.  Note that it needs the executable name
    // to be set.
    std::string f2 = macdir + "Info.plist";
    macdir += "MacOS";
    cmSystemTools::MakeDirectory(macdir.c_str());
    this->Makefile->AddDefinition("MACOSX_BUNDLE_EXECUTABLE_NAME",
                                  targetName.c_str());
    this->Makefile->ConfigureFile(f1.c_str(), f2.c_str(), false, false, false);
    }
#endif
  if(relink)
    {
    outpath = this->Makefile->GetStartOutputDirectory();
    outpath += "/CMakeFiles/CMakeRelink.dir";
    cmSystemTools::MakeDirectory(outpath.c_str());
    outpath += "/";
    }
  std::string targetFullPath = outpath + targetName;
  std::string targetFullPathReal = outpath + targetNameReal;

  // Convert to the output path to use in constructing commands.
  std::string targetOutPath =
    this->Convert(targetFullPath.c_str(),
                                  cmLocalGenerator::START_OUTPUT,
                                  cmLocalGenerator::MAKEFILE);
  std::string targetOutPathReal =
    this->Convert(targetFullPathReal.c_str(),
                                  cmLocalGenerator::START_OUTPUT,
                                  cmLocalGenerator::MAKEFILE);

  // Get the language to use for linking this executable.
  const char* linkLanguage =
    this->Target->GetLinkerLanguage(this->GlobalGenerator);

  // Make sure we have a link language.
  if(!linkLanguage)
    {
    cmSystemTools::Error("Cannot determine link language for target \"",
                         this->Target->GetName(), "\".");
    return;
    }

  // Add the link message.
  std::string buildEcho = "Linking ";
  buildEcho += linkLanguage;
  buildEcho += " executable ";
  buildEcho += targetOutPath;
  this->LocalGenerator->AppendEcho(commands, buildEcho.c_str());

  // Build a list of compiler flags and linker flags.
  std::string flags;
  std::string linkFlags;

  // Add flags to deal with shared libraries.  Any library being
  // linked in might be shared, so always use shared flags for an
  // executable.
  this->LocalGenerator->AddSharedFlags(linkFlags, linkLanguage, true);

  // Add flags to create an executable.
  this->LocalGenerator->
    AddConfigVariableFlags(linkFlags, "CMAKE_EXE_LINKER_FLAGS",
                           this->LocalGenerator->m_ConfigurationName.c_str());


  if(this->Target->GetPropertyAsBool("WIN32_EXECUTABLE"))
    {
    this->LocalGenerator->AppendFlags(linkFlags,
                                      this->Makefile->GetDefinition("CMAKE_CREATE_WIN32_EXE"));
    }
  else
    {
    this->LocalGenerator->AppendFlags(linkFlags,
                                      this->Makefile->GetDefinition("CMAKE_CREATE_CONSOLE_EXE"));
    }

  // Add language-specific flags.
  this->LocalGenerator
    ->AddLanguageFlags(flags, linkLanguage,
                       this->LocalGenerator->m_ConfigurationName.c_str());

  // Add target-specific linker flags.
  this->LocalGenerator->AppendFlags(linkFlags, this->Target->GetProperty("LINK_FLAGS"));

  // Construct a list of files associated with this executable that
  // may need to be cleaned.
  std::vector<std::string> exeCleanFiles;
  {
  std::string cleanName;
  std::string cleanRealName;
  this->Target->GetExecutableCleanNames(cleanName, cleanRealName,
                                        this->LocalGenerator->m_ConfigurationName.c_str());
  std::string cleanFullName = outpath + cleanName;
  std::string cleanFullRealName = outpath + cleanRealName;
  exeCleanFiles.push_back(this->Convert(cleanFullName.c_str(),
                                        cmLocalGenerator::START_OUTPUT,
                                        cmLocalGenerator::UNCHANGED));
  if(cleanRealName != cleanName)
    {
    exeCleanFiles.push_back(this->Convert(cleanFullRealName.c_str(),
                                          cmLocalGenerator::START_OUTPUT,
                                          cmLocalGenerator::UNCHANGED));
    }
  } 

  // Add a command to remove any existing files for this executable.
  std::vector<std::string> commands1;
  this->LocalGenerator->AppendCleanCommand(commands1, exeCleanFiles,
                                           *this->Target, "target");
  this->LocalGenerator->CreateCDCommand(commands1,
                                        this->Makefile->GetStartOutputDirectory(),
                                        this->Makefile->GetHomeOutputDirectory()); 
  commands.insert(commands.end(), commands1.begin(), commands1.end());
  commands1.clear();

  // Add the pre-build and pre-link rules building but not when relinking.
  if(!relink)
    {
    this->LocalGenerator
      ->AppendCustomCommands(commands, this->Target->GetPreBuildCommands());
    this->LocalGenerator
      ->AppendCustomCommands(commands, this->Target->GetPreLinkCommands());
    }

  // Construct the main link rule.
  std::string linkRuleVar = "CMAKE_";
  linkRuleVar += linkLanguage;
  linkRuleVar += "_LINK_EXECUTABLE";
  std::string linkRule = 
    this->Makefile->GetRequiredDefinition(linkRuleVar.c_str());
  cmSystemTools::ExpandListArgument(linkRule, commands1);
  this->LocalGenerator->CreateCDCommand
    (commands1,
     this->Makefile->GetStartOutputDirectory(),
     this->Makefile->GetHomeOutputDirectory());
  commands.insert(commands.end(), commands1.begin(), commands1.end());

  // Add a rule to create necessary symlinks for the library.
  if(targetOutPath != targetOutPathReal)
    {
    std::string symlink = "$(CMAKE_COMMAND) -E cmake_symlink_executable ";
    symlink += targetOutPathReal;
    symlink += " ";
    symlink += targetOutPath;
    commands.push_back(symlink);
    }

  // Add the post-build rules when building but not when relinking.
  if(!relink)
    {
    this->LocalGenerator->
      AppendCustomCommands(commands, this->Target->GetPostBuildCommands());
    }

  // Collect up flags to link in needed libraries.
  cmOStringStream linklibs;
  this->LocalGenerator->OutputLinkLibraries(linklibs, *this->Target, relink);

  // Construct object file lists that may be needed to expand the
  // rule.
  std::string variableName;
  std::string variableNameExternal;
  this->WriteObjectsVariable(variableName, variableNameExternal);
  std::string buildObjs = "$(";
  buildObjs += variableName;
  buildObjs += ") $(";
  buildObjs += variableNameExternal;
  buildObjs += ")";
  std::string cleanObjs = "$(";
  cleanObjs += variableName;
  cleanObjs += ")";

  cmLocalGenerator::RuleVariables vars;
  vars.Language = linkLanguage;
  vars.Objects = buildObjs.c_str();
  vars.Target = targetOutPathReal.c_str();
  std::string linkString = linklibs.str();
  vars.LinkLibraries = linkString.c_str();
  vars.Flags = flags.c_str();
  vars.LinkFlags = linkFlags.c_str();
  // Expand placeholders in the commands.
  for(std::vector<std::string>::iterator i = commands.begin();
      i != commands.end(); ++i)
    {
    this->LocalGenerator->ExpandRuleVariables(*i, vars);
    }

  // Write the build rule.
  this->LocalGenerator->WriteMakeRule(*this->BuildFileStream, 
                                      0,
                                      targetFullPathReal.c_str(), 
                                      depends, commands, false);

  // The symlink name for the target should depend on the real target
  // so if the target version changes it rebuilds and recreates the
  // symlink.
  if(targetFullPath != targetFullPathReal)
    {
    depends.clear();
    commands.clear();
    depends.push_back(targetFullPathReal.c_str());
    this->LocalGenerator->WriteMakeRule(*this->BuildFileStream, 0,
                                        targetFullPath.c_str(), 
                                        depends, commands, false);
    }

  // Write convenience targets.
  std::string dir = this->Makefile->GetStartOutputDirectory();
  dir += "/";
  dir += this->LocalGenerator->GetTargetDirectory(*this->Target);
  std::string buildTargetRuleName = dir;
  buildTargetRuleName += relink?"/preinstall":"/build";
  buildTargetRuleName = 
    this->Convert(buildTargetRuleName.c_str(),
                  cmLocalGenerator::HOME_OUTPUT,
                  cmLocalGenerator::MAKEFILE);
  this->LocalGenerator->WriteConvenienceRule(*this->BuildFileStream, 
                                             targetFullPath.c_str(),
                                             buildTargetRuleName.c_str());

  // Clean all the possible executable names and symlinks and object files.
  this->CleanFiles.insert(this->CleanFiles.end(),
                          exeCleanFiles.begin(),
                          exeCleanFiles.end());
  this->CleanFiles.insert(this->CleanFiles.end(),
                          this->Objects.begin(),
                          this->Objects.end());
}

