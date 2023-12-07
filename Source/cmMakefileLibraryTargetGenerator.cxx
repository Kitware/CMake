/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmMakefileLibraryTargetGenerator.h"

#include <cstddef>
#include <set>
#include <sstream>
#include <utility>
#include <vector>

#include <cm/memory>
#include <cmext/algorithm>

#include "cmGeneratedFileStream.h"
#include "cmGeneratorTarget.h"
#include "cmGlobalUnixMakefileGenerator3.h"
#include "cmLinkLineComputer.h"
#include "cmLinkLineDeviceComputer.h"
#include "cmList.h"
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
#include "cmValue.h"

cmMakefileLibraryTargetGenerator::cmMakefileLibraryTargetGenerator(
  cmGeneratorTarget* target)
  : cmMakefileTargetGenerator(target)
{
  this->CustomCommandDriver = OnDepends;
  if (this->GeneratorTarget->GetType() != cmStateEnums::INTERFACE_LIBRARY) {
    this->TargetNames =
      this->GeneratorTarget->GetLibraryNames(this->GetConfigName());
  }

  this->OSXBundleGenerator = cm::make_unique<cmOSXBundleGenerator>(target);
  this->OSXBundleGenerator->SetMacContentFolders(&this->MacContentFolders);
}

cmMakefileLibraryTargetGenerator::~cmMakefileLibraryTargetGenerator() =
  default;

void cmMakefileLibraryTargetGenerator::WriteRuleFiles()
{
  // create the build.make file and directory, put in the common blocks
  this->CreateRuleFile();

  // write rules used to help build object files
  this->WriteCommonCodeRules();

  // write the per-target per-language flags
  this->WriteTargetLanguageFlags();

  // write in rules for object files and custom commands
  this->WriteTargetBuildRules();

  // Write in the rules for the link dependency file
  this->WriteTargetLinkDependRules();

  // write the link rules
  // Write the rule for this target type.
  switch (this->GeneratorTarget->GetType()) {
    case cmStateEnums::STATIC_LIBRARY:
      this->WriteStaticLibraryRules();
      break;
    case cmStateEnums::SHARED_LIBRARY:
      this->WriteSharedLibraryRules(false);
      if (this->GeneratorTarget->NeedRelinkBeforeInstall(
            this->GetConfigName())) {
        // Write rules to link an installable version of the target.
        this->WriteSharedLibraryRules(true);
      }
      break;
    case cmStateEnums::MODULE_LIBRARY:
      this->WriteModuleLibraryRules(false);
      if (this->GeneratorTarget->NeedRelinkBeforeInstall(
            this->GetConfigName())) {
        // Write rules to link an installable version of the target.
        this->WriteModuleLibraryRules(true);
      }
      break;
    case cmStateEnums::OBJECT_LIBRARY:
      this->WriteObjectLibraryRules();
      break;
    default:
      // If language is not known, this is an error.
      cmSystemTools::Error("Unknown Library Type");
      break;
  }

  // Write clean target
  this->WriteTargetCleanRules();

  // Write the dependency generation rule.  This must be done last so
  // that multiple output pair information is available.
  this->WriteTargetDependRules();

  // close the streams
  this->CloseFileStreams();
}

void cmMakefileLibraryTargetGenerator::WriteObjectLibraryRules()
{
  std::vector<std::string> commands;
  std::vector<std::string> depends;

  // Add post-build rules.
  this->LocalGenerator->AppendCustomCommands(
    commands, this->GeneratorTarget->GetPostBuildCommands(),
    this->GeneratorTarget, this->LocalGenerator->GetBinaryDirectory());

  // Depend on the object files.
  this->AppendObjectDepends(depends);

  // Write the rule.
  this->LocalGenerator->WriteMakeRule(*this->BuildFileStream, nullptr,
                                      this->GeneratorTarget->GetName(),
                                      depends, commands, true);

  // Write the main driver rule to build everything in this target.
  this->WriteTargetDriverRule(this->GeneratorTarget->GetName(), false);
}

void cmMakefileLibraryTargetGenerator::WriteStaticLibraryRules()
{
  const bool requiresDeviceLinking = requireDeviceLinking(
    *this->GeneratorTarget, *this->LocalGenerator, this->GetConfigName());
  if (requiresDeviceLinking) {
    this->WriteDeviceLibraryRules("CMAKE_CUDA_DEVICE_LINK_LIBRARY", false);
  }

  std::string linkLanguage =
    this->GeneratorTarget->GetLinkerLanguage(this->GetConfigName());

  std::string linkRuleVar = this->GeneratorTarget->GetCreateRuleVariable(
    linkLanguage, this->GetConfigName());

  std::string extraFlags;
  this->LocalGenerator->GetStaticLibraryFlags(
    extraFlags, this->GetConfigName(), linkLanguage, this->GeneratorTarget);
  this->WriteLibraryRules(linkRuleVar, extraFlags, false);
}

void cmMakefileLibraryTargetGenerator::WriteSharedLibraryRules(bool relink)
{
  if (this->GeneratorTarget->IsFrameworkOnApple()) {
    this->WriteFrameworkRules(relink);
    return;
  }

  if (!relink) {
    const bool requiresDeviceLinking = requireDeviceLinking(
      *this->GeneratorTarget, *this->LocalGenerator, this->GetConfigName());
    if (requiresDeviceLinking) {
      this->WriteDeviceLibraryRules("CMAKE_CUDA_DEVICE_LINK_LIBRARY", relink);
    }
  }

  std::string linkLanguage =
    this->GeneratorTarget->GetLinkerLanguage(this->GetConfigName());
  std::string linkRuleVar =
    cmStrCat("CMAKE_", linkLanguage, "_CREATE_SHARED_LIBRARY");

  std::string extraFlags;
  this->GetTargetLinkFlags(extraFlags, linkLanguage);
  this->LocalGenerator->AddConfigVariableFlags(
    extraFlags, "CMAKE_SHARED_LINKER_FLAGS", this->GetConfigName());

  std::unique_ptr<cmLinkLineComputer> linkLineComputer =
    this->CreateLinkLineComputer(
      this->LocalGenerator,
      this->LocalGenerator->GetStateSnapshot().GetDirectory());

  this->LocalGenerator->AppendModuleDefinitionFlag(
    extraFlags, this->GeneratorTarget, linkLineComputer.get(),
    this->GetConfigName());

  this->UseLWYU = this->LocalGenerator->AppendLWYUFlags(
    extraFlags, this->GeneratorTarget, linkLanguage);

  this->WriteLibraryRules(linkRuleVar, extraFlags, relink);
}

void cmMakefileLibraryTargetGenerator::WriteModuleLibraryRules(bool relink)
{
  if (!relink) {
    const bool requiresDeviceLinking = requireDeviceLinking(
      *this->GeneratorTarget, *this->LocalGenerator, this->GetConfigName());
    if (requiresDeviceLinking) {
      this->WriteDeviceLibraryRules("CMAKE_CUDA_DEVICE_LINK_LIBRARY", relink);
    }
  }

  std::string linkLanguage =
    this->GeneratorTarget->GetLinkerLanguage(this->GetConfigName());
  std::string linkRuleVar =
    cmStrCat("CMAKE_", linkLanguage, "_CREATE_SHARED_MODULE");

  std::string extraFlags;
  this->GetTargetLinkFlags(extraFlags, linkLanguage);
  this->LocalGenerator->AddConfigVariableFlags(
    extraFlags, "CMAKE_MODULE_LINKER_FLAGS", this->GetConfigName());

  std::unique_ptr<cmLinkLineComputer> linkLineComputer =
    this->CreateLinkLineComputer(
      this->LocalGenerator,
      this->LocalGenerator->GetStateSnapshot().GetDirectory());

  this->LocalGenerator->AppendModuleDefinitionFlag(
    extraFlags, this->GeneratorTarget, linkLineComputer.get(),
    this->GetConfigName());

  this->UseLWYU = this->LocalGenerator->AppendLWYUFlags(
    extraFlags, this->GeneratorTarget, linkLanguage);

  this->WriteLibraryRules(linkRuleVar, extraFlags, relink);
}

void cmMakefileLibraryTargetGenerator::WriteFrameworkRules(bool relink)
{
  std::string linkLanguage =
    this->GeneratorTarget->GetLinkerLanguage(this->GetConfigName());
  std::string linkRuleVar =
    cmStrCat("CMAKE_", linkLanguage, "_CREATE_MACOSX_FRAMEWORK");

  std::string extraFlags;
  this->GetTargetLinkFlags(extraFlags, linkLanguage);
  this->LocalGenerator->AddConfigVariableFlags(
    extraFlags, "CMAKE_MACOSX_FRAMEWORK_LINKER_FLAGS", this->GetConfigName());

  this->WriteLibraryRules(linkRuleVar, extraFlags, relink);
}

void cmMakefileLibraryTargetGenerator::WriteDeviceLibraryRules(
  const std::string& linkRuleVar, bool relink)
{
#ifndef CMAKE_BOOTSTRAP
  // TODO: Merge the methods that call this method to avoid
  // code duplication.
  std::vector<std::string> commands;
  std::string const objExt =
    this->Makefile->GetSafeDefinition("CMAKE_CUDA_OUTPUT_EXTENSION");

  // Get the name of the device object to generate.
  std::string const targetOutput =
    this->GeneratorTarget->ObjectDirectory + "cmake_device_link" + objExt;
  this->DeviceLinkObject = targetOutput;

  this->NumberOfProgressActions++;
  if (!this->NoRuleMessages) {
    cmLocalUnixMakefileGenerator3::EchoProgress progress;
    this->MakeEchoProgress(progress);
    // Add the link message.
    std::string buildEcho = cmStrCat(
      "Linking CUDA device code ",
      this->LocalGenerator->ConvertToOutputFormat(
        this->LocalGenerator->MaybeRelativeToCurBinDir(this->DeviceLinkObject),
        cmOutputConverter::SHELL));
    this->LocalGenerator->AppendEcho(
      commands, buildEcho, cmLocalUnixMakefileGenerator3::EchoLink, &progress);
  }

  if (this->Makefile->GetSafeDefinition("CMAKE_CUDA_COMPILER_ID") == "Clang") {
    this->WriteDeviceLinkRule(commands, targetOutput);
  } else {
    this->WriteNvidiaDeviceLibraryRules(linkRuleVar, relink, commands,
                                        targetOutput);
  }

  // Write the main driver rule to build everything in this target.
  this->WriteTargetDriverRule(targetOutput, relink);
}

void cmMakefileLibraryTargetGenerator::WriteNvidiaDeviceLibraryRules(
  const std::string& linkRuleVar, bool relink,
  std::vector<std::string>& commands, const std::string& targetOutput)
{
  std::string linkLanguage = "CUDA";

  // Build list of dependencies.
  std::vector<std::string> depends;
  this->AppendLinkDepends(depends, linkLanguage);

  // Add language-specific flags.
  std::string langFlags;
  this->LocalGenerator->AddLanguageFlagsForLinking(
    langFlags, this->GeneratorTarget, linkLanguage, this->GetConfigName());

  // Clean files associated with this library.
  std::set<std::string> libCleanFiles;
  libCleanFiles.insert(
    this->LocalGenerator->MaybeRelativeToCurBinDir(targetOutput));

  // Determine whether a link script will be used.
  bool useLinkScript = this->GlobalGenerator->GetUseLinkScript();

  bool useResponseFileForObjects =
    this->CheckUseResponseFileForObjects(linkLanguage);
  bool const useResponseFileForLibs =
    this->CheckUseResponseFileForLibraries(linkLanguage);

  cmRulePlaceholderExpander::RuleVariables vars;
  vars.Language = linkLanguage.c_str();

  // Expand the rule variables.
  cmList real_link_commands;
  {
    // Set path conversion for link script shells.
    this->LocalGenerator->SetLinkScriptShell(useLinkScript);

    // Collect up flags to link in needed libraries.
    std::string linkLibs;
    std::unique_ptr<cmLinkLineDeviceComputer> linkLineComputer(
      new cmLinkLineDeviceComputer(
        this->LocalGenerator,
        this->LocalGenerator->GetStateSnapshot().GetDirectory()));
    linkLineComputer->SetForResponse(useResponseFileForLibs);
    linkLineComputer->SetRelink(relink);

    // Create set of linking flags.
    std::string linkFlags;
    std::string ignored_;
    this->LocalGenerator->GetDeviceLinkFlags(
      *linkLineComputer, this->GetConfigName(), ignored_, linkFlags, ignored_,
      ignored_, this->GeneratorTarget);

    this->CreateLinkLibs(
      linkLineComputer.get(), linkLibs, useResponseFileForLibs, depends,
      linkLanguage, cmMakefileTargetGenerator::ResponseFlagFor::DeviceLink);

    // Construct object file lists that may be needed to expand the
    // rule.
    std::string buildObjs;
    this->CreateObjectLists(
      useLinkScript, false, // useArchiveRules
      useResponseFileForObjects, buildObjs, depends, false, linkLanguage,
      cmMakefileTargetGenerator::ResponseFlagFor::DeviceLink);

    std::string objectDir = this->GeneratorTarget->GetSupportDirectory();
    objectDir = this->LocalGenerator->ConvertToOutputFormat(
      this->LocalGenerator->MaybeRelativeToCurBinDir(objectDir),
      cmOutputConverter::SHELL);

    std::string target = this->LocalGenerator->ConvertToOutputFormat(
      this->LocalGenerator->MaybeRelativeToCurBinDir(targetOutput),
      cmOutputConverter::SHELL);

    std::string targetFullPathCompilePDB =
      this->ComputeTargetCompilePDB(this->GetConfigName());
    std::string targetOutPathCompilePDB =
      this->LocalGenerator->ConvertToOutputFormat(targetFullPathCompilePDB,
                                                  cmOutputConverter::SHELL);

    vars.Objects = buildObjs.c_str();
    vars.ObjectDir = objectDir.c_str();
    vars.Target = target.c_str();
    vars.LinkLibraries = linkLibs.c_str();
    vars.ObjectsQuoted = buildObjs.c_str();
    vars.LanguageCompileFlags = langFlags.c_str();
    vars.LinkFlags = linkFlags.c_str();
    vars.TargetCompilePDB = targetOutPathCompilePDB.c_str();

    std::string launcher;
    std::string val = this->LocalGenerator->GetRuleLauncher(
      this->GeneratorTarget, "RULE_LAUNCH_LINK",
      this->Makefile->GetSafeDefinition("CMAKE_BUILD_TYPE"));
    if (cmNonempty(val)) {
      launcher = cmStrCat(val, ' ');
    }

    auto rulePlaceholderExpander =
      this->LocalGenerator->CreateRulePlaceholderExpander();

    // Construct the main link rule and expand placeholders.
    rulePlaceholderExpander->SetTargetImpLib(targetOutput);
    std::string linkRule = this->GetLinkRule(linkRuleVar);
    real_link_commands.append(linkRule);

    // Expand placeholders.
    for (auto& real_link_command : real_link_commands) {
      real_link_command = cmStrCat(launcher, real_link_command);
      rulePlaceholderExpander->ExpandRuleVariables(this->LocalGenerator,
                                                   real_link_command, vars);
    }
    // Restore path conversion to normal shells.
    this->LocalGenerator->SetLinkScriptShell(false);

    // Clean all the possible library names and symlinks.
    this->CleanFiles.insert(libCleanFiles.begin(), libCleanFiles.end());
  }

  std::vector<std::string> commands1;
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

  // Compute the list of outputs.
  std::vector<std::string> outputs(1, targetOutput);

  // Write the build rule.
  this->WriteMakeRule(*this->BuildFileStream, nullptr, outputs, depends,
                      commands, false);
#else
  static_cast<void>(linkRuleVar);
  static_cast<void>(relink);
#endif
}

void cmMakefileLibraryTargetGenerator::WriteLibraryRules(
  const std::string& linkRuleVar, const std::string& extraFlags, bool relink)
{
  // TODO: Merge the methods that call this method to avoid
  // code duplication.
  std::vector<std::string> commands;

  // Get the language to use for linking this library.
  std::string linkLanguage =
    this->GeneratorTarget->GetLinkerLanguage(this->GetConfigName());

  // Make sure we have a link language.
  if (linkLanguage.empty()) {
    cmSystemTools::Error("Cannot determine link language for target \"" +
                         this->GeneratorTarget->GetName() + "\".");
    return;
  }

  auto linker = this->GeneratorTarget->GetLinkerTool(this->GetConfigName());

  // Build list of dependencies.
  std::vector<std::string> depends;
  this->AppendLinkDepends(depends, linkLanguage);
  if (!this->DeviceLinkObject.empty()) {
    depends.push_back(this->DeviceLinkObject);
  }

  // Create set of linking flags.
  std::string linkFlags;
  this->LocalGenerator->AppendFlags(linkFlags, extraFlags);
  this->LocalGenerator->AppendIPOLinkerFlags(
    linkFlags, this->GeneratorTarget, this->GetConfigName(), linkLanguage);

  // Add OSX version flags, if any.
  if (this->GeneratorTarget->GetType() == cmStateEnums::SHARED_LIBRARY ||
      this->GeneratorTarget->GetType() == cmStateEnums::MODULE_LIBRARY) {
    this->AppendOSXVerFlag(linkFlags, linkLanguage, "COMPATIBILITY", true);
    this->AppendOSXVerFlag(linkFlags, linkLanguage, "CURRENT", false);
  }

  // Construct the name of the library.
  this->GeneratorTarget->GetLibraryNames(this->GetConfigName());

  // Construct the full path version of the names.
  std::string outpath;
  std::string outpathImp;
  if (this->GeneratorTarget->IsFrameworkOnApple()) {
    outpath = this->GeneratorTarget->GetDirectory(this->GetConfigName());
    cmOSXBundleGenerator::SkipParts bundleSkipParts;
    if (this->GeneratorTarget->HasImportLibrary(this->GetConfigName())) {
      bundleSkipParts.TextStubs = false;
    }
    this->OSXBundleGenerator->CreateFramework(this->TargetNames.Output,
                                              outpath, this->GetConfigName(),
                                              bundleSkipParts);
    outpath += '/';
    if (!this->TargetNames.ImportLibrary.empty()) {
      outpathImp = this->GeneratorTarget->GetDirectory(
        this->GetConfigName(), cmStateEnums::ImportLibraryArtifact);
      cmSystemTools::MakeDirectory(outpathImp);
      outpathImp += '/';
    }
  } else if (this->GeneratorTarget->IsCFBundleOnApple()) {
    outpath = this->GeneratorTarget->GetDirectory(this->GetConfigName());
    this->OSXBundleGenerator->CreateCFBundle(this->TargetNames.Output, outpath,
                                             this->GetConfigName());
    outpath += '/';
  } else if (relink) {
    outpath = cmStrCat(this->Makefile->GetCurrentBinaryDirectory(),
                       "/CMakeFiles/CMakeRelink.dir");
    cmSystemTools::MakeDirectory(outpath);
    outpath += '/';
    if (!this->TargetNames.ImportLibrary.empty()) {
      outpathImp = outpath;
    }
  } else {
    outpath = this->GeneratorTarget->GetDirectory(this->GetConfigName());
    cmSystemTools::MakeDirectory(outpath);
    outpath += '/';
    if (!this->TargetNames.ImportLibrary.empty()) {
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
  pdbOutputPath += "/";

  std::string targetFullPath = outpath + this->TargetNames.Output;
  std::string targetFullPathPDB = pdbOutputPath + this->TargetNames.PDB;
  std::string targetFullPathSO = outpath + this->TargetNames.SharedObject;
  std::string targetFullPathReal = outpath + this->TargetNames.Real;
  std::string targetFullPathImport =
    outpathImp + this->TargetNames.ImportLibrary;

  // Construct the output path version of the names for use in command
  // arguments.
  std::string targetOutPathPDB = this->LocalGenerator->ConvertToOutputFormat(
    targetFullPathPDB, cmOutputConverter::SHELL);

  std::string targetOutPath = this->LocalGenerator->ConvertToOutputFormat(
    this->LocalGenerator->MaybeRelativeToCurBinDir(targetFullPath),
    cmOutputConverter::SHELL);
  std::string targetOutPathSO = this->LocalGenerator->ConvertToOutputFormat(
    this->LocalGenerator->MaybeRelativeToCurBinDir(targetFullPathSO),
    cmOutputConverter::SHELL);
  std::string targetOutPathReal = this->LocalGenerator->ConvertToOutputFormat(
    this->LocalGenerator->MaybeRelativeToCurBinDir(targetFullPathReal),
    cmOutputConverter::SHELL);
  std::string targetOutPathImport =
    this->LocalGenerator->ConvertToOutputFormat(
      this->LocalGenerator->MaybeRelativeToCurBinDir(targetFullPathImport),
      cmOutputConverter::SHELL);

  this->NumberOfProgressActions++;
  if (!this->NoRuleMessages) {
    cmLocalUnixMakefileGenerator3::EchoProgress progress;
    this->MakeEchoProgress(progress);
    // Add the link message.
    std::string buildEcho = cmStrCat("Linking ", linkLanguage);
    switch (this->GeneratorTarget->GetType()) {
      case cmStateEnums::STATIC_LIBRARY:
        buildEcho += " static library ";
        break;
      case cmStateEnums::SHARED_LIBRARY:
        buildEcho += " shared library ";
        break;
      case cmStateEnums::MODULE_LIBRARY:
        if (this->GeneratorTarget->IsCFBundleOnApple()) {
          buildEcho += " CFBundle";
        }
        buildEcho += " shared module ";
        break;
      default:
        buildEcho += " library ";
        break;
    }
    buildEcho += targetOutPath;
    this->LocalGenerator->AppendEcho(
      commands, buildEcho, cmLocalUnixMakefileGenerator3::EchoLink, &progress);
  }

  // Clean files associated with this library.
  std::set<std::string> libCleanFiles;
  libCleanFiles.insert(
    this->LocalGenerator->MaybeRelativeToCurBinDir(targetFullPathReal));

  std::vector<std::string> commands1;
  // Add a command to remove any existing files for this library.
  // for static libs only
  if (this->GeneratorTarget->GetType() == cmStateEnums::STATIC_LIBRARY) {
    this->LocalGenerator->AppendCleanCommand(commands1, libCleanFiles,
                                             this->GeneratorTarget, "target");
    this->LocalGenerator->CreateCDCommand(
      commands1, this->Makefile->GetCurrentBinaryDirectory(),
      this->LocalGenerator->GetBinaryDirectory());
    cm::append(commands, commands1);
    commands1.clear();
  }

  if (this->TargetNames.Output != this->TargetNames.Real) {
    libCleanFiles.insert(
      this->LocalGenerator->MaybeRelativeToCurBinDir(targetFullPath));
  }
  if (this->TargetNames.SharedObject != this->TargetNames.Real &&
      this->TargetNames.SharedObject != this->TargetNames.Output) {
    libCleanFiles.insert(
      this->LocalGenerator->MaybeRelativeToCurBinDir(targetFullPathSO));
  }
  if (!this->TargetNames.ImportLibrary.empty()) {
    libCleanFiles.insert(
      this->LocalGenerator->MaybeRelativeToCurBinDir(targetFullPathImport));
    std::string implib;
    if (this->GeneratorTarget->GetImplibGNUtoMS(
          this->GetConfigName(), targetFullPathImport, implib)) {
      libCleanFiles.insert(
        this->LocalGenerator->MaybeRelativeToCurBinDir(implib));
    }
  }

  // List the PDB for cleaning only when the whole target is
  // cleaned.  We do not want to delete the .pdb file just before
  // linking the target.
  this->CleanFiles.insert(
    this->LocalGenerator->MaybeRelativeToCurBinDir(targetFullPathPDB));

#ifdef _WIN32
  // There may be a manifest file for this target.  Add it to the
  // clean set just in case.
  if (this->GeneratorTarget->GetType() != cmStateEnums::STATIC_LIBRARY) {
    libCleanFiles.insert(this->LocalGenerator->MaybeRelativeToCurBinDir(
      targetFullPath + ".manifest"));
  }
#endif

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

  bool useResponseFileForObjects =
    this->CheckUseResponseFileForObjects(linkLanguage);
  bool const useResponseFileForLibs =
    this->CheckUseResponseFileForLibraries(linkLanguage);

  // For static libraries there might be archiving rules.
  bool haveStaticLibraryRule = false;
  cmList archiveCreateCommands;
  cmList archiveAppendCommands;
  cmList archiveFinishCommands;
  std::string::size_type archiveCommandLimit = std::string::npos;
  if (this->GeneratorTarget->GetType() == cmStateEnums::STATIC_LIBRARY) {
    haveStaticLibraryRule = this->Makefile->IsDefinitionSet(linkRuleVar);
    std::string arCreateVar =
      cmStrCat("CMAKE_", linkLanguage, "_ARCHIVE_CREATE");

    arCreateVar = this->GeneratorTarget->GetFeatureSpecificLinkRuleVariable(
      arCreateVar, linkLanguage, this->GetConfigName());

    archiveCreateCommands.assign(this->Makefile->GetDefinition(arCreateVar));

    std::string arAppendVar =
      cmStrCat("CMAKE_", linkLanguage, "_ARCHIVE_APPEND");

    arAppendVar = this->GeneratorTarget->GetFeatureSpecificLinkRuleVariable(
      arAppendVar, linkLanguage, this->GetConfigName());

    archiveAppendCommands.assign(this->Makefile->GetDefinition(arAppendVar));

    std::string arFinishVar =
      cmStrCat("CMAKE_", linkLanguage, "_ARCHIVE_FINISH");

    arFinishVar = this->GeneratorTarget->GetFeatureSpecificLinkRuleVariable(
      arFinishVar, linkLanguage, this->GetConfigName());

    archiveFinishCommands.assign(this->Makefile->GetDefinition(arFinishVar));
  }

  // Decide whether to use archiving rules.
  bool useArchiveRules = !haveStaticLibraryRule &&
    !archiveCreateCommands.empty() && !archiveAppendCommands.empty();
  if (useArchiveRules) {
    // Archiving rules are always run with a link script.
    useLinkScript = true;

    // Archiving rules never use a response file.
    useResponseFileForObjects = false;

    // Limit the length of individual object lists to less than half of
    // the command line length limit (leaving half for other flags).
    // This may result in several calls to the archiver.
    if (size_t limit = cmSystemTools::CalculateCommandLineLengthLimit()) {
      archiveCommandLimit = limit / 2;
    } else {
      archiveCommandLimit = 8000;
    }
  }

  // Expand the rule variables.
  auto rulePlaceholderExpander =
    this->LocalGenerator->CreateRulePlaceholderExpander();
  bool useWatcomQuote =
    this->Makefile->IsOn(linkRuleVar + "_USE_WATCOM_QUOTE");
  cmList real_link_commands;
  {
    // Set path conversion for link script shells.
    this->LocalGenerator->SetLinkScriptShell(useLinkScript);

    // Collect up flags to link in needed libraries.
    std::string linkLibs;
    if (this->GeneratorTarget->GetType() != cmStateEnums::STATIC_LIBRARY) {

      std::unique_ptr<cmLinkLineComputer> linkLineComputer =
        this->CreateLinkLineComputer(
          this->LocalGenerator,
          this->LocalGenerator->GetStateSnapshot().GetDirectory());
      linkLineComputer->SetForResponse(useResponseFileForLibs);
      linkLineComputer->SetUseWatcomQuote(useWatcomQuote);
      linkLineComputer->SetRelink(relink);

      this->CreateLinkLibs(linkLineComputer.get(), linkLibs,
                           useResponseFileForLibs, depends, linkLanguage);
    }

    // Construct object file lists that may be needed to expand the
    // rule.
    std::string buildObjs;
    this->CreateObjectLists(useLinkScript, useArchiveRules,
                            useResponseFileForObjects, buildObjs, depends,
                            useWatcomQuote, linkLanguage);
    if (!this->DeviceLinkObject.empty()) {
      buildObjs += " " +
        this->LocalGenerator->ConvertToOutputFormat(
          this->LocalGenerator->MaybeRelativeToCurBinDir(
            this->DeviceLinkObject),
          cmOutputConverter::SHELL);
    }

    std::string const& aixExports = this->GetAIXExports(this->GetConfigName());

    // maybe create .def file from list of objects
    this->GenDefFile(real_link_commands);

    std::string manifests = this->GetManifests(this->GetConfigName());

    cmRulePlaceholderExpander::RuleVariables vars;
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

    vars.CMTargetName = this->GeneratorTarget->GetName().c_str();
    vars.CMTargetType =
      cmState::GetTargetTypeName(this->GeneratorTarget->GetType()).c_str();
    vars.Language = linkLanguage.c_str();
    vars.Linker = linker.c_str();
    vars.AIXExports = aixExports.c_str();
    vars.Objects = buildObjs.c_str();
    std::string objectDir = this->GeneratorTarget->GetSupportDirectory();

    objectDir = this->LocalGenerator->ConvertToOutputFormat(
      this->LocalGenerator->MaybeRelativeToCurBinDir(objectDir),
      cmOutputConverter::SHELL);

    vars.ObjectDir = objectDir.c_str();
    std::string target = this->LocalGenerator->ConvertToOutputFormat(
      this->LocalGenerator->MaybeRelativeToCurBinDir(targetFullPathReal),
      cmOutputConverter::SHELL, useWatcomQuote);
    vars.Target = target.c_str();
    vars.LinkLibraries = linkLibs.c_str();
    vars.ObjectsQuoted = buildObjs.c_str();
    std::string targetOutSOName;
    if (this->GeneratorTarget->HasSOName(this->GetConfigName())) {
      vars.SONameFlag = this->Makefile->GetSONameFlag(linkLanguage);
      targetOutSOName = this->LocalGenerator->ConvertToOutputFormat(
        this->TargetNames.SharedObject, cmOutputConverter::SHELL);
      vars.TargetSOName = targetOutSOName.c_str();
    }
    vars.LinkFlags = linkFlags.c_str();

    vars.Manifests = manifests.c_str();

    // Compute the directory portion of the install_name setting.
    std::string install_name_dir;
    if (this->GeneratorTarget->GetType() == cmStateEnums::SHARED_LIBRARY) {
      // Get the install_name directory for the build tree.
      install_name_dir = this->GeneratorTarget->GetInstallNameDirForBuildTree(
        this->GetConfigName());

      // Set the rule variable replacement value.
      if (install_name_dir.empty()) {
        vars.TargetInstallNameDir = "";
      } else {
        // Convert to a path for the native build tool.
        install_name_dir = this->LocalGenerator->ConvertToOutputFormat(
          install_name_dir, cmOutputConverter::SHELL);
        vars.TargetInstallNameDir = install_name_dir.c_str();
      }
    }

    // Add language-specific flags.
    std::string langFlags;
    this->LocalGenerator->AddLanguageFlagsForLinking(
      langFlags, this->GeneratorTarget, linkLanguage, this->GetConfigName());

    this->LocalGenerator->AddArchitectureFlags(
      langFlags, this->GeneratorTarget, linkLanguage, this->GetConfigName());

    vars.LanguageCompileFlags = langFlags.c_str();

    std::string linkerLauncher =
      this->GetLinkerLauncher(this->GetConfigName());
    if (cmNonempty(linkerLauncher)) {
      vars.Launcher = linkerLauncher.c_str();
    }

    std::string launcher;
    std::string val = this->LocalGenerator->GetRuleLauncher(
      this->GeneratorTarget, "RULE_LAUNCH_LINK",
      this->Makefile->GetSafeDefinition("CMAKE_BUILD_TYPE"));
    if (cmNonempty(val)) {
      launcher = cmStrCat(val, ' ');
    }

    // Construct the main link rule and expand placeholders.
    rulePlaceholderExpander->SetTargetImpLib(targetOutPathImport);
    if (useArchiveRules) {
      // Construct the individual object list strings.
      std::vector<std::string> object_strings;
      this->WriteObjectsStrings(object_strings, false, archiveCommandLimit);

      // Add the cuda device object to the list of archive files. This will
      // only occur on archives which have CUDA_RESOLVE_DEVICE_SYMBOLS enabled
      if (!this->DeviceLinkObject.empty()) {
        object_strings.push_back(this->LocalGenerator->ConvertToOutputFormat(
          this->LocalGenerator->MaybeRelativeToCurBinDir(
            this->DeviceLinkObject),
          cmOutputConverter::SHELL));
      }

      // Create the archive with the first set of objects.
      auto osi = object_strings.begin();
      {
        vars.Objects = osi->c_str();
        for (std::string const& acc : archiveCreateCommands) {
          std::string cmd = launcher + acc;
          rulePlaceholderExpander->ExpandRuleVariables(this->LocalGenerator,
                                                       cmd, vars);
          real_link_commands.push_back(std::move(cmd));
        }
      }
      // Append to the archive with the other object sets.
      for (++osi; osi != object_strings.end(); ++osi) {
        vars.Objects = osi->c_str();
        for (std::string const& aac : archiveAppendCommands) {
          std::string cmd = launcher + aac;
          rulePlaceholderExpander->ExpandRuleVariables(this->LocalGenerator,
                                                       cmd, vars);
          real_link_commands.push_back(std::move(cmd));
        }
      }
      // Finish the archive.
      vars.Objects = "";
      for (std::string const& afc : archiveFinishCommands) {
        std::string cmd = launcher + afc;
        rulePlaceholderExpander->ExpandRuleVariables(this->LocalGenerator, cmd,
                                                     vars);
        // If there is no ranlib the command will be ":".  Skip it.
        if (!cmd.empty() && cmd[0] != ':') {
          real_link_commands.push_back(std::move(cmd));
        }
      }
    } else {
      // Get the set of commands.
      std::string linkRule = this->GetLinkRule(linkRuleVar);
      real_link_commands.append(linkRule);
      if (this->UseLWYU) {
        cmValue lwyuCheck =
          this->Makefile->GetDefinition("CMAKE_LINK_WHAT_YOU_USE_CHECK");
        if (lwyuCheck) {
          std::string cmakeCommand = cmStrCat(
            this->LocalGenerator->ConvertToOutputFormat(
              cmSystemTools::GetCMakeCommand(), cmLocalGenerator::SHELL),
            " -E __run_co_compile --lwyu=");
          cmakeCommand += this->LocalGenerator->EscapeForShell(*lwyuCheck);
          cmakeCommand += cmStrCat(" --source=", targetOutPathReal);
          real_link_commands.push_back(std::move(cmakeCommand));
        }
      }

      // Expand placeholders.
      for (auto& real_link_command : real_link_commands) {
        real_link_command = cmStrCat(launcher, real_link_command);
        rulePlaceholderExpander->ExpandRuleVariables(this->LocalGenerator,
                                                     real_link_command, vars);
      }
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
  // Frameworks are handled by cmOSXBundleGenerator.
  if (targetOutPath != targetOutPathReal &&
      !this->GeneratorTarget->IsFrameworkOnApple()) {
    std::string symlink =
      cmStrCat("$(CMAKE_COMMAND) -E cmake_symlink_library ", targetOutPathReal,
               ' ', targetOutPathSO, ' ', targetOutPath);
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

  // Compute the list of outputs.
  std::vector<std::string> outputs;
  outputs.reserve(3);
  outputs.push_back(targetFullPathReal);
  if (this->TargetNames.SharedObject != this->TargetNames.Real) {
    outputs.push_back(targetFullPathSO);
  }
  if (this->TargetNames.Output != this->TargetNames.SharedObject &&
      this->TargetNames.Output != this->TargetNames.Real) {
    outputs.push_back(targetFullPath);
  }

  // Write the build rule.
  this->WriteMakeRule(*this->BuildFileStream, nullptr, outputs, depends,
                      commands, false);

  // Add rule to generate text-based stubs, if required
  if (this->GeneratorTarget->IsApple() &&
      this->GeneratorTarget->HasImportLibrary(this->GetConfigName())) {
    auto genStubsRule =
      this->Makefile->GetDefinition("CMAKE_CREATE_TEXT_STUBS");
    cmList genStubs_commands{ genStubsRule };

    std::string TBDFullPath =
      cmStrCat(outpathImp, this->TargetNames.ImportOutput);
    std::string TBDFullPathReal =
      cmStrCat(outpathImp, this->TargetNames.ImportReal);
    std::string TBDFullPathSO =
      cmStrCat(outpathImp, this->TargetNames.ImportLibrary);

    // Expand placeholders.
    cmRulePlaceholderExpander::RuleVariables vars;
    std::string target = this->LocalGenerator->ConvertToOutputFormat(
      this->LocalGenerator->MaybeRelativeToCurBinDir(targetFullPathReal),
      cmOutputConverter::SHELL, useWatcomQuote);
    vars.Target = target.c_str();
    std::string TBDOutPathReal = this->LocalGenerator->ConvertToOutputFormat(
      this->LocalGenerator->MaybeRelativeToCurBinDir(TBDFullPathReal),
      cmOutputConverter::SHELL, useWatcomQuote);
    rulePlaceholderExpander->SetTargetImpLib(TBDOutPathReal);
    for (std::string& command : genStubs_commands) {
      rulePlaceholderExpander->ExpandRuleVariables(this->LocalGenerator,
                                                   command, vars);
    }
    outputs.clear();
    outputs.push_back(TBDFullPathReal);
    if (this->TargetNames.ImportLibrary != this->TargetNames.ImportReal) {
      outputs.push_back(TBDFullPathSO);
    }
    if (this->TargetNames.ImportOutput != this->TargetNames.ImportLibrary &&
        this->TargetNames.ImportOutput != this->TargetNames.ImportReal) {
      outputs.push_back(TBDFullPath);
    }
    this->ExtraFiles.insert(TBDFullPath);

    depends.clear();
    depends.push_back(targetFullPathReal);

    // Add a rule to create necessary symlinks for the library.
    // Frameworks are handled by cmOSXBundleGenerator.
    if (TBDFullPath != TBDFullPathReal &&
        !this->GeneratorTarget->IsFrameworkOnApple()) {
      auto TBDOutPathSO = this->LocalGenerator->ConvertToOutputFormat(
        this->LocalGenerator->MaybeRelativeToCurBinDir(TBDFullPathSO),
        cmOutputConverter::SHELL, useWatcomQuote);
      auto TBDOutPath = this->LocalGenerator->ConvertToOutputFormat(
        this->LocalGenerator->MaybeRelativeToCurBinDir(TBDFullPath),
        cmOutputConverter::SHELL, useWatcomQuote);

      std::string symlink =
        cmStrCat("$(CMAKE_COMMAND) -E cmake_symlink_library ", TBDOutPathReal,
                 ' ', TBDOutPathSO, ' ', TBDOutPath);
      commands1.push_back(std::move(symlink));
      this->LocalGenerator->CreateCDCommand(
        commands1, this->Makefile->GetCurrentBinaryDirectory(),
        this->LocalGenerator->GetBinaryDirectory());
      cm::append(genStubs_commands, commands1);
      commands1.clear();
    }

    this->WriteMakeRule(*this->BuildFileStream, nullptr, outputs, depends,
                        genStubs_commands, false);

    // clean actions for apple specific outputs
    // clean actions for ImportLibrary are already specified
    if (this->TargetNames.ImportReal != this->TargetNames.ImportLibrary) {
      libCleanFiles.insert(
        this->LocalGenerator->MaybeRelativeToCurBinDir(TBDFullPathReal));
    }
    if (this->TargetNames.ImportOutput != this->TargetNames.ImportReal &&
        this->TargetNames.ImportOutput != this->TargetNames.ImportLibrary) {
      libCleanFiles.insert(
        this->LocalGenerator->MaybeRelativeToCurBinDir(TBDFullPath));
    }
  }

  // Write the main driver rule to build everything in this target.
  this->WriteTargetDriverRule(targetFullPath, relink);

  // Clean all the possible library names and symlinks.
  this->CleanFiles.insert(libCleanFiles.begin(), libCleanFiles.end());
}
