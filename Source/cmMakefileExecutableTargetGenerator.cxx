/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmMakefileExecutableTargetGenerator.h"

#include "cmGeneratedFileStream.h"
#include "cmGlobalUnixMakefileGenerator3.h"
#include "cmLocalUnixMakefileGenerator3.h"
#include "cmMakefile.h"
#include "cmSourceFile.h"
#include "cmTarget.h"
#include "cmake.h"

//----------------------------------------------------------------------------
cmMakefileExecutableTargetGenerator
::cmMakefileExecutableTargetGenerator(cmTarget* target):
  cmMakefileTargetGenerator(target)
{
  this->CustomCommandDriver = OnDepends;
  this->Target->GetExecutableNames(
    this->TargetNameOut, this->TargetNameReal, this->TargetNameImport,
    this->TargetNamePDB, this->ConfigName);

  if(this->Target->IsAppBundleOnApple())
    {
    this->MacContentDirectory = this->Target->GetDirectory(this->ConfigName);
    this->MacContentDirectory += "/";
    this->MacContentDirectory += this->TargetNameOut;
    this->MacContentDirectory += ".app/Contents/";
    }
}

//----------------------------------------------------------------------------
void cmMakefileExecutableTargetGenerator::WriteRuleFiles()
{
  // create the build.make file and directory, put in the common blocks
  this->CreateRuleFile();

  // write rules used to help build object files
  this->WriteCommonCodeRules();

  // write the per-target per-language flags
  this->WriteTargetLanguageFlags();

  // write in rules for object files and custom commands
  this->WriteTargetBuildRules();

  // write the link rules
  this->WriteExecutableRule(false);
  if(this->Target->NeedRelinkBeforeInstall(this->ConfigName))
    {
    // Write rules to link an installable version of the target.
    this->WriteExecutableRule(true);
    }

  // Write the requires target.
  this->WriteTargetRequiresRules();

  // Write clean target
  this->WriteTargetCleanRules();

  // Write the dependency generation rule.  This must be done last so
  // that multiple output pair information is available.
  this->WriteTargetDependRules();

  // close the streams
  this->CloseFileStreams();
}



//----------------------------------------------------------------------------
void cmMakefileExecutableTargetGenerator::WriteExecutableRule(bool relink)
{
  std::vector<std::string> commands;

  // Build list of dependencies.
  std::vector<std::string> depends;
  this->AppendLinkDepends(depends);

  // Get the name of the executable to generate.
  std::string targetName;
  std::string targetNameReal;
  std::string targetNameImport;
  std::string targetNamePDB;
  this->Target->GetExecutableNames
    (targetName, targetNameReal, targetNameImport, targetNamePDB,
     this->ConfigName);

  // Construct the full path version of the names.
  std::string outpath = this->Target->GetDirectory(this->ConfigName);
  outpath += "/";
  if(this->Target->IsAppBundleOnApple())
    {
    this->CreateAppBundle(targetName, outpath);
    }
  std::string outpathImp;
  if(relink)
    {
    outpath = this->Makefile->GetStartOutputDirectory();
    outpath += cmake::GetCMakeFilesDirectory();
    outpath += "/CMakeRelink.dir";
    cmSystemTools::MakeDirectory(outpath.c_str());
    outpath += "/";
    if(!targetNameImport.empty())
      {
      outpathImp = outpath;
      }
    }
  else
    {
    cmSystemTools::MakeDirectory(outpath.c_str());
    if(!targetNameImport.empty())
      {
      outpathImp = this->Target->GetDirectory(this->ConfigName, true);
      cmSystemTools::MakeDirectory(outpathImp.c_str());
      outpathImp += "/";
      }
    }
  std::string targetFullPath = outpath + targetName;
  std::string targetFullPathReal = outpath + targetNameReal;
  std::string targetFullPathPDB = outpath + targetNamePDB;
  std::string targetFullPathImport = outpathImp + targetNameImport;
  std::string targetOutPathPDB = 
    this->Convert(targetFullPathPDB.c_str(),
                  cmLocalGenerator::NONE,
                  cmLocalGenerator::SHELL);
  // Convert to the output path to use in constructing commands.
  std::string targetOutPath =
    this->Convert(targetFullPath.c_str(),
                  cmLocalGenerator::START_OUTPUT,
                  cmLocalGenerator::SHELL);
  std::string targetOutPathReal =
    this->Convert(targetFullPathReal.c_str(),
                  cmLocalGenerator::START_OUTPUT,
                  cmLocalGenerator::SHELL);
  std::string targetOutPathImport =
    this->Convert(targetFullPathImport.c_str(),
                  cmLocalGenerator::START_OUTPUT,
                  cmLocalGenerator::SHELL);

  // Get the language to use for linking this executable.
  const char* linkLanguage =
    this->Target->GetLinkerLanguage(this->ConfigName);

  // Make sure we have a link language.
  if(!linkLanguage)
    {
    cmSystemTools::Error("Cannot determine link language for target \"",
                         this->Target->GetName(), "\".");
    return;
    }

  if(!this->NoRuleMessages)
    {
    // Add the link message.
    std::string buildEcho = "Linking ";
    buildEcho += linkLanguage;
    buildEcho += " executable ";
    buildEcho += targetOutPath;
    this->LocalGenerator->AppendEcho(commands, buildEcho.c_str(),
                                     cmLocalUnixMakefileGenerator3::EchoLink);
    }

  // Build a list of compiler flags and linker flags.
  std::string flags;
  std::string linkFlags;

  // Add flags to create an executable.
  this->LocalGenerator->
    AddConfigVariableFlags(linkFlags, "CMAKE_EXE_LINKER_FLAGS",
                           this->ConfigName);


  if(this->Target->GetPropertyAsBool("WIN32_EXECUTABLE"))
    {
    this->LocalGenerator->AppendFlags
      (linkFlags, this->Makefile->GetDefinition("CMAKE_CREATE_WIN32_EXE"));
    }
  else
    {
    this->LocalGenerator->AppendFlags
      (linkFlags, this->Makefile->GetDefinition("CMAKE_CREATE_CONSOLE_EXE"));
    }

  // Add symbol export flags if necessary.
  if(this->Target->IsExecutableWithExports())
    {
    std::string export_flag_var = "CMAKE_EXE_EXPORTS_";
    export_flag_var += linkLanguage;
    export_flag_var += "_FLAG";
    this->LocalGenerator->AppendFlags
      (linkFlags, this->Makefile->GetDefinition(export_flag_var.c_str()));
    }

  // Add language feature flags.
  this->AddFeatureFlags(flags, linkLanguage);

  this->LocalGenerator->AddArchitectureFlags(flags, this->Target,
                                             linkLanguage, this->ConfigName);

  // Add target-specific linker flags.
  this->LocalGenerator->AppendFlags
    (linkFlags, this->Target->GetProperty("LINK_FLAGS"));
  std::string linkFlagsConfig = "LINK_FLAGS_";
  linkFlagsConfig += cmSystemTools::UpperCase(this->ConfigName);
  this->LocalGenerator->AppendFlags
    (linkFlags, this->Target->GetProperty(linkFlagsConfig.c_str()));

  this->AddModuleDefinitionFlag(linkFlags);

  // Construct a list of files associated with this executable that
  // may need to be cleaned.
  std::vector<std::string> exeCleanFiles;
  exeCleanFiles.push_back(this->Convert(targetFullPath.c_str(),
                                        cmLocalGenerator::START_OUTPUT,
                                        cmLocalGenerator::UNCHANGED));
#ifdef _WIN32
  // There may be a manifest file for this target.  Add it to the
  // clean set just in case.
  exeCleanFiles.push_back(this->Convert((targetFullPath+".manifest").c_str(),
                                        cmLocalGenerator::START_OUTPUT,
                                        cmLocalGenerator::UNCHANGED));
#endif
  if(targetNameReal != targetName)
    {
    exeCleanFiles.push_back(this->Convert(targetFullPathReal.c_str(),
                                          cmLocalGenerator::START_OUTPUT,
                                          cmLocalGenerator::UNCHANGED));
    }
  if(!targetNameImport.empty())
    {
    exeCleanFiles.push_back(this->Convert(targetFullPathImport.c_str(),
                                          cmLocalGenerator::START_OUTPUT,
                                          cmLocalGenerator::UNCHANGED));
    std::string implib;
    if(this->Target->GetImplibGNUtoMS(targetFullPathImport, implib))
      {
      exeCleanFiles.push_back(this->Convert(implib.c_str(),
                                            cmLocalGenerator::START_OUTPUT,
                                            cmLocalGenerator::UNCHANGED));
      }
    }

  // List the PDB for cleaning only when the whole target is
  // cleaned.  We do not want to delete the .pdb file just before
  // linking the target.
  this->CleanFiles.push_back
    (this->Convert(targetFullPathPDB.c_str(),
                   cmLocalGenerator::START_OUTPUT,
                   cmLocalGenerator::UNCHANGED));

  // Add the pre-build and pre-link rules building but not when relinking.
  if(!relink)
    {
    this->LocalGenerator
      ->AppendCustomCommands(commands, this->Target->GetPreBuildCommands(),
                             this->Target);
    this->LocalGenerator
      ->AppendCustomCommands(commands, this->Target->GetPreLinkCommands(),
                             this->Target);
    }

  // Determine whether a link script will be used.
  bool useLinkScript = this->GlobalGenerator->GetUseLinkScript();

  // Construct the main link rule.
  std::vector<std::string> real_link_commands;
  std::string linkRuleVar = "CMAKE_";
  linkRuleVar += linkLanguage;
  linkRuleVar += "_LINK_EXECUTABLE";
  std::string linkRule = this->GetLinkRule(linkRuleVar.c_str());
  std::vector<std::string> commands1;
  cmSystemTools::ExpandListArgument(linkRule, real_link_commands);
  if(this->Target->IsExecutableWithExports())
    {
    // If a separate rule for creating an import library is specified
    // add it now.
    std::string implibRuleVar = "CMAKE_";
    implibRuleVar += linkLanguage;
    implibRuleVar += "_CREATE_IMPORT_LIBRARY";
    if(const char* rule =
       this->Makefile->GetDefinition(implibRuleVar.c_str()))
      {
      cmSystemTools::ExpandListArgument(rule, real_link_commands);
      }
    }

  // Select whether to use a response file for objects.
  bool useResponseFile = false;
  {
  std::string responseVar = "CMAKE_";
  responseVar += linkLanguage;
  responseVar += "_USE_RESPONSE_FILE_FOR_OBJECTS";
  if(this->Makefile->IsOn(responseVar.c_str()))
    {
    useResponseFile = true;
    }
  }

  // Expand the rule variables.
  {
  // Set path conversion for link script shells.
  this->LocalGenerator->SetLinkScriptShell(useLinkScript);

  // Collect up flags to link in needed libraries.
  cmOStringStream linklibs;
  this->LocalGenerator->OutputLinkLibraries(linklibs, *this->Target, relink);

  // Construct object file lists that may be needed to expand the
  // rule.
  std::string buildObjs;
  this->CreateObjectLists(useLinkScript, false, useResponseFile,
                          buildObjs, depends);

  cmLocalGenerator::RuleVariables vars;
  vars.RuleLauncher = "RULE_LAUNCH_LINK";
  vars.CMTarget = this->Target;
  vars.Language = linkLanguage;
  vars.Objects = buildObjs.c_str();
  std::string objdir = cmake::GetCMakeFilesDirectoryPostSlash();
  objdir += this->Target->GetName();
  objdir += ".dir";
  objdir = this->Convert(objdir.c_str(),
                         cmLocalGenerator::START_OUTPUT,
                         cmLocalGenerator::SHELL);
  vars.ObjectDir = objdir.c_str();
  vars.Target = targetOutPathReal.c_str();
  vars.TargetPDB = targetOutPathPDB.c_str();

  // Setup the target version.
  std::string targetVersionMajor;
  std::string targetVersionMinor;
  {
  cmOStringStream majorStream;
  cmOStringStream minorStream;
  int major;
  int minor;
  this->Target->GetTargetVersion(major, minor);
  majorStream << major;
  minorStream << minor;
  targetVersionMajor = majorStream.str();
  targetVersionMinor = minorStream.str();
  }
  vars.TargetVersionMajor = targetVersionMajor.c_str();
  vars.TargetVersionMinor = targetVersionMinor.c_str();

  std::string linkString = linklibs.str();
  vars.LinkLibraries = linkString.c_str();
  vars.Flags = flags.c_str();
  vars.LinkFlags = linkFlags.c_str();
  // Expand placeholders in the commands.
  this->LocalGenerator->TargetImplib = targetOutPathImport;
  for(std::vector<std::string>::iterator i = real_link_commands.begin();
      i != real_link_commands.end(); ++i)
    {
    this->LocalGenerator->ExpandRuleVariables(*i, vars);
    }
  this->LocalGenerator->TargetImplib = "";

  // Restore path conversion to normal shells.
  this->LocalGenerator->SetLinkScriptShell(false);
  }

  // Optionally convert the build rule to use a script to avoid long
  // command lines in the make shell.
  if(useLinkScript)
    {
    // Use a link script.
    const char* name = (relink? "relink.txt" : "link.txt");
    this->CreateLinkScript(name, real_link_commands, commands1, depends);
    }
  else
    {
    // No link script.  Just use the link rule directly.
    commands1 = real_link_commands;
    }
  this->LocalGenerator->CreateCDCommand
    (commands1,
     this->Makefile->GetStartOutputDirectory(),
     cmLocalGenerator::HOME_OUTPUT);
  commands.insert(commands.end(), commands1.begin(), commands1.end());
  commands1.clear();

  // Add a rule to create necessary symlinks for the library.
  if(targetOutPath != targetOutPathReal)
    {
    std::string symlink = "$(CMAKE_COMMAND) -E cmake_symlink_executable ";
    symlink += targetOutPathReal;
    symlink += " ";
    symlink += targetOutPath;
    commands1.push_back(symlink);
    this->LocalGenerator->CreateCDCommand(commands1,
                                  this->Makefile->GetStartOutputDirectory(),
                                  cmLocalGenerator::HOME_OUTPUT);
    commands.insert(commands.end(), commands1.begin(), commands1.end());
    commands1.clear();
    }

  // Add the post-build rules when building but not when relinking.
  if(!relink)
    {
    this->LocalGenerator->
      AppendCustomCommands(commands, this->Target->GetPostBuildCommands(),
                           this->Target);
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

  // Write the main driver rule to build everything in this target.
  this->WriteTargetDriverRule(targetFullPath.c_str(), relink);

  // Clean all the possible executable names and symlinks.
  this->CleanFiles.insert(this->CleanFiles.end(),
                          exeCleanFiles.begin(),
                          exeCleanFiles.end());
}

//----------------------------------------------------------------------------
void
cmMakefileExecutableTargetGenerator::CreateAppBundle(std::string& targetName,
                                                     std::string& outpath)
{
  // Compute bundle directory names.
  outpath = this->MacContentDirectory;
  outpath += "MacOS";
  cmSystemTools::MakeDirectory(outpath.c_str());
  this->Makefile->AddCMakeOutputFile(outpath.c_str());
  outpath += "/";

  // Configure the Info.plist file.  Note that it needs the executable name
  // to be set.
  std::string plist = this->MacContentDirectory + "Info.plist";
  this->LocalGenerator->GenerateAppleInfoPList(this->Target,
                                               targetName.c_str(),
                                               plist.c_str());
  this->Makefile->AddCMakeOutputFile(plist.c_str());
}
