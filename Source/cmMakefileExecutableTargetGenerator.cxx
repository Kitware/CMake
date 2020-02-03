/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmMakefileExecutableTargetGenerator.h"

#include <set>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include <cm/memory>
#include <cmext/algorithm>

#include "cmGeneratedFileStream.h"
#include "cmGeneratorTarget.h"
#include "cmGlobalUnixMakefileGenerator3.h"
#include "cmLinkLineComputer.h"
#include "cmLinkLineDeviceComputer.h"
#include "cmLocalGenerator.h"
#include "cmLocalUnixMakefileGenerator3.h"
#include "cmMakefile.h"
#include "cmOSXBundleGenerator.h"
#include "cmOutputConverter.h"
#include "cmRulePlaceholderExpander.h"
#include "cmState.h"
#include "cmStateDirectory.h"
#include "cmStateSnapshot.h"
#include "cmStateTypes.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"

cmMakefileExecutableTargetGenerator::cmMakefileExecutableTargetGenerator(
  cmGeneratorTarget* target)
  : cmMakefileTargetGenerator(target)
{
  this->CustomCommandDriver = OnDepends;
  this->TargetNames =
    this->GeneratorTarget->GetExecutableNames(this->GetConfigName());

  this->OSXBundleGenerator = cm::make_unique<cmOSXBundleGenerator>(target);
  this->OSXBundleGenerator->SetMacContentFolders(&this->MacContentFolders);
}

cmMakefileExecutableTargetGenerator::~cmMakefileExecutableTargetGenerator() =
  default;

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

  // write the device link rules
  this->WriteDeviceExecutableRule(false);

  // write the link rules
  this->WriteExecutableRule(false);
  if (this->GeneratorTarget->NeedRelinkBeforeInstall(this->GetConfigName())) {
    // Write rules to link an installable version of the target.
    this->WriteExecutableRule(true);
  }

  // Write clean target
  this->WriteTargetCleanRules();

  // Write the dependency generation rule.  This must be done last so
  // that multiple output pair information is available.
  this->WriteTargetDependRules();

  // close the streams
  this->CloseFileStreams();
}

void cmMakefileExecutableTargetGenerator::WriteDeviceExecutableRule(
  bool relink)
{
#ifndef CMAKE_BOOTSTRAP
  const bool requiresDeviceLinking = requireDeviceLinking(
    *this->GeneratorTarget, *this->LocalGenerator, this->GetConfigName());
  if (!requiresDeviceLinking) {
    return;
  }

  std::vector<std::string> commands;

  // Get the language to use for linking this library.
  std::string linkLanguage = "CUDA";
  std::string const& objExt =
    this->Makefile->GetSafeDefinition("CMAKE_CUDA_OUTPUT_EXTENSION");

  // Build list of dependencies.
  std::vector<std::string> depends;
  this->AppendLinkDepends(depends, linkLanguage);

  // Get the name of the device object to generate.
  std::string const targetOutputReal =
    this->GeneratorTarget->ObjectDirectory + "cmake_device_link" + objExt;
  this->DeviceLinkObject = targetOutputReal;

  this->NumberOfProgressActions++;
  if (!this->NoRuleMessages) {
    cmLocalUnixMakefileGenerator3::EchoProgress progress;
    this->MakeEchoProgress(progress);
    // Add the link message.
    std::string buildEcho =
      cmStrCat("Linking ", linkLanguage, " device code ",
               this->LocalGenerator->ConvertToOutputFormat(
                 this->LocalGenerator->MaybeConvertToRelativePath(
                   this->LocalGenerator->GetCurrentBinaryDirectory(),
                   this->DeviceLinkObject),
                 cmOutputConverter::SHELL));
    this->LocalGenerator->AppendEcho(
      commands, buildEcho, cmLocalUnixMakefileGenerator3::EchoLink, &progress);
  }

  // Build a list of compiler flags and linker flags.
  std::string flags;
  std::string linkFlags;

  // Add flags to create an executable.
  // Add symbol export flags if necessary.
  if (this->GeneratorTarget->IsExecutableWithExports()) {
    std::string export_flag_var =
      cmStrCat("CMAKE_EXE_EXPORTS_", linkLanguage, "_FLAG");
    this->LocalGenerator->AppendFlags(
      linkFlags, this->Makefile->GetSafeDefinition(export_flag_var));
  }

  this->LocalGenerator->AppendFlags(linkFlags,
                                    this->LocalGenerator->GetLinkLibsCMP0065(
                                      linkLanguage, *this->GeneratorTarget));

  // Add language feature flags.
  this->LocalGenerator->AddLanguageFlagsForLinking(
    flags, this->GeneratorTarget, linkLanguage, this->GetConfigName());

  this->LocalGenerator->AddArchitectureFlags(
    flags, this->GeneratorTarget, linkLanguage, this->GetConfigName());

  // Add target-specific linker flags.
  this->GetTargetLinkFlags(linkFlags, linkLanguage);

  // Construct a list of files associated with this executable that
  // may need to be cleaned.
  std::vector<std::string> exeCleanFiles;
  exeCleanFiles.push_back(this->LocalGenerator->MaybeConvertToRelativePath(
    this->LocalGenerator->GetCurrentBinaryDirectory(), targetOutputReal));

  // Determine whether a link script will be used.
  bool useLinkScript = this->GlobalGenerator->GetUseLinkScript();

  // Construct the main link rule.
  std::vector<std::string> real_link_commands;
  const std::string linkRuleVar = "CMAKE_CUDA_DEVICE_LINK_EXECUTABLE";
  const std::string linkRule = this->GetLinkRule(linkRuleVar);
  std::vector<std::string> commands1;
  cmExpandList(linkRule, real_link_commands);

  bool useResponseFileForObjects =
    this->CheckUseResponseFileForObjects(linkLanguage);
  bool const useResponseFileForLibs =
    this->CheckUseResponseFileForLibraries(linkLanguage);

  // Expand the rule variables.
  {
    bool useWatcomQuote =
      this->Makefile->IsOn(linkRuleVar + "_USE_WATCOM_QUOTE");

    // Set path conversion for link script shells.
    this->LocalGenerator->SetLinkScriptShell(useLinkScript);

    std::unique_ptr<cmLinkLineComputer> linkLineComputer(
      new cmLinkLineDeviceComputer(
        this->LocalGenerator,
        this->LocalGenerator->GetStateSnapshot().GetDirectory()));
    linkLineComputer->SetForResponse(useResponseFileForLibs);
    linkLineComputer->SetUseWatcomQuote(useWatcomQuote);
    linkLineComputer->SetRelink(relink);

    // Collect up flags to link in needed libraries.
    std::string linkLibs;
    this->CreateLinkLibs(linkLineComputer.get(), linkLibs,
                         useResponseFileForLibs, depends);

    // Construct object file lists that may be needed to expand the
    // rule.
    std::string buildObjs;
    this->CreateObjectLists(useLinkScript, false, useResponseFileForObjects,
                            buildObjs, depends, useWatcomQuote);

    std::string const& aixExports = this->GetAIXExports(this->GetConfigName());

    cmRulePlaceholderExpander::RuleVariables vars;
    std::string objectDir = this->GeneratorTarget->GetSupportDirectory();

    objectDir = this->LocalGenerator->ConvertToOutputFormat(
      this->LocalGenerator->MaybeConvertToRelativePath(
        this->LocalGenerator->GetCurrentBinaryDirectory(), objectDir),
      cmOutputConverter::SHELL);

    cmOutputConverter::OutputFormat output = (useWatcomQuote)
      ? cmOutputConverter::WATCOMQUOTE
      : cmOutputConverter::SHELL;
    std::string target = this->LocalGenerator->ConvertToOutputFormat(
      this->LocalGenerator->MaybeConvertToRelativePath(
        this->LocalGenerator->GetCurrentBinaryDirectory(), targetOutputReal),
      output);

    std::string targetFullPathCompilePDB =
      this->ComputeTargetCompilePDB(this->GetConfigName());
    std::string targetOutPathCompilePDB =
      this->LocalGenerator->ConvertToOutputFormat(targetFullPathCompilePDB,
                                                  cmOutputConverter::SHELL);

    vars.Language = linkLanguage.c_str();
    vars.AIXExports = aixExports.c_str();
    vars.Objects = buildObjs.c_str();
    vars.ObjectDir = objectDir.c_str();
    vars.Target = target.c_str();
    vars.LinkLibraries = linkLibs.c_str();
    vars.Flags = flags.c_str();
    vars.LinkFlags = linkFlags.c_str();
    vars.TargetCompilePDB = targetOutPathCompilePDB.c_str();

    std::string launcher;

    const char* val = this->LocalGenerator->GetRuleLauncher(
      this->GeneratorTarget, "RULE_LAUNCH_LINK");
    if (val && *val) {
      launcher = cmStrCat(val, ' ');
    }

    std::unique_ptr<cmRulePlaceholderExpander> rulePlaceholderExpander(
      this->LocalGenerator->CreateRulePlaceholderExpander());

    // Expand placeholders in the commands.
    rulePlaceholderExpander->SetTargetImpLib(targetOutputReal);
    for (std::string& real_link_command : real_link_commands) {
      real_link_command = cmStrCat(launcher, real_link_command);
      rulePlaceholderExpander->ExpandRuleVariables(this->LocalGenerator,
                                                   real_link_command, vars);
    }

    // Restore path conversion to normal shells.
    this->LocalGenerator->SetLinkScriptShell(false);
  }

  // Optionally convert the build rule to use a script to avoid long
  // command lines in the make shell.
  if (useLinkScript) {
    // Use a link script.
    const char* name = (relink ? "drelink.txt" : "dlink.txt");
    this->CreateLinkScript(name, real_link_commands, commands1, depends);
  } else {
    // No link script.  Just use the link rule directly.
    commands1 = real_link_commands;
  }
  this->LocalGenerator->CreateCDCommand(
    commands1, this->Makefile->GetCurrentBinaryDirectory(),
    this->LocalGenerator->GetBinaryDirectory());
  cm::append(commands, commands1);
  commands1.clear();

  // Write the build rule.
  this->LocalGenerator->WriteMakeRule(*this->BuildFileStream, nullptr,
                                      targetOutputReal, depends, commands,
                                      false);

  // Write the main driver rule to build everything in this target.
  this->WriteTargetDriverRule(targetOutputReal, relink);

  // Clean all the possible executable names and symlinks.
  this->CleanFiles.insert(exeCleanFiles.begin(), exeCleanFiles.end());
#else
  static_cast<void>(relink);
#endif
}

void cmMakefileExecutableTargetGenerator::WriteExecutableRule(bool relink)
{
  std::vector<std::string> commands;

  // Get the name of the executable to generate.
  cmGeneratorTarget::Names targetNames =
    this->GeneratorTarget->GetExecutableNames(this->GetConfigName());

  // Construct the full path version of the names.
  std::string outpath =
    this->GeneratorTarget->GetDirectory(this->GetConfigName());
  if (this->GeneratorTarget->IsAppBundleOnApple()) {
    this->OSXBundleGenerator->CreateAppBundle(targetNames.Output, outpath,
                                              this->GetConfigName());
  }
  outpath += '/';
  std::string outpathImp;
  if (relink) {
    outpath = cmStrCat(this->Makefile->GetCurrentBinaryDirectory(),
                       "/CMakeFiles/CMakeRelink.dir");
    cmSystemTools::MakeDirectory(outpath);
    outpath += '/';
    if (!targetNames.ImportLibrary.empty()) {
      outpathImp = outpath;
    }
  } else {
    cmSystemTools::MakeDirectory(outpath);
    if (!targetNames.ImportLibrary.empty()) {
      outpathImp = this->GeneratorTarget->GetDirectory(
        this->GetConfigName(), cmStateEnums::ImportLibraryArtifact);
      cmSystemTools::MakeDirectory(outpathImp);
      outpathImp += '/';
    }
  }

  std::string compilePdbOutputPath =
    this->GeneratorTarget->GetCompilePDBDirectory(this->GetConfigName());
  cmSystemTools::MakeDirectory(compilePdbOutputPath);

  std::string pdbOutputPath =
    this->GeneratorTarget->GetPDBDirectory(this->GetConfigName());
  cmSystemTools::MakeDirectory(pdbOutputPath);
  pdbOutputPath += '/';

  std::string targetFullPath = outpath + targetNames.Output;
  std::string targetFullPathReal = outpath + targetNames.Real;
  std::string targetFullPathPDB = pdbOutputPath + targetNames.PDB;
  std::string targetFullPathImport = outpathImp + targetNames.ImportLibrary;
  std::string targetOutPathPDB = this->LocalGenerator->ConvertToOutputFormat(
    targetFullPathPDB, cmOutputConverter::SHELL);
  // Convert to the output path to use in constructing commands.
  std::string targetOutPath = this->LocalGenerator->ConvertToOutputFormat(
    this->LocalGenerator->MaybeConvertToRelativePath(
      this->LocalGenerator->GetCurrentBinaryDirectory(), targetFullPath),
    cmOutputConverter::SHELL);
  std::string targetOutPathReal = this->LocalGenerator->ConvertToOutputFormat(
    this->LocalGenerator->MaybeConvertToRelativePath(
      this->LocalGenerator->GetCurrentBinaryDirectory(), targetFullPathReal),
    cmOutputConverter::SHELL);
  std::string targetOutPathImport =
    this->LocalGenerator->ConvertToOutputFormat(
      this->LocalGenerator->MaybeConvertToRelativePath(
        this->LocalGenerator->GetCurrentBinaryDirectory(),
        targetFullPathImport),
      cmOutputConverter::SHELL);

  // Get the language to use for linking this executable.
  std::string linkLanguage =
    this->GeneratorTarget->GetLinkerLanguage(this->GetConfigName());

  // Make sure we have a link language.
  if (linkLanguage.empty()) {
    cmSystemTools::Error("Cannot determine link language for target \"" +
                         this->GeneratorTarget->GetName() + "\".");
    return;
  }

  // Build list of dependencies.
  std::vector<std::string> depends;
  this->AppendLinkDepends(depends, linkLanguage);
  if (!this->DeviceLinkObject.empty()) {
    depends.push_back(this->DeviceLinkObject);
  }

  this->NumberOfProgressActions++;
  if (!this->NoRuleMessages) {
    cmLocalUnixMakefileGenerator3::EchoProgress progress;
    this->MakeEchoProgress(progress);
    // Add the link message.
    std::string buildEcho =
      cmStrCat("Linking ", linkLanguage, " executable ", targetOutPath);
    this->LocalGenerator->AppendEcho(
      commands, buildEcho, cmLocalUnixMakefileGenerator3::EchoLink, &progress);
  }

  // Build a list of compiler flags and linker flags.
  std::string flags;
  std::string linkFlags;

  // Add flags to create an executable.
  this->LocalGenerator->AddConfigVariableFlags(
    linkFlags, "CMAKE_EXE_LINKER_FLAGS", this->GetConfigName());

  if (this->GeneratorTarget->GetPropertyAsBool("WIN32_EXECUTABLE")) {
    this->LocalGenerator->AppendFlags(
      linkFlags, this->Makefile->GetSafeDefinition("CMAKE_CREATE_WIN32_EXE"));
  } else {
    this->LocalGenerator->AppendFlags(
      linkFlags,
      this->Makefile->GetSafeDefinition("CMAKE_CREATE_CONSOLE_EXE"));
  }

  // Add symbol export flags if necessary.
  if (this->GeneratorTarget->IsExecutableWithExports()) {
    std::string export_flag_var =
      cmStrCat("CMAKE_EXE_EXPORTS_", linkLanguage, "_FLAG");
    this->LocalGenerator->AppendFlags(
      linkFlags, this->Makefile->GetSafeDefinition(export_flag_var));
  }

  this->LocalGenerator->AppendFlags(linkFlags,
                                    this->LocalGenerator->GetLinkLibsCMP0065(
                                      linkLanguage, *this->GeneratorTarget));

  if (this->GeneratorTarget->GetPropertyAsBool("LINK_WHAT_YOU_USE")) {
    this->LocalGenerator->AppendFlags(linkFlags, " -Wl,--no-as-needed");
  }

  // Add language feature flags.
  this->LocalGenerator->AddLanguageFlagsForLinking(
    flags, this->GeneratorTarget, linkLanguage, this->GetConfigName());

  this->LocalGenerator->AddArchitectureFlags(
    flags, this->GeneratorTarget, linkLanguage, this->GetConfigName());

  // Add target-specific linker flags.
  this->GetTargetLinkFlags(linkFlags, linkLanguage);

  {
    std::unique_ptr<cmLinkLineComputer> linkLineComputer =
      this->CreateLinkLineComputer(
        this->LocalGenerator,
        this->LocalGenerator->GetStateSnapshot().GetDirectory());

    this->AddModuleDefinitionFlag(linkLineComputer.get(), linkFlags,
                                  this->GetConfigName());
  }

  this->LocalGenerator->AppendIPOLinkerFlags(
    linkFlags, this->GeneratorTarget, this->GetConfigName(), linkLanguage);

  // Construct a list of files associated with this executable that
  // may need to be cleaned.
  std::vector<std::string> exeCleanFiles;
  exeCleanFiles.push_back(this->LocalGenerator->MaybeConvertToRelativePath(
    this->LocalGenerator->GetCurrentBinaryDirectory(), targetFullPath));
#ifdef _WIN32
  // There may be a manifest file for this target.  Add it to the
  // clean set just in case.
  exeCleanFiles.push_back(this->LocalGenerator->MaybeConvertToRelativePath(
    this->LocalGenerator->GetCurrentBinaryDirectory(),
    targetFullPath + ".manifest"));
#endif
  if (this->TargetNames.Real != this->TargetNames.Output) {
    exeCleanFiles.push_back(this->LocalGenerator->MaybeConvertToRelativePath(
      this->LocalGenerator->GetCurrentBinaryDirectory(), targetFullPathReal));
  }
  if (!this->TargetNames.ImportLibrary.empty()) {
    exeCleanFiles.push_back(this->LocalGenerator->MaybeConvertToRelativePath(
      this->LocalGenerator->GetCurrentBinaryDirectory(),
      targetFullPathImport));
    std::string implib;
    if (this->GeneratorTarget->GetImplibGNUtoMS(
          this->GetConfigName(), targetFullPathImport, implib)) {
      exeCleanFiles.push_back(this->LocalGenerator->MaybeConvertToRelativePath(
        this->LocalGenerator->GetCurrentBinaryDirectory(), implib));
    }
  }

  // List the PDB for cleaning only when the whole target is
  // cleaned.  We do not want to delete the .pdb file just before
  // linking the target.
  this->CleanFiles.insert(this->LocalGenerator->MaybeConvertToRelativePath(
    this->LocalGenerator->GetCurrentBinaryDirectory(), targetFullPathPDB));

  // Add the pre-build and pre-link rules building but not when relinking.
  if (!relink) {
    this->LocalGenerator->AppendCustomCommands(
      commands, this->GeneratorTarget->GetPreBuildCommands(),
      this->GeneratorTarget, this->LocalGenerator->GetBinaryDirectory());
    this->LocalGenerator->AppendCustomCommands(
      commands, this->GeneratorTarget->GetPreLinkCommands(),
      this->GeneratorTarget, this->LocalGenerator->GetBinaryDirectory());
  }

  // Determine whether a link script will be used.
  bool useLinkScript = this->GlobalGenerator->GetUseLinkScript();

  // Construct the main link rule.
  std::vector<std::string> real_link_commands;
  std::string linkRuleVar = this->GeneratorTarget->GetCreateRuleVariable(
    linkLanguage, this->GetConfigName());
  std::string linkRule = this->GetLinkRule(linkRuleVar);
  std::vector<std::string> commands1;
  cmExpandList(linkRule, real_link_commands);
  if (this->GeneratorTarget->IsExecutableWithExports()) {
    // If a separate rule for creating an import library is specified
    // add it now.
    std::string implibRuleVar =
      cmStrCat("CMAKE_", linkLanguage, "_CREATE_IMPORT_LIBRARY");
    if (const char* rule = this->Makefile->GetDefinition(implibRuleVar)) {
      cmExpandList(rule, real_link_commands);
    }
  }

  bool useResponseFileForObjects =
    this->CheckUseResponseFileForObjects(linkLanguage);
  bool const useResponseFileForLibs =
    this->CheckUseResponseFileForLibraries(linkLanguage);

  // Expand the rule variables.
  {
    bool useWatcomQuote =
      this->Makefile->IsOn(linkRuleVar + "_USE_WATCOM_QUOTE");

    // Set path conversion for link script shells.
    this->LocalGenerator->SetLinkScriptShell(useLinkScript);

    std::unique_ptr<cmLinkLineComputer> linkLineComputer =
      this->CreateLinkLineComputer(
        this->LocalGenerator,
        this->LocalGenerator->GetStateSnapshot().GetDirectory());
    linkLineComputer->SetForResponse(useResponseFileForLibs);
    linkLineComputer->SetUseWatcomQuote(useWatcomQuote);
    linkLineComputer->SetRelink(relink);

    // Collect up flags to link in needed libraries.
    std::string linkLibs;
    this->CreateLinkLibs(linkLineComputer.get(), linkLibs,
                         useResponseFileForLibs, depends);

    // Construct object file lists that may be needed to expand the
    // rule.
    std::string buildObjs;
    this->CreateObjectLists(useLinkScript, false, useResponseFileForObjects,
                            buildObjs, depends, useWatcomQuote);
    if (!this->DeviceLinkObject.empty()) {
      buildObjs += " " +
        this->LocalGenerator->ConvertToOutputFormat(
          this->LocalGenerator->MaybeConvertToRelativePath(
            this->LocalGenerator->GetCurrentBinaryDirectory(),
            this->DeviceLinkObject),
          cmOutputConverter::SHELL);
    }

    // maybe create .def file from list of objects
    this->GenDefFile(real_link_commands);

    std::string manifests = this->GetManifests(this->GetConfigName());

    cmRulePlaceholderExpander::RuleVariables vars;
    vars.CMTargetName = this->GeneratorTarget->GetName().c_str();
    vars.CMTargetType =
      cmState::GetTargetTypeName(this->GeneratorTarget->GetType());
    vars.Language = linkLanguage.c_str();
    vars.Objects = buildObjs.c_str();
    std::string objectDir = this->GeneratorTarget->GetSupportDirectory();

    objectDir = this->LocalGenerator->ConvertToOutputFormat(
      this->LocalGenerator->MaybeConvertToRelativePath(
        this->LocalGenerator->GetCurrentBinaryDirectory(), objectDir),
      cmOutputConverter::SHELL);
    vars.ObjectDir = objectDir.c_str();
    cmOutputConverter::OutputFormat output = (useWatcomQuote)
      ? cmOutputConverter::WATCOMQUOTE
      : cmOutputConverter::SHELL;
    std::string target = this->LocalGenerator->ConvertToOutputFormat(
      this->LocalGenerator->MaybeConvertToRelativePath(
        this->LocalGenerator->GetCurrentBinaryDirectory(), targetFullPathReal),
      output);
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

    if (this->GeneratorTarget->GetPropertyAsBool("LINK_WHAT_YOU_USE")) {
      std::string cmakeCommand =
        cmStrCat(this->LocalGenerator->ConvertToOutputFormat(
                   cmSystemTools::GetCMakeCommand(), cmLocalGenerator::SHELL),
                 " -E __run_co_compile --lwyu=", targetOutPathReal);
      real_link_commands.push_back(std::move(cmakeCommand));
    }

    std::string launcher;

    const char* val = this->LocalGenerator->GetRuleLauncher(
      this->GeneratorTarget, "RULE_LAUNCH_LINK");
    if (val && *val) {
      launcher = cmStrCat(val, ' ');
    }

    std::unique_ptr<cmRulePlaceholderExpander> rulePlaceholderExpander(
      this->LocalGenerator->CreateRulePlaceholderExpander());

    // Expand placeholders in the commands.
    rulePlaceholderExpander->SetTargetImpLib(targetOutPathImport);
    for (std::string& real_link_command : real_link_commands) {
      real_link_command = cmStrCat(launcher, real_link_command);
      rulePlaceholderExpander->ExpandRuleVariables(this->LocalGenerator,
                                                   real_link_command, vars);
    }

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
    this->LocalGenerator->GetBinaryDirectory());
  cm::append(commands, commands1);
  commands1.clear();

  // Add a rule to create necessary symlinks for the library.
  if (targetOutPath != targetOutPathReal) {
    std::string symlink =
      cmStrCat("$(CMAKE_COMMAND) -E cmake_symlink_executable ",
               targetOutPathReal, ' ', targetOutPath);
    commands1.push_back(std::move(symlink));
    this->LocalGenerator->CreateCDCommand(
      commands1, this->Makefile->GetCurrentBinaryDirectory(),
      this->LocalGenerator->GetBinaryDirectory());
    cm::append(commands, commands1);
    commands1.clear();
  }

  // Add the post-build rules when building but not when relinking.
  if (!relink) {
    this->LocalGenerator->AppendCustomCommands(
      commands, this->GeneratorTarget->GetPostBuildCommands(),
      this->GeneratorTarget, this->LocalGenerator->GetBinaryDirectory());
  }

  // Write the build rule.
  this->LocalGenerator->WriteMakeRule(*this->BuildFileStream, nullptr,
                                      targetFullPathReal, depends, commands,
                                      false);

  // The symlink name for the target should depend on the real target
  // so if the target version changes it rebuilds and recreates the
  // symlink.
  if (targetFullPath != targetFullPathReal) {
    depends.clear();
    commands.clear();
    depends.push_back(targetFullPathReal);
    this->LocalGenerator->WriteMakeRule(*this->BuildFileStream, nullptr,
                                        targetFullPath, depends, commands,
                                        false);
  }

  // Write the main driver rule to build everything in this target.
  this->WriteTargetDriverRule(targetFullPath, relink);

  // Clean all the possible executable names and symlinks.
  this->CleanFiles.insert(exeCleanFiles.begin(), exeCleanFiles.end());
}
