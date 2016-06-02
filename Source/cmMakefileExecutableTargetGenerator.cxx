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
#include "cmake.h"

cmMakefileExecutableTargetGenerator::cmMakefileExecutableTargetGenerator(
  cmGeneratorTarget* target)
  : cmMakefileTargetGenerator(target)
{
  this->CustomCommandDriver = OnDepends;
  this->GeneratorTarget->GetExecutableNames(
    this->TargetNameOut, this->TargetNameReal, this->TargetNameImport,
    this->TargetNamePDB, this->ConfigName);

  this->OSXBundleGenerator =
    new cmOSXBundleGenerator(target, this->ConfigName);
  this->OSXBundleGenerator->SetMacContentFolders(&this->MacContentFolders);
}

cmMakefileExecutableTargetGenerator::~cmMakefileExecutableTargetGenerator()
{
  delete this->OSXBundleGenerator;
}

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
  if (this->GeneratorTarget->NeedRelinkBeforeInstall(this->ConfigName)) {
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
  this->GeneratorTarget->GetExecutableNames(targetName, targetNameReal,
                                            targetNameImport, targetNamePDB,
                                            this->ConfigName);

  // Construct the full path version of the names.
  std::string outpath = this->GeneratorTarget->GetDirectory(this->ConfigName);
  if (this->GeneratorTarget->IsAppBundleOnApple()) {
    this->OSXBundleGenerator->CreateAppBundle(targetName, outpath);
  }
  outpath += "/";
  std::string outpathImp;
  if (relink) {
    outpath = this->Makefile->GetCurrentBinaryDirectory();
    outpath += cmake::GetCMakeFilesDirectory();
    outpath += "/CMakeRelink.dir";
    cmSystemTools::MakeDirectory(outpath.c_str());
    outpath += "/";
    if (!targetNameImport.empty()) {
      outpathImp = outpath;
    }
  } else {
    cmSystemTools::MakeDirectory(outpath.c_str());
    if (!targetNameImport.empty()) {
      outpathImp = this->GeneratorTarget->GetDirectory(this->ConfigName, true);
      cmSystemTools::MakeDirectory(outpathImp.c_str());
      outpathImp += "/";
    }
  }

  std::string compilePdbOutputPath =
    this->GeneratorTarget->GetCompilePDBDirectory(this->ConfigName);
  cmSystemTools::MakeDirectory(compilePdbOutputPath.c_str());

  std::string pdbOutputPath =
    this->GeneratorTarget->GetPDBDirectory(this->ConfigName);
  cmSystemTools::MakeDirectory(pdbOutputPath.c_str());
  pdbOutputPath += "/";

  std::string targetFullPath = outpath + targetName;
  std::string targetFullPathReal = outpath + targetNameReal;
  std::string targetFullPathPDB = pdbOutputPath + targetNamePDB;
  std::string targetFullPathImport = outpathImp + targetNameImport;
  std::string targetOutPathPDB = this->Convert(
    targetFullPathPDB, cmOutputConverter::NONE, cmOutputConverter::SHELL);
  // Convert to the output path to use in constructing commands.
  std::string targetOutPath = this->Convert(
    targetFullPath, cmOutputConverter::START_OUTPUT, cmOutputConverter::SHELL);
  std::string targetOutPathReal =
    this->Convert(targetFullPathReal, cmOutputConverter::START_OUTPUT,
                  cmOutputConverter::SHELL);
  std::string targetOutPathImport =
    this->Convert(targetFullPathImport, cmOutputConverter::START_OUTPUT,
                  cmOutputConverter::SHELL);

  // Get the language to use for linking this executable.
  std::string linkLanguage =
    this->GeneratorTarget->GetLinkerLanguage(this->ConfigName);

  // Make sure we have a link language.
  if (linkLanguage.empty()) {
    cmSystemTools::Error("Cannot determine link language for target \"",
                         this->GeneratorTarget->GetName().c_str(), "\".");
    return;
  }

  this->NumberOfProgressActions++;
  if (!this->NoRuleMessages) {
    cmLocalUnixMakefileGenerator3::EchoProgress progress;
    this->MakeEchoProgress(progress);
    // Add the link message.
    std::string buildEcho = "Linking ";
    buildEcho += linkLanguage;
    buildEcho += " executable ";
    buildEcho += targetOutPath;
    this->LocalGenerator->AppendEcho(
      commands, buildEcho, cmLocalUnixMakefileGenerator3::EchoLink, &progress);
  }

  // Build a list of compiler flags and linker flags.
  std::string flags;
  std::string linkFlags;

  // Add flags to create an executable.
  this->LocalGenerator->AddConfigVariableFlags(
    linkFlags, "CMAKE_EXE_LINKER_FLAGS", this->ConfigName);

  if (this->GeneratorTarget->GetPropertyAsBool("WIN32_EXECUTABLE")) {
    this->LocalGenerator->AppendFlags(
      linkFlags, this->Makefile->GetDefinition("CMAKE_CREATE_WIN32_EXE"));
  } else {
    this->LocalGenerator->AppendFlags(
      linkFlags, this->Makefile->GetDefinition("CMAKE_CREATE_CONSOLE_EXE"));
  }

  // Add symbol export flags if necessary.
  if (this->GeneratorTarget->IsExecutableWithExports()) {
    std::string export_flag_var = "CMAKE_EXE_EXPORTS_";
    export_flag_var += linkLanguage;
    export_flag_var += "_FLAG";
    this->LocalGenerator->AppendFlags(
      linkFlags, this->Makefile->GetDefinition(export_flag_var));
  }
  if(this->GeneratorTarget->GetProperty("LINK_WHAT_YOU_USE")) {
    this->LocalGenerator->AppendFlags(linkFlags,
                                      " -Wl,--no-as-needed");
  }

  // Add language feature flags.
  this->AddFeatureFlags(flags, linkLanguage);

  this->LocalGenerator->AddArchitectureFlags(flags, this->GeneratorTarget,
                                             linkLanguage, this->ConfigName);

  // Add target-specific linker flags.
  this->LocalGenerator->AppendFlags(
    linkFlags, this->GeneratorTarget->GetProperty("LINK_FLAGS"));
  std::string linkFlagsConfig = "LINK_FLAGS_";
  linkFlagsConfig += cmSystemTools::UpperCase(this->ConfigName);
  this->LocalGenerator->AppendFlags(
    linkFlags, this->GeneratorTarget->GetProperty(linkFlagsConfig));

  this->AddModuleDefinitionFlag(linkFlags);

  // Construct a list of files associated with this executable that
  // may need to be cleaned.
  std::vector<std::string> exeCleanFiles;
  exeCleanFiles.push_back(this->Convert(targetFullPath,
                                        cmOutputConverter::START_OUTPUT,
                                        cmOutputConverter::UNCHANGED));
#ifdef _WIN32
  // There may be a manifest file for this target.  Add it to the
  // clean set just in case.
  exeCleanFiles.push_back(this->Convert((targetFullPath + ".manifest").c_str(),
                                        cmOutputConverter::START_OUTPUT,
                                        cmOutputConverter::UNCHANGED));
#endif
  if (targetNameReal != targetName) {
    exeCleanFiles.push_back(this->Convert(targetFullPathReal,
                                          cmOutputConverter::START_OUTPUT,
                                          cmOutputConverter::UNCHANGED));
  }
  if (!targetNameImport.empty()) {
    exeCleanFiles.push_back(this->Convert(targetFullPathImport,
                                          cmOutputConverter::START_OUTPUT,
                                          cmOutputConverter::UNCHANGED));
    std::string implib;
    if (this->GeneratorTarget->GetImplibGNUtoMS(targetFullPathImport,
                                                implib)) {
      exeCleanFiles.push_back(this->Convert(implib,
                                            cmOutputConverter::START_OUTPUT,
                                            cmOutputConverter::UNCHANGED));
    }
  }

  // List the PDB for cleaning only when the whole target is
  // cleaned.  We do not want to delete the .pdb file just before
  // linking the target.
  this->CleanFiles.push_back(this->Convert(targetFullPathPDB,
                                           cmOutputConverter::START_OUTPUT,
                                           cmOutputConverter::UNCHANGED));

  // Add the pre-build and pre-link rules building but not when relinking.
  if (!relink) {
    this->LocalGenerator->AppendCustomCommands(
      commands, this->GeneratorTarget->GetPreBuildCommands(),
      this->GeneratorTarget);
    this->LocalGenerator->AppendCustomCommands(
      commands, this->GeneratorTarget->GetPreLinkCommands(),
      this->GeneratorTarget);
  }

  // Determine whether a link script will be used.
  bool useLinkScript = this->GlobalGenerator->GetUseLinkScript();

  // Construct the main link rule.
  std::vector<std::string> real_link_commands;
  std::string linkRuleVar = "CMAKE_";
  linkRuleVar += linkLanguage;
  linkRuleVar += "_LINK_EXECUTABLE";
  std::string linkRule = this->GetLinkRule(linkRuleVar);
  std::vector<std::string> commands1;
  cmSystemTools::ExpandListArgument(linkRule, real_link_commands);
  if (this->GeneratorTarget->IsExecutableWithExports()) {
    // If a separate rule for creating an import library is specified
    // add it now.
    std::string implibRuleVar = "CMAKE_";
    implibRuleVar += linkLanguage;
    implibRuleVar += "_CREATE_IMPORT_LIBRARY";
    if (const char* rule = this->Makefile->GetDefinition(implibRuleVar)) {
      cmSystemTools::ExpandListArgument(rule, real_link_commands);
    }
  }

  // Select whether to use a response file for objects.
  bool useResponseFileForObjects = false;
  {
    std::string responseVar = "CMAKE_";
    responseVar += linkLanguage;
    responseVar += "_USE_RESPONSE_FILE_FOR_OBJECTS";
    if (this->Makefile->IsOn(responseVar)) {
      useResponseFileForObjects = true;
    }
  }

  // Select whether to use a response file for libraries.
  bool useResponseFileForLibs = false;
  {
    std::string responseVar = "CMAKE_";
    responseVar += linkLanguage;
    responseVar += "_USE_RESPONSE_FILE_FOR_LIBRARIES";
    if (this->Makefile->IsOn(responseVar)) {
      useResponseFileForLibs = true;
    }
  }

  // Expand the rule variables.
  {
    bool useWatcomQuote =
      this->Makefile->IsOn(linkRuleVar + "_USE_WATCOM_QUOTE");

    // Set path conversion for link script shells.
    this->LocalGenerator->SetLinkScriptShell(useLinkScript);

    // Collect up flags to link in needed libraries.
    std::string linkLibs;
    this->CreateLinkLibs(linkLibs, relink, useResponseFileForLibs, depends,
                         useWatcomQuote);

    // Construct object file lists that may be needed to expand the
    // rule.
    std::string buildObjs;
    this->CreateObjectLists(useLinkScript, false, useResponseFileForObjects,
                            buildObjs, depends, useWatcomQuote);

    std::string manifests = this->GetManifests();

    cmLocalGenerator::RuleVariables vars;
    vars.RuleLauncher = "RULE_LAUNCH_LINK";
    vars.CMTarget = this->GeneratorTarget;
    vars.Language = linkLanguage.c_str();
    vars.Objects = buildObjs.c_str();
    std::string objectDir = this->GeneratorTarget->GetSupportDirectory();
    objectDir = this->Convert(objectDir, cmOutputConverter::START_OUTPUT,
                              cmOutputConverter::SHELL);
    vars.ObjectDir = objectDir.c_str();
    cmOutputConverter::OutputFormat output = (useWatcomQuote)
      ? cmOutputConverter::WATCOMQUOTE
      : cmOutputConverter::SHELL;
    std::string target = this->Convert(
      targetFullPathReal, cmOutputConverter::START_OUTPUT, output);
    vars.Target = target.c_str();
    vars.TargetPDB = targetOutPathPDB.c_str();

    // Setup the target version.
    std::string targetVersionMajor;
    std::string targetVersionMinor;
    {
      std::ostringstream majorStream;
      std::ostringstream minorStream;
      int major;
      int minor;
      this->GeneratorTarget->GetTargetVersion(major, minor);
      majorStream << major;
      minorStream << minor;
      targetVersionMajor = majorStream.str();
      targetVersionMinor = minorStream.str();
    }
    vars.TargetVersionMajor = targetVersionMajor.c_str();
    vars.TargetVersionMinor = targetVersionMinor.c_str();

    vars.LinkLibraries = linkLibs.c_str();
    vars.Flags = flags.c_str();
    vars.LinkFlags = linkFlags.c_str();
    vars.Manifests = manifests.c_str();

    if(this->GeneratorTarget->GetProperty("LINK_WHAT_YOU_USE")) {
      std::string cmakeCommand =
        this->Convert(
          cmSystemTools::GetCMakeCommand(), cmLocalGenerator::NONE,
          cmLocalGenerator::SHELL);
      cmakeCommand += " -E __run_iwyu --lwyu=";
      cmakeCommand += targetOutPathReal;
      real_link_commands.push_back(cmakeCommand);
    }

    // Expand placeholders in the commands.
    this->LocalGenerator->TargetImplib = targetOutPathImport;
    for (std::vector<std::string>::iterator i = real_link_commands.begin();
         i != real_link_commands.end(); ++i) {
      this->LocalGenerator->ExpandRuleVariables(*i, vars);
    }
    this->LocalGenerator->TargetImplib = "";

    // Restore path conversion to normal shells.
    this->LocalGenerator->SetLinkScriptShell(false);
  }

  // Optionally convert the build rule to use a script to avoid long
  // command lines in the make shell.
  if (useLinkScript) {
    // Use a link script.
    const char* name = (relink ? "relink.txt" : "link.txt");
    this->CreateLinkScript(name, real_link_commands, commands1, depends);
  } else {
    // No link script.  Just use the link rule directly.
    commands1 = real_link_commands;
  }
  this->LocalGenerator->CreateCDCommand(
    commands1, this->Makefile->GetCurrentBinaryDirectory(),
    cmOutputConverter::HOME_OUTPUT);
  commands.insert(commands.end(), commands1.begin(), commands1.end());
  commands1.clear();

  // Add a rule to create necessary symlinks for the library.
  if (targetOutPath != targetOutPathReal) {
    std::string symlink = "$(CMAKE_COMMAND) -E cmake_symlink_executable ";
    symlink += targetOutPathReal;
    symlink += " ";
    symlink += targetOutPath;
    commands1.push_back(symlink);
    this->LocalGenerator->CreateCDCommand(
      commands1, this->Makefile->GetCurrentBinaryDirectory(),
      cmOutputConverter::HOME_OUTPUT);
    commands.insert(commands.end(), commands1.begin(), commands1.end());
    commands1.clear();
  }

  // Add the post-build rules when building but not when relinking.
  if (!relink) {
    this->LocalGenerator->AppendCustomCommands(
      commands, this->GeneratorTarget->GetPostBuildCommands(),
      this->GeneratorTarget);
  }

  // Write the build rule.
  this->LocalGenerator->WriteMakeRule(
    *this->BuildFileStream, 0, targetFullPathReal, depends, commands, false);

  // The symlink name for the target should depend on the real target
  // so if the target version changes it rebuilds and recreates the
  // symlink.
  if (targetFullPath != targetFullPathReal) {
    depends.clear();
    commands.clear();
    depends.push_back(targetFullPathReal);
    this->LocalGenerator->WriteMakeRule(
      *this->BuildFileStream, 0, targetFullPath, depends, commands, false);
  }

  // Write the main driver rule to build everything in this target.
  this->WriteTargetDriverRule(targetFullPath, relink);

  // Clean all the possible executable names and symlinks.
  this->CleanFiles.insert(this->CleanFiles.end(), exeCleanFiles.begin(),
                          exeCleanFiles.end());
}
