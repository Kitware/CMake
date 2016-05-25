/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmMakefileLibraryTargetGenerator.h"

#include "cmAlgorithms.h"
#include "cmGeneratedFileStream.h"
#include "cmGlobalUnixMakefileGenerator3.h"
#include "cmLocalUnixMakefileGenerator3.h"
#include "cmMakefile.h"
#include "cmSourceFile.h"
#include "cmake.h"

cmMakefileLibraryTargetGenerator::cmMakefileLibraryTargetGenerator(
  cmGeneratorTarget* target)
  : cmMakefileTargetGenerator(target)
{
  this->CustomCommandDriver = OnDepends;
  if (this->GeneratorTarget->GetType() != cmState::INTERFACE_LIBRARY) {
    this->GeneratorTarget->GetLibraryNames(
      this->TargetNameOut, this->TargetNameSO, this->TargetNameReal,
      this->TargetNameImport, this->TargetNamePDB, this->ConfigName);
  }

  this->OSXBundleGenerator =
    new cmOSXBundleGenerator(target, this->ConfigName);
  this->OSXBundleGenerator->SetMacContentFolders(&this->MacContentFolders);
}

cmMakefileLibraryTargetGenerator::~cmMakefileLibraryTargetGenerator()
{
  delete this->OSXBundleGenerator;
}

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

  // write the link rules
  // Write the rule for this target type.
  switch (this->GeneratorTarget->GetType()) {
    case cmState::STATIC_LIBRARY:
      this->WriteStaticLibraryRules();
      break;
    case cmState::SHARED_LIBRARY:
      this->WriteSharedLibraryRules(false);
      if (this->GeneratorTarget->NeedRelinkBeforeInstall(this->ConfigName)) {
        // Write rules to link an installable version of the target.
        this->WriteSharedLibraryRules(true);
      }
      break;
    case cmState::MODULE_LIBRARY:
      this->WriteModuleLibraryRules(false);
      if (this->GeneratorTarget->NeedRelinkBeforeInstall(this->ConfigName)) {
        // Write rules to link an installable version of the target.
        this->WriteModuleLibraryRules(true);
      }
      break;
    case cmState::OBJECT_LIBRARY:
      this->WriteObjectLibraryRules();
      break;
    default:
      // If language is not known, this is an error.
      cmSystemTools::Error("Unknown Library Type");
      break;
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

void cmMakefileLibraryTargetGenerator::WriteObjectLibraryRules()
{
  std::vector<std::string> commands;
  std::vector<std::string> depends;

  // Add post-build rules.
  this->LocalGenerator->AppendCustomCommands(
    commands, this->GeneratorTarget->GetPostBuildCommands(),
    this->GeneratorTarget);

  // Depend on the object files.
  this->AppendObjectDepends(depends);

  // Write the rule.
  this->LocalGenerator->WriteMakeRule(*this->BuildFileStream, 0,
                                      this->GeneratorTarget->GetName(),
                                      depends, commands, true);

  // Write the main driver rule to build everything in this target.
  this->WriteTargetDriverRule(this->GeneratorTarget->GetName(), false);
}

void cmMakefileLibraryTargetGenerator::WriteStaticLibraryRules()
{
  std::string linkLanguage =
    this->GeneratorTarget->GetLinkerLanguage(this->ConfigName);
  std::string linkRuleVar = "CMAKE_";
  linkRuleVar += linkLanguage;
  linkRuleVar += "_CREATE_STATIC_LIBRARY";

  if (this->GetFeatureAsBool("INTERPROCEDURAL_OPTIMIZATION") &&
      this->Makefile->GetDefinition(linkRuleVar + "_IPO")) {
    linkRuleVar += "_IPO";
  }

  std::string extraFlags;
  this->LocalGenerator->GetStaticLibraryFlags(
    extraFlags, cmSystemTools::UpperCase(this->ConfigName),
    this->GeneratorTarget);
  this->WriteLibraryRules(linkRuleVar, extraFlags, false);
}

void cmMakefileLibraryTargetGenerator::WriteSharedLibraryRules(bool relink)
{
  if (this->GeneratorTarget->IsFrameworkOnApple()) {
    this->WriteFrameworkRules(relink);
    return;
  }
  std::string linkLanguage =
    this->GeneratorTarget->GetLinkerLanguage(this->ConfigName);
  std::string linkRuleVar = "CMAKE_";
  linkRuleVar += linkLanguage;
  linkRuleVar += "_CREATE_SHARED_LIBRARY";

  std::string extraFlags;
  this->LocalGenerator->AppendFlags(
    extraFlags, this->GeneratorTarget->GetProperty("LINK_FLAGS"));
  std::string linkFlagsConfig = "LINK_FLAGS_";
  linkFlagsConfig += cmSystemTools::UpperCase(this->ConfigName);
  this->LocalGenerator->AppendFlags(
    extraFlags, this->GeneratorTarget->GetProperty(linkFlagsConfig));

  this->LocalGenerator->AddConfigVariableFlags(
    extraFlags, "CMAKE_SHARED_LINKER_FLAGS", this->ConfigName);
  this->AddModuleDefinitionFlag(extraFlags);

  this->WriteLibraryRules(linkRuleVar, extraFlags, relink);
}

void cmMakefileLibraryTargetGenerator::WriteModuleLibraryRules(bool relink)
{
  std::string linkLanguage =
    this->GeneratorTarget->GetLinkerLanguage(this->ConfigName);
  std::string linkRuleVar = "CMAKE_";
  linkRuleVar += linkLanguage;
  linkRuleVar += "_CREATE_SHARED_MODULE";

  std::string extraFlags;
  this->LocalGenerator->AppendFlags(
    extraFlags, this->GeneratorTarget->GetProperty("LINK_FLAGS"));
  std::string linkFlagsConfig = "LINK_FLAGS_";
  linkFlagsConfig += cmSystemTools::UpperCase(this->ConfigName);
  this->LocalGenerator->AppendFlags(
    extraFlags, this->GeneratorTarget->GetProperty(linkFlagsConfig));
  this->LocalGenerator->AddConfigVariableFlags(
    extraFlags, "CMAKE_MODULE_LINKER_FLAGS", this->ConfigName);
  this->AddModuleDefinitionFlag(extraFlags);

  this->WriteLibraryRules(linkRuleVar, extraFlags, relink);
}

void cmMakefileLibraryTargetGenerator::WriteFrameworkRules(bool relink)
{
  std::string linkLanguage =
    this->GeneratorTarget->GetLinkerLanguage(this->ConfigName);
  std::string linkRuleVar = "CMAKE_";
  linkRuleVar += linkLanguage;
  linkRuleVar += "_CREATE_MACOSX_FRAMEWORK";

  std::string extraFlags;
  this->LocalGenerator->AppendFlags(
    extraFlags, this->GeneratorTarget->GetProperty("LINK_FLAGS"));
  std::string linkFlagsConfig = "LINK_FLAGS_";
  linkFlagsConfig += cmSystemTools::UpperCase(this->ConfigName);
  this->LocalGenerator->AppendFlags(
    extraFlags, this->GeneratorTarget->GetProperty(linkFlagsConfig));
  this->LocalGenerator->AddConfigVariableFlags(
    extraFlags, "CMAKE_MACOSX_FRAMEWORK_LINKER_FLAGS", this->ConfigName);

  this->WriteLibraryRules(linkRuleVar, extraFlags, relink);
}

void cmMakefileLibraryTargetGenerator::WriteLibraryRules(
  const std::string& linkRuleVar, const std::string& extraFlags, bool relink)
{
  // TODO: Merge the methods that call this method to avoid
  // code duplication.
  std::vector<std::string> commands;

  // Build list of dependencies.
  std::vector<std::string> depends;
  this->AppendLinkDepends(depends);

  // Get the language to use for linking this library.
  std::string linkLanguage =
    this->GeneratorTarget->GetLinkerLanguage(this->ConfigName);

  // Make sure we have a link language.
  if (linkLanguage.empty()) {
    cmSystemTools::Error("Cannot determine link language for target \"",
                         this->GeneratorTarget->GetName().c_str(), "\".");
    return;
  }

  // Create set of linking flags.
  std::string linkFlags;
  this->LocalGenerator->AppendFlags(linkFlags, extraFlags);

  // Add OSX version flags, if any.
  if (this->GeneratorTarget->GetType() == cmState::SHARED_LIBRARY ||
      this->GeneratorTarget->GetType() == cmState::MODULE_LIBRARY) {
    this->AppendOSXVerFlag(linkFlags, linkLanguage, "COMPATIBILITY", true);
    this->AppendOSXVerFlag(linkFlags, linkLanguage, "CURRENT", false);
  }

  // Construct the name of the library.
  std::string targetName;
  std::string targetNameSO;
  std::string targetNameReal;
  std::string targetNameImport;
  std::string targetNamePDB;
  this->GeneratorTarget->GetLibraryNames(targetName, targetNameSO,
                                         targetNameReal, targetNameImport,
                                         targetNamePDB, this->ConfigName);

  // Construct the full path version of the names.
  std::string outpath;
  std::string outpathImp;
  if (this->GeneratorTarget->IsFrameworkOnApple()) {
    outpath = this->GeneratorTarget->GetDirectory(this->ConfigName);
    this->OSXBundleGenerator->CreateFramework(targetName, outpath);
    outpath += "/";
  } else if (this->GeneratorTarget->IsCFBundleOnApple()) {
    outpath = this->GeneratorTarget->GetDirectory(this->ConfigName);
    this->OSXBundleGenerator->CreateCFBundle(targetName, outpath);
    outpath += "/";
  } else if (relink) {
    outpath = this->Makefile->GetCurrentBinaryDirectory();
    outpath += cmake::GetCMakeFilesDirectory();
    outpath += "/CMakeRelink.dir";
    cmSystemTools::MakeDirectory(outpath.c_str());
    outpath += "/";
    if (!targetNameImport.empty()) {
      outpathImp = outpath;
    }
  } else {
    outpath = this->GeneratorTarget->GetDirectory(this->ConfigName);
    cmSystemTools::MakeDirectory(outpath.c_str());
    outpath += "/";
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
  std::string targetFullPathPDB = pdbOutputPath + targetNamePDB;
  std::string targetFullPathSO = outpath + targetNameSO;
  std::string targetFullPathReal = outpath + targetNameReal;
  std::string targetFullPathImport = outpathImp + targetNameImport;

  // Construct the output path version of the names for use in command
  // arguments.
  std::string targetOutPathPDB = this->Convert(
    targetFullPathPDB, cmOutputConverter::NONE, cmOutputConverter::SHELL);
  std::string targetOutPath = this->Convert(
    targetFullPath, cmOutputConverter::START_OUTPUT, cmOutputConverter::SHELL);
  std::string targetOutPathSO =
    this->Convert(targetFullPathSO, cmOutputConverter::START_OUTPUT,
                  cmOutputConverter::SHELL);
  std::string targetOutPathReal =
    this->Convert(targetFullPathReal, cmOutputConverter::START_OUTPUT,
                  cmOutputConverter::SHELL);
  std::string targetOutPathImport =
    this->Convert(targetFullPathImport, cmOutputConverter::START_OUTPUT,
                  cmOutputConverter::SHELL);

  this->NumberOfProgressActions++;
  if (!this->NoRuleMessages) {
    cmLocalUnixMakefileGenerator3::EchoProgress progress;
    this->MakeEchoProgress(progress);
    // Add the link message.
    std::string buildEcho = "Linking ";
    buildEcho += linkLanguage;
    switch (this->GeneratorTarget->GetType()) {
      case cmState::STATIC_LIBRARY:
        buildEcho += " static library ";
        break;
      case cmState::SHARED_LIBRARY:
        buildEcho += " shared library ";
        break;
      case cmState::MODULE_LIBRARY:
        if (this->GeneratorTarget->IsCFBundleOnApple())
          buildEcho += " CFBundle";
        buildEcho += " shared module ";
        break;
      default:
        buildEcho += " library ";
        break;
    }
    buildEcho += targetOutPath.c_str();
    this->LocalGenerator->AppendEcho(commands, buildEcho.c_str(),
                                     cmLocalUnixMakefileGenerator3::EchoLink,
                                     &progress);
  }

  const char* forbiddenFlagVar = 0;
  switch (this->GeneratorTarget->GetType()) {
    case cmState::SHARED_LIBRARY:
      forbiddenFlagVar = "_CREATE_SHARED_LIBRARY_FORBIDDEN_FLAGS";
      break;
    case cmState::MODULE_LIBRARY:
      forbiddenFlagVar = "_CREATE_SHARED_MODULE_FORBIDDEN_FLAGS";
      break;
    default:
      break;
  }

  // Clean files associated with this library.
  std::vector<std::string> libCleanFiles;
  libCleanFiles.push_back(this->Convert(targetFullPath,
                                        cmOutputConverter::START_OUTPUT,
                                        cmOutputConverter::UNCHANGED));
  if (targetNameReal != targetName) {
    libCleanFiles.push_back(this->Convert(targetFullPathReal,
                                          cmOutputConverter::START_OUTPUT,
                                          cmOutputConverter::UNCHANGED));
  }
  if (targetNameSO != targetName && targetNameSO != targetNameReal) {
    libCleanFiles.push_back(this->Convert(targetFullPathSO,
                                          cmOutputConverter::START_OUTPUT,
                                          cmOutputConverter::UNCHANGED));
  }
  if (!targetNameImport.empty()) {
    libCleanFiles.push_back(this->Convert(targetFullPathImport,
                                          cmOutputConverter::START_OUTPUT,
                                          cmOutputConverter::UNCHANGED));
    std::string implib;
    if (this->GeneratorTarget->GetImplibGNUtoMS(targetFullPathImport,
                                                implib)) {
      libCleanFiles.push_back(this->Convert(implib,
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

#ifdef _WIN32
  // There may be a manifest file for this target.  Add it to the
  // clean set just in case.
  if (this->GeneratorTarget->GetType() != cmState::STATIC_LIBRARY) {
    libCleanFiles.push_back(this->Convert(
      (targetFullPath + ".manifest").c_str(), cmOutputConverter::START_OUTPUT,
      cmOutputConverter::UNCHANGED));
  }
#endif

  std::vector<std::string> commands1;
  // Add a command to remove any existing files for this library.
  // for static libs only
  if (this->GeneratorTarget->GetType() == cmState::STATIC_LIBRARY) {
    this->LocalGenerator->AppendCleanCommand(commands1, libCleanFiles,
                                             this->GeneratorTarget, "target");
    this->LocalGenerator->CreateCDCommand(
      commands1, this->Makefile->GetCurrentBinaryDirectory(),
      cmOutputConverter::HOME_OUTPUT);
    commands.insert(commands.end(), commands1.begin(), commands1.end());
    commands1.clear();
  }

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

  // For static libraries there might be archiving rules.
  bool haveStaticLibraryRule = false;
  std::vector<std::string> archiveCreateCommands;
  std::vector<std::string> archiveAppendCommands;
  std::vector<std::string> archiveFinishCommands;
  std::string::size_type archiveCommandLimit = std::string::npos;
  if (this->GeneratorTarget->GetType() == cmState::STATIC_LIBRARY) {
    haveStaticLibraryRule =
      this->Makefile->GetDefinition(linkRuleVar) ? true : false;
    std::string arCreateVar = "CMAKE_";
    arCreateVar += linkLanguage;
    arCreateVar += "_ARCHIVE_CREATE";
    if (const char* rule = this->Makefile->GetDefinition(arCreateVar)) {
      cmSystemTools::ExpandListArgument(rule, archiveCreateCommands);
    }
    std::string arAppendVar = "CMAKE_";
    arAppendVar += linkLanguage;
    arAppendVar += "_ARCHIVE_APPEND";
    if (const char* rule = this->Makefile->GetDefinition(arAppendVar)) {
      cmSystemTools::ExpandListArgument(rule, archiveAppendCommands);
    }
    std::string arFinishVar = "CMAKE_";
    arFinishVar += linkLanguage;
    arFinishVar += "_ARCHIVE_FINISH";
    if (const char* rule = this->Makefile->GetDefinition(arFinishVar)) {
      cmSystemTools::ExpandListArgument(rule, archiveFinishCommands);
    }
  }

  // Decide whether to use archiving rules.
  bool useArchiveRules = !haveStaticLibraryRule &&
    !archiveCreateCommands.empty() && !archiveAppendCommands.empty();
  if (useArchiveRules) {
    // Archiving rules are always run with a link script.
    useLinkScript = true;

    // Archiving rules never use a response file.
    useResponseFileForObjects = false;

    // Limit the length of individual object lists to less than the
    // 32K command line length limit on Windows.  We could make this a
    // platform file variable but this should work everywhere.
    archiveCommandLimit = 30000;
  }

  // Expand the rule variables.
  std::vector<std::string> real_link_commands;
  {
    bool useWatcomQuote =
      this->Makefile->IsOn(linkRuleVar + "_USE_WATCOM_QUOTE");

    // Set path conversion for link script shells.
    this->LocalGenerator->SetLinkScriptShell(useLinkScript);

    // Collect up flags to link in needed libraries.
    std::string linkLibs;
    if (this->GeneratorTarget->GetType() != cmState::STATIC_LIBRARY) {
      this->CreateLinkLibs(linkLibs, relink, useResponseFileForLibs, depends,
                           useWatcomQuote);
    }

    // Construct object file lists that may be needed to expand the
    // rule.
    std::string buildObjs;
    this->CreateObjectLists(useLinkScript, useArchiveRules,
                            useResponseFileForObjects, buildObjs, depends,
                            useWatcomQuote);

    // maybe create .def file from list of objects
    if (this->GeneratorTarget->GetType() == cmState::SHARED_LIBRARY &&
        this->Makefile->IsOn("CMAKE_SUPPORT_WINDOWS_EXPORT_ALL_SYMBOLS")) {
      if (this->GeneratorTarget->GetPropertyAsBool(
            "WINDOWS_EXPORT_ALL_SYMBOLS")) {
        std::string name_of_def_file =
          this->GeneratorTarget->GetSupportDirectory();
        name_of_def_file +=
          std::string("/") + this->GeneratorTarget->GetName();
        name_of_def_file += ".def";
        std::string cmd = cmSystemTools::GetCMakeCommand();
        cmd = this->Convert(cmd, cmOutputConverter::NONE,
                            cmOutputConverter::SHELL);
        cmd += " -E __create_def ";
        cmd += this->Convert(name_of_def_file, cmOutputConverter::START_OUTPUT,
                             cmOutputConverter::SHELL);
        cmd += " ";
        std::string objlist_file = name_of_def_file;
        objlist_file += ".objs";
        cmd += this->Convert(objlist_file, cmOutputConverter::START_OUTPUT,
                             cmOutputConverter::SHELL);
        real_link_commands.push_back(cmd);
        // create a list of obj files for the -E __create_def to read
        cmGeneratedFileStream fout(objlist_file.c_str());
        for (std::vector<std::string>::const_iterator i =
               this->Objects.begin();
             i != this->Objects.end(); ++i) {
          if (cmHasLiteralSuffix(*i, ".obj")) {
            fout << *i << "\n";
          }
        }
        for (std::vector<std::string>::const_iterator i =
               this->ExternalObjects.begin();
             i != this->ExternalObjects.end(); ++i) {
          fout << *i << "\n";
        }
        // now add the def file link flag
        linkFlags += " ";
        linkFlags +=
          this->Makefile->GetSafeDefinition("CMAKE_LINK_DEF_FILE_FLAG");
        linkFlags +=
          this->Convert(name_of_def_file, cmOutputConverter::START_OUTPUT,
                        cmOutputConverter::SHELL);
        linkFlags += " ";
      }
    }

    std::string manifests = this->GetManifests();

    cmLocalGenerator::RuleVariables vars;
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
    vars.LinkLibraries = linkLibs.c_str();
    vars.ObjectsQuoted = buildObjs.c_str();
    if (this->GeneratorTarget->HasSOName(this->ConfigName)) {
      vars.SONameFlag = this->Makefile->GetSONameFlag(linkLanguage);
      vars.TargetSOName = targetNameSO.c_str();
    }
    vars.LinkFlags = linkFlags.c_str();

    vars.Manifests = manifests.c_str();

    // Compute the directory portion of the install_name setting.
    std::string install_name_dir;
    if (this->GeneratorTarget->GetType() == cmState::SHARED_LIBRARY) {
      // Get the install_name directory for the build tree.
      install_name_dir =
        this->GeneratorTarget->GetInstallNameDirForBuildTree(this->ConfigName);

      // Set the rule variable replacement value.
      if (install_name_dir.empty()) {
        vars.TargetInstallNameDir = "";
      } else {
        // Convert to a path for the native build tool.
        install_name_dir = this->LocalGenerator->Convert(
          install_name_dir, cmOutputConverter::NONE, cmOutputConverter::SHELL);
        vars.TargetInstallNameDir = install_name_dir.c_str();
      }
    }

    // Add language feature flags.
    std::string langFlags;
    this->AddFeatureFlags(langFlags, linkLanguage);

    this->LocalGenerator->AddArchitectureFlags(
      langFlags, this->GeneratorTarget, linkLanguage, this->ConfigName);

    // remove any language flags that might not work with the
    // particular os
    if (forbiddenFlagVar) {
      this->RemoveForbiddenFlags(forbiddenFlagVar, linkLanguage, langFlags);
    }
    vars.LanguageCompileFlags = langFlags.c_str();

    // Construct the main link rule and expand placeholders.
    this->LocalGenerator->TargetImplib = targetOutPathImport;
    if (useArchiveRules) {
      // Construct the individual object list strings.
      std::vector<std::string> object_strings;
      this->WriteObjectsStrings(object_strings, archiveCommandLimit);

      // Create the archive with the first set of objects.
      std::vector<std::string>::iterator osi = object_strings.begin();
      {
        vars.Objects = osi->c_str();
        for (std::vector<std::string>::const_iterator i =
               archiveCreateCommands.begin();
             i != archiveCreateCommands.end(); ++i) {
          std::string cmd = *i;
          this->LocalGenerator->ExpandRuleVariables(cmd, vars);
          real_link_commands.push_back(cmd);
        }
      }
      // Append to the archive with the other object sets.
      for (++osi; osi != object_strings.end(); ++osi) {
        vars.Objects = osi->c_str();
        for (std::vector<std::string>::const_iterator i =
               archiveAppendCommands.begin();
             i != archiveAppendCommands.end(); ++i) {
          std::string cmd = *i;
          this->LocalGenerator->ExpandRuleVariables(cmd, vars);
          real_link_commands.push_back(cmd);
        }
      }
      // Finish the archive.
      vars.Objects = "";
      for (std::vector<std::string>::const_iterator i =
             archiveFinishCommands.begin();
           i != archiveFinishCommands.end(); ++i) {
        std::string cmd = *i;
        this->LocalGenerator->ExpandRuleVariables(cmd, vars);
        real_link_commands.push_back(cmd);
      }
    } else {
      // Get the set of commands.
      std::string linkRule = this->GetLinkRule(linkRuleVar);
      cmSystemTools::ExpandListArgument(linkRule, real_link_commands);

      // Expand placeholders.
      for (std::vector<std::string>::iterator i = real_link_commands.begin();
           i != real_link_commands.end(); ++i) {
        this->LocalGenerator->ExpandRuleVariables(*i, vars);
      }
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
  // Frameworks are handled by cmOSXBundleGenerator.
  if (targetOutPath != targetOutPathReal &&
      !this->GeneratorTarget->IsFrameworkOnApple()) {
    std::string symlink = "$(CMAKE_COMMAND) -E cmake_symlink_library ";
    symlink += targetOutPathReal;
    symlink += " ";
    symlink += targetOutPathSO;
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

  // Compute the list of outputs.
  std::vector<std::string> outputs(1, targetFullPathReal);
  if (targetNameSO != targetNameReal) {
    outputs.push_back(targetFullPathSO);
  }
  if (targetName != targetNameSO && targetName != targetNameReal) {
    outputs.push_back(targetFullPath);
  }

  // Write the build rule.
  this->WriteMakeRule(*this->BuildFileStream, 0, outputs, depends, commands,
                      false);

  // Write the main driver rule to build everything in this target.
  this->WriteTargetDriverRule(targetFullPath, relink);

  // Clean all the possible library names and symlinks.
  this->CleanFiles.insert(this->CleanFiles.end(), libCleanFiles.begin(),
                          libCleanFiles.end());
}
