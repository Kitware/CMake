/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmGlobalUnixMakefileGenerator3.h"

#include <algorithm>
#include <functional>
#include <sstream>
#include <utility>

#include "cmAlgorithms.h"
#include "cmDocumentationEntry.h"
#include "cmGeneratedFileStream.h"
#include "cmGeneratorTarget.h"
#include "cmGlobalGenerator.h"
#include "cmLocalGenerator.h"
#include "cmLocalUnixMakefileGenerator3.h"
#include "cmMakefile.h"
#include "cmMakefileTargetGenerator.h"
#include "cmOutputConverter.h"
#include "cmState.h"
#include "cmStateDirectory.h"
#include "cmStateTypes.h"
#include "cmSystemTools.h"
#include "cmTargetDepend.h"
#include "cmake.h"

cmGlobalUnixMakefileGenerator3::cmGlobalUnixMakefileGenerator3(cmake* cm)
  : cmGlobalCommonGenerator(cm)
{
  // This type of makefile always requires unix style paths
  this->ForceUnixPaths = true;
  this->FindMakeProgramFile = "CMakeUnixFindMake.cmake";
  this->ToolSupportsColor = true;

#if defined(_WIN32) || defined(__VMS)
  this->UseLinkScript = false;
#else
  this->UseLinkScript = true;
#endif
  this->CommandDatabase = nullptr;

  this->IncludeDirective = "include";
  this->DefineWindowsNULL = false;
  this->PassMakeflags = false;
  this->UnixCD = true;
}

void cmGlobalUnixMakefileGenerator3::EnableLanguage(
  std::vector<std::string> const& languages, cmMakefile* mf, bool optional)
{
  this->cmGlobalGenerator::EnableLanguage(languages, mf, optional);
  for (std::string const& language : languages) {
    if (language == "NONE") {
      continue;
    }
    this->ResolveLanguageCompiler(language, mf, optional);
  }
}

///! Create a local generator appropriate to this Global Generator
cmLocalGenerator* cmGlobalUnixMakefileGenerator3::CreateLocalGenerator(
  cmMakefile* mf)
{
  return new cmLocalUnixMakefileGenerator3(this, mf);
}

void cmGlobalUnixMakefileGenerator3::GetDocumentation(
  cmDocumentationEntry& entry)
{
  entry.Name = cmGlobalUnixMakefileGenerator3::GetActualName();
  entry.Brief = "Generates standard UNIX makefiles.";
}

std::string cmGlobalUnixMakefileGenerator3::GetEditCacheCommand() const
{
  // If generating for an extra IDE, the edit_cache target cannot
  // launch a terminal-interactive tool, so always use cmake-gui.
  if (!this->GetExtraGeneratorName().empty()) {
    return cmSystemTools::GetCMakeGUICommand();
  }

  // Use an internal cache entry to track the latest dialog used
  // to edit the cache, and use that for the edit_cache target.
  cmake* cm = this->GetCMakeInstance();
  std::string editCacheCommand = cm->GetCMakeEditCommand();
  if (!cm->GetCacheDefinition("CMAKE_EDIT_COMMAND") ||
      !editCacheCommand.empty()) {
    if (editCacheCommand.empty()) {
      editCacheCommand = cmSystemTools::GetCMakeCursesCommand();
    }
    if (editCacheCommand.empty()) {
      editCacheCommand = cmSystemTools::GetCMakeGUICommand();
    }
    if (!editCacheCommand.empty()) {
      cm->AddCacheEntry("CMAKE_EDIT_COMMAND", editCacheCommand.c_str(),
                        "Path to cache edit program executable.",
                        cmStateEnums::INTERNAL);
    }
  }
  const char* edit_cmd = cm->GetCacheDefinition("CMAKE_EDIT_COMMAND");
  return edit_cmd ? edit_cmd : "";
}

void cmGlobalUnixMakefileGenerator3::ComputeTargetObjectDirectory(
  cmGeneratorTarget* gt) const
{
  // Compute full path to object file directory for this target.
  std::string dir;
  dir += gt->LocalGenerator->GetCurrentBinaryDirectory();
  dir += "/";
  dir += gt->LocalGenerator->GetTargetDirectory(gt);
  dir += "/";
  gt->ObjectDirectory = dir;
}

void cmGlobalUnixMakefileGenerator3::Configure()
{
  // Initialize CMAKE_EDIT_COMMAND cache entry.
  this->GetEditCacheCommand();

  this->cmGlobalGenerator::Configure();
}

void cmGlobalUnixMakefileGenerator3::Generate()
{
  // first do superclass method
  this->cmGlobalGenerator::Generate();

  // initialize progress
  unsigned long total = 0;
  for (auto const& pmi : this->ProgressMap) {
    total += pmi.second.NumberOfActions;
  }

  // write each target's progress.make this loop is done twice. Bascially the
  // Generate pass counts all the actions, the first loop below determines
  // how many actions have progress updates for each target and writes to
  // corrrect variable values for everything except the all targets. The
  // second loop actually writes out correct values for the all targets as
  // well. This is because the all targets require more information that is
  // computed in the first loop.
  unsigned long current = 0;
  for (auto& pmi : this->ProgressMap) {
    pmi.second.WriteProgressVariables(total, current);
  }
  for (cmLocalGenerator* lg : this->LocalGenerators) {
    std::string markFileName = lg->GetCurrentBinaryDirectory();
    markFileName += "/";
    markFileName += "/CMakeFiles";
    markFileName += "/progress.marks";
    cmGeneratedFileStream markFile(markFileName);
    markFile << this->CountProgressMarksInAll(lg) << "\n";
  }

  // write the main makefile
  this->WriteMainMakefile2();
  this->WriteMainCMakefile();

  if (this->CommandDatabase != nullptr) {
    *this->CommandDatabase << std::endl << "]";
    delete this->CommandDatabase;
    this->CommandDatabase = nullptr;
  }
}

void cmGlobalUnixMakefileGenerator3::AddCXXCompileCommand(
  const std::string& sourceFile, const std::string& workingDirectory,
  const std::string& compileCommand)
{
  if (this->CommandDatabase == nullptr) {
    std::string commandDatabaseName =
      std::string(this->GetCMakeInstance()->GetHomeOutputDirectory()) +
      "/compile_commands.json";
    this->CommandDatabase = new cmGeneratedFileStream(commandDatabaseName);
    *this->CommandDatabase << "[" << std::endl;
  } else {
    *this->CommandDatabase << "," << std::endl;
  }
  *this->CommandDatabase << "{" << std::endl
                         << "  \"directory\": \""
                         << cmGlobalGenerator::EscapeJSON(workingDirectory)
                         << "\"," << std::endl
                         << "  \"command\": \""
                         << cmGlobalGenerator::EscapeJSON(compileCommand)
                         << "\"," << std::endl
                         << "  \"file\": \""
                         << cmGlobalGenerator::EscapeJSON(sourceFile) << "\""
                         << std::endl
                         << "}";
}

void cmGlobalUnixMakefileGenerator3::WriteMainMakefile2()
{
  // Open the output file.  This should not be copy-if-different
  // because the check-build-system step compares the makefile time to
  // see if the build system must be regenerated.
  std::string makefileName =
    this->GetCMakeInstance()->GetHomeOutputDirectory();
  makefileName += "/CMakeFiles";
  makefileName += "/Makefile2";
  cmGeneratedFileStream makefileStream(makefileName, false,
                                       this->GetMakefileEncoding());
  if (!makefileStream) {
    return;
  }

  // get a local generator for some useful methods
  cmLocalUnixMakefileGenerator3* lg =
    static_cast<cmLocalUnixMakefileGenerator3*>(this->LocalGenerators[0]);

  // Write the do not edit header.
  lg->WriteDisclaimer(makefileStream);

  // Write the main entry point target.  This must be the VERY first
  // target so that make with no arguments will run it.
  // Just depend on the all target to drive the build.
  std::vector<std::string> depends;
  std::vector<std::string> no_commands;
  depends.emplace_back("all");

  // Write the rule.
  lg->WriteMakeRule(makefileStream,
                    "Default target executed when no arguments are "
                    "given to make.",
                    "default_target", depends, no_commands, true);

  depends.clear();

  // The all and preinstall rules might never have any dependencies
  // added to them.
  if (!this->EmptyRuleHackDepends.empty()) {
    depends.push_back(this->EmptyRuleHackDepends);
  }

  // Write and empty all:
  lg->WriteMakeRule(makefileStream, "The main recursive all target", "all",
                    depends, no_commands, true);

  // Write an empty preinstall:
  lg->WriteMakeRule(makefileStream, "The main recursive preinstall target",
                    "preinstall", depends, no_commands, true);

  // Write an empty clean:
  lg->WriteMakeRule(makefileStream, "The main recursive clean target", "clean",
                    depends, no_commands, true);

  // Write out the "special" stuff
  lg->WriteSpecialTargetsTop(makefileStream);

  // write the target convenience rules
  for (cmLocalGenerator* localGen : this->LocalGenerators) {
    lg = static_cast<cmLocalUnixMakefileGenerator3*>(localGen);
    this->WriteConvenienceRules2(makefileStream, lg);
  }

  lg = static_cast<cmLocalUnixMakefileGenerator3*>(this->LocalGenerators[0]);
  lg->WriteSpecialTargetsBottom(makefileStream);
}

void cmGlobalUnixMakefileGenerator3::WriteMainCMakefile()
{
  if (this->GlobalSettingIsOn("CMAKE_SUPPRESS_REGENERATION")) {
    return;
  }

  // Open the output file.  This should not be copy-if-different
  // because the check-build-system step compares the makefile time to
  // see if the build system must be regenerated.
  std::string cmakefileName =
    this->GetCMakeInstance()->GetHomeOutputDirectory();
  cmakefileName += "/CMakeFiles";
  cmakefileName += "/Makefile.cmake";
  cmGeneratedFileStream cmakefileStream(cmakefileName);
  if (!cmakefileStream) {
    return;
  }

  std::string makefileName =
    this->GetCMakeInstance()->GetHomeOutputDirectory();
  makefileName += "/Makefile";

  // get a local generator for some useful methods
  cmLocalUnixMakefileGenerator3* lg =
    static_cast<cmLocalUnixMakefileGenerator3*>(this->LocalGenerators[0]);

  // Write the do not edit header.
  lg->WriteDisclaimer(cmakefileStream);

  // Save the generator name
  cmakefileStream << "# The generator used is:\n"
                  << "set(CMAKE_DEPENDS_GENERATOR \"" << this->GetName()
                  << "\")\n\n";

  // for each cmMakefile get its list of dependencies
  std::vector<std::string> lfiles;
  for (cmLocalGenerator* localGen : this->LocalGenerators) {
    lg = static_cast<cmLocalUnixMakefileGenerator3*>(localGen);

    // Get the list of files contributing to this generation step.
    lfiles.insert(lfiles.end(), lg->GetMakefile()->GetListFiles().begin(),
                  lg->GetMakefile()->GetListFiles().end());
  }

  cmake* cm = this->GetCMakeInstance();
  if (cm->DoWriteGlobVerifyTarget()) {
    lfiles.push_back(cm->GetGlobVerifyScript());
    lfiles.push_back(cm->GetGlobVerifyStamp());
  }

  // Sort the list and remove duplicates.
  std::sort(lfiles.begin(), lfiles.end(), std::less<std::string>());
#if !defined(__VMS) // The Compaq STL on VMS crashes, so accept duplicates.
  std::vector<std::string>::iterator new_end =
    std::unique(lfiles.begin(), lfiles.end());
  lfiles.erase(new_end, lfiles.end());
#endif

  // reset lg to the first makefile
  lg = static_cast<cmLocalUnixMakefileGenerator3*>(this->LocalGenerators[0]);

  std::string currentBinDir = lg->GetCurrentBinaryDirectory();
  // Save the list to the cmake file.
  cmakefileStream
    << "# The top level Makefile was generated from the following files:\n"
    << "set(CMAKE_MAKEFILE_DEPENDS\n"
    << "  \"CMakeCache.txt\"\n";
  for (std::string const& f : lfiles) {
    cmakefileStream << "  \""
                    << lg->MaybeConvertToRelativePath(currentBinDir, f)
                    << "\"\n";
  }
  cmakefileStream << "  )\n\n";

  // Build the path to the cache check file.
  std::string check = this->GetCMakeInstance()->GetHomeOutputDirectory();
  check += "/CMakeFiles";
  check += "/cmake.check_cache";

  // Set the corresponding makefile in the cmake file.
  cmakefileStream << "# The corresponding makefile is:\n"
                  << "set(CMAKE_MAKEFILE_OUTPUTS\n"
                  << "  \""
                  << lg->MaybeConvertToRelativePath(currentBinDir,
                                                    makefileName)
                  << "\"\n"
                  << "  \""
                  << lg->MaybeConvertToRelativePath(currentBinDir, check)
                  << "\"\n";
  cmakefileStream << "  )\n\n";

  const std::string binDir = lg->GetBinaryDirectory();

  // CMake must rerun if a byproduct is missing.
  {
    cmakefileStream << "# Byproducts of CMake generate step:\n"
                    << "set(CMAKE_MAKEFILE_PRODUCTS\n";
    const std::vector<std::string>& outfiles =
      lg->GetMakefile()->GetOutputFiles();
    for (std::string const& outfile : outfiles) {
      cmakefileStream << "  \""
                      << lg->MaybeConvertToRelativePath(binDir, outfile)
                      << "\"\n";
    }

    // add in all the directory information files
    std::string tmpStr;
    for (cmLocalGenerator* localGen : this->LocalGenerators) {
      lg = static_cast<cmLocalUnixMakefileGenerator3*>(localGen);
      tmpStr = lg->GetCurrentBinaryDirectory();
      tmpStr += "/CMakeFiles";
      tmpStr += "/CMakeDirectoryInformation.cmake";
      cmakefileStream << "  \""
                      << lg->MaybeConvertToRelativePath(binDir, tmpStr)
                      << "\"\n";
    }
    cmakefileStream << "  )\n\n";
  }

  this->WriteMainCMakefileLanguageRules(cmakefileStream,
                                        this->LocalGenerators);
}

void cmGlobalUnixMakefileGenerator3::WriteMainCMakefileLanguageRules(
  cmGeneratedFileStream& cmakefileStream,
  std::vector<cmLocalGenerator*>& lGenerators)
{
  cmLocalUnixMakefileGenerator3* lg;

  // now list all the target info files
  cmakefileStream << "# Dependency information for all targets:\n";
  cmakefileStream << "set(CMAKE_DEPEND_INFO_FILES\n";
  for (cmLocalGenerator* lGenerator : lGenerators) {
    lg = static_cast<cmLocalUnixMakefileGenerator3*>(lGenerator);
    // for all of out targets
    const std::vector<cmGeneratorTarget*>& tgts = lg->GetGeneratorTargets();
    for (cmGeneratorTarget* tgt : tgts) {
      if ((tgt->GetType() == cmStateEnums::EXECUTABLE) ||
          (tgt->GetType() == cmStateEnums::STATIC_LIBRARY) ||
          (tgt->GetType() == cmStateEnums::SHARED_LIBRARY) ||
          (tgt->GetType() == cmStateEnums::MODULE_LIBRARY) ||
          (tgt->GetType() == cmStateEnums::OBJECT_LIBRARY) ||
          (tgt->GetType() == cmStateEnums::UTILITY)) {
        cmGeneratorTarget* gt = tgt;
        std::string tname = lg->GetRelativeTargetDirectory(gt);
        tname += "/DependInfo.cmake";
        cmSystemTools::ConvertToUnixSlashes(tname);
        cmakefileStream << "  \"" << tname << "\"\n";
      }
    }
  }
  cmakefileStream << "  )\n";
}

void cmGlobalUnixMakefileGenerator3::WriteDirectoryRule2(
  std::ostream& ruleFileStream, cmLocalUnixMakefileGenerator3* lg,
  const char* pass, bool check_all, bool check_relink)
{
  // Get the relative path to the subdirectory from the top.
  std::string makeTarget = lg->GetCurrentBinaryDirectory();
  makeTarget += "/";
  makeTarget += pass;

  // The directory-level rule should depend on the target-level rules
  // for all targets in the directory.
  std::vector<std::string> depends;
  const std::vector<cmGeneratorTarget*>& targets = lg->GetGeneratorTargets();
  for (cmGeneratorTarget* gtarget : targets) {
    int type = gtarget->GetType();
    if ((type == cmStateEnums::EXECUTABLE) ||
        (type == cmStateEnums::STATIC_LIBRARY) ||
        (type == cmStateEnums::SHARED_LIBRARY) ||
        (type == cmStateEnums::MODULE_LIBRARY) ||
        (type == cmStateEnums::OBJECT_LIBRARY) ||
        (type == cmStateEnums::UTILITY)) {
      // Add this to the list of depends rules in this directory.
      if ((!check_all || !gtarget->GetPropertyAsBool("EXCLUDE_FROM_ALL")) &&
          (!check_relink ||
           gtarget->NeedRelinkBeforeInstall(lg->GetConfigName()))) {
        std::string tname = lg->GetRelativeTargetDirectory(gtarget);
        tname += "/";
        tname += pass;
        depends.push_back(std::move(tname));
      }
    }
  }

  // The directory-level rule should depend on the directory-level
  // rules of the subdirectories.
  std::vector<cmStateSnapshot> children = lg->GetStateSnapshot().GetChildren();
  for (cmStateSnapshot const& c : children) {
    std::string subdir = c.GetDirectory().GetCurrentBinary();
    subdir += "/";
    subdir += pass;
    depends.push_back(std::move(subdir));
  }

  // Work-around for makes that drop rules that have no dependencies
  // or commands.
  if (depends.empty() && !this->EmptyRuleHackDepends.empty()) {
    depends.push_back(this->EmptyRuleHackDepends);
  }

  // Write the rule.
  std::string doc = "Convenience name for \"";
  doc += pass;
  doc += "\" pass in the directory.";
  std::vector<std::string> no_commands;
  lg->WriteMakeRule(ruleFileStream, doc.c_str(), makeTarget, depends,
                    no_commands, true);
}

void cmGlobalUnixMakefileGenerator3::WriteDirectoryRules2(
  std::ostream& ruleFileStream, cmLocalUnixMakefileGenerator3* lg)
{
  // Only subdirectories need these rules.
  if (lg->IsRootMakefile()) {
    return;
  }

  // Begin the directory-level rules section.
  std::string dir =
    cmSystemTools::ConvertToOutputPath(lg->MaybeConvertToRelativePath(
      lg->GetBinaryDirectory(), lg->GetCurrentBinaryDirectory()));
  lg->WriteDivider(ruleFileStream);
  ruleFileStream << "# Directory level rules for directory " << dir << "\n\n";

  // Write directory-level rules for "all".
  this->WriteDirectoryRule2(ruleFileStream, lg, "all", true, false);

  // Write directory-level rules for "clean".
  this->WriteDirectoryRule2(ruleFileStream, lg, "clean", false, false);

  // Write directory-level rules for "preinstall".
  this->WriteDirectoryRule2(ruleFileStream, lg, "preinstall", true, true);
}

std::vector<cmGlobalGenerator::GeneratedMakeCommand>
cmGlobalUnixMakefileGenerator3::GenerateBuildCommand(
  const std::string& makeProgram, const std::string& /*projectName*/,
  const std::string& /*projectDir*/,
  std::vector<std::string> const& targetNames, const std::string& /*config*/,
  bool fast, int jobs, bool verbose,
  std::vector<std::string> const& makeOptions)
{
  std::unique_ptr<cmMakefile> mfu;
  cmMakefile* mf;
  if (!this->Makefiles.empty()) {
    mf = this->Makefiles[0];
  } else {
    cmStateSnapshot snapshot = this->CMakeInstance->GetCurrentSnapshot();
    snapshot.GetDirectory().SetCurrentSource(
      this->CMakeInstance->GetHomeDirectory());
    snapshot.GetDirectory().SetCurrentBinary(
      this->CMakeInstance->GetHomeOutputDirectory());
    snapshot.SetDefaultDefinitions();
    mfu = cm::make_unique<cmMakefile>(this, snapshot);
    mf = mfu.get();
  }

  GeneratedMakeCommand makeCommand;

  // Make it possible to set verbosity also from command line
  if (verbose) {
    makeCommand.Add(cmSystemTools::GetCMakeCommand());
    makeCommand.Add("-E");
    makeCommand.Add("env");
    makeCommand.Add("VERBOSE=1");
  }
  makeCommand.Add(this->SelectMakeProgram(makeProgram));

  if (jobs != cmake::NO_BUILD_PARALLEL_LEVEL) {
    makeCommand.Add("-j");
    if (jobs != cmake::DEFAULT_BUILD_PARALLEL_LEVEL) {
      makeCommand.Add(std::to_string(jobs));
    }
  }

  makeCommand.Add(makeOptions.begin(), makeOptions.end());
  for (auto tname : targetNames) {
    if (!tname.empty()) {
      if (fast) {
        tname += "/fast";
      }
      tname =
        mf->GetStateSnapshot().GetDirectory().ConvertToRelPathIfNotContained(
          mf->GetState()->GetBinaryDirectory(), tname);
      cmSystemTools::ConvertToOutputSlashes(tname);
      makeCommand.Add(std::move(tname));
    }
  }
  return { std::move(makeCommand) };
}

void cmGlobalUnixMakefileGenerator3::WriteConvenienceRules(
  std::ostream& ruleFileStream, std::set<std::string>& emitted)
{
  std::vector<std::string> depends;
  std::vector<std::string> commands;

  bool regenerate = !this->GlobalSettingIsOn("CMAKE_SUPPRESS_REGENERATION");
  if (regenerate) {
    depends.emplace_back("cmake_check_build_system");
  }

  // write the target convenience rules
  for (cmLocalGenerator* localGen : this->LocalGenerators) {
    cmLocalUnixMakefileGenerator3* lg =
      static_cast<cmLocalUnixMakefileGenerator3*>(localGen);
    // for each target Generate the rule files for each target.
    const std::vector<cmGeneratorTarget*>& targets = lg->GetGeneratorTargets();
    for (cmGeneratorTarget* gtarget : targets) {
      // Don't emit the same rule twice (e.g. two targets with the same
      // simple name)
      int type = gtarget->GetType();
      std::string name = gtarget->GetName();
      if (!name.empty() && emitted.insert(name).second &&
          // Handle user targets here.  Global targets are handled in
          // the local generator on a per-directory basis.
          ((type == cmStateEnums::EXECUTABLE) ||
           (type == cmStateEnums::STATIC_LIBRARY) ||
           (type == cmStateEnums::SHARED_LIBRARY) ||
           (type == cmStateEnums::MODULE_LIBRARY) ||
           (type == cmStateEnums::OBJECT_LIBRARY) ||
           (type == cmStateEnums::UTILITY))) {
        // Add a rule to build the target by name.
        lg->WriteDivider(ruleFileStream);
        ruleFileStream << "# Target rules for targets named " << name
                       << "\n\n";

        // Write the rule.
        commands.clear();
        std::string tmp = "CMakeFiles/";
        tmp += "Makefile2";
        commands.push_back(lg->GetRecursiveMakeCall(tmp, name));
        depends.clear();
        if (regenerate) {
          depends.emplace_back("cmake_check_build_system");
        }
        lg->WriteMakeRule(ruleFileStream, "Build rule for target.", name,
                          depends, commands, true);

        // Add a fast rule to build the target
        std::string localName = lg->GetRelativeTargetDirectory(gtarget);
        std::string makefileName;
        makefileName = localName;
        makefileName += "/build.make";
        depends.clear();
        commands.clear();
        std::string makeTargetName = localName;
        makeTargetName += "/build";
        localName = name;
        localName += "/fast";
        commands.push_back(
          lg->GetRecursiveMakeCall(makefileName, makeTargetName));
        lg->WriteMakeRule(ruleFileStream, "fast build rule for target.",
                          localName, depends, commands, true);

        // Add a local name for the rule to relink the target before
        // installation.
        if (gtarget->NeedRelinkBeforeInstall(lg->GetConfigName())) {
          makeTargetName = lg->GetRelativeTargetDirectory(gtarget);
          makeTargetName += "/preinstall";
          localName = name;
          localName += "/preinstall";
          depends.clear();
          commands.clear();
          commands.push_back(
            lg->GetRecursiveMakeCall(makefileName, makeTargetName));
          lg->WriteMakeRule(ruleFileStream,
                            "Manual pre-install relink rule for target.",
                            localName, depends, commands, true);
        }
      }
    }
  }
}

void cmGlobalUnixMakefileGenerator3::WriteConvenienceRules2(
  std::ostream& ruleFileStream, cmLocalUnixMakefileGenerator3* lg)
{
  std::vector<std::string> depends;
  std::vector<std::string> commands;
  std::string localName;
  std::string makeTargetName;

  // write the directory level rules for this local gen
  this->WriteDirectoryRules2(ruleFileStream, lg);

  bool regenerate = !this->GlobalSettingIsOn("CMAKE_SUPPRESS_REGENERATION");
  if (regenerate) {
    depends.emplace_back("cmake_check_build_system");
  }

  // for each target Generate the rule files for each target.
  const std::vector<cmGeneratorTarget*>& targets = lg->GetGeneratorTargets();
  for (cmGeneratorTarget* gtarget : targets) {
    int type = gtarget->GetType();
    std::string name = gtarget->GetName();
    if (!name.empty() &&
        ((type == cmStateEnums::EXECUTABLE) ||
         (type == cmStateEnums::STATIC_LIBRARY) ||
         (type == cmStateEnums::SHARED_LIBRARY) ||
         (type == cmStateEnums::MODULE_LIBRARY) ||
         (type == cmStateEnums::OBJECT_LIBRARY) ||
         (type == cmStateEnums::UTILITY))) {
      std::string makefileName;
      // Add a rule to build the target by name.
      localName = lg->GetRelativeTargetDirectory(gtarget);
      makefileName = localName;
      makefileName += "/build.make";

      lg->WriteDivider(ruleFileStream);
      ruleFileStream << "# Target rules for target " << localName << "\n\n";

      commands.clear();
      makeTargetName = localName;
      makeTargetName += "/depend";
      commands.push_back(
        lg->GetRecursiveMakeCall(makefileName, makeTargetName));

      makeTargetName = localName;
      makeTargetName += "/build";
      commands.push_back(
        lg->GetRecursiveMakeCall(makefileName, makeTargetName));

      // Write the rule.
      localName += "/all";
      depends.clear();

      cmLocalUnixMakefileGenerator3::EchoProgress progress;
      progress.Dir = lg->GetBinaryDirectory();
      progress.Dir += "/CMakeFiles";
      {
        std::ostringstream progressArg;
        const char* sep = "";
        std::vector<unsigned long> const& progFiles =
          this->ProgressMap[gtarget].Marks;
        for (unsigned long progFile : progFiles) {
          progressArg << sep << progFile;
          sep = ",";
        }
        progress.Arg = progressArg.str();
      }

      bool targetMessages = true;
      if (const char* tgtMsg =
            this->GetCMakeInstance()->GetState()->GetGlobalProperty(
              "TARGET_MESSAGES")) {
        targetMessages = cmSystemTools::IsOn(tgtMsg);
      }

      if (targetMessages) {
        lg->AppendEcho(commands, "Built target " + name,
                       cmLocalUnixMakefileGenerator3::EchoNormal, &progress);
      }

      this->AppendGlobalTargetDepends(depends, gtarget);
      lg->WriteMakeRule(ruleFileStream, "All Build rule for target.",
                        localName, depends, commands, true);

      // add the all/all dependency
      if (!this->IsExcluded(gtarget)) {
        depends.clear();
        depends.push_back(localName);
        commands.clear();
        lg->WriteMakeRule(ruleFileStream, "Include target in all.", "all",
                          depends, commands, true);
      }

      // Write the rule.
      commands.clear();

      {
        // TODO: Convert the total progress count to a make variable.
        std::ostringstream progCmd;
        progCmd << "$(CMAKE_COMMAND) -E cmake_progress_start ";
        // # in target
        progCmd << lg->ConvertToOutputFormat(
          cmSystemTools::CollapseFullPath(progress.Dir),
          cmOutputConverter::SHELL);
        //
        std::set<cmGeneratorTarget const*> emitted;
        progCmd << " " << this->CountProgressMarksInTarget(gtarget, emitted);
        commands.push_back(progCmd.str());
      }
      std::string tmp = "CMakeFiles/";
      tmp += "Makefile2";
      commands.push_back(lg->GetRecursiveMakeCall(tmp, localName));
      {
        std::ostringstream progCmd;
        progCmd << "$(CMAKE_COMMAND) -E cmake_progress_start "; // # 0
        progCmd << lg->ConvertToOutputFormat(
          cmSystemTools::CollapseFullPath(progress.Dir),
          cmOutputConverter::SHELL);
        progCmd << " 0";
        commands.push_back(progCmd.str());
      }
      depends.clear();
      if (regenerate) {
        depends.emplace_back("cmake_check_build_system");
      }
      localName = lg->GetRelativeTargetDirectory(gtarget);
      localName += "/rule";
      lg->WriteMakeRule(ruleFileStream,
                        "Build rule for subdir invocation for target.",
                        localName, depends, commands, true);

      // Add a target with the canonical name (no prefix, suffix or path).
      commands.clear();
      depends.clear();
      depends.push_back(localName);
      lg->WriteMakeRule(ruleFileStream, "Convenience name for target.", name,
                        depends, commands, true);

      // Add rules to prepare the target for installation.
      if (gtarget->NeedRelinkBeforeInstall(lg->GetConfigName())) {
        localName = lg->GetRelativeTargetDirectory(gtarget);
        localName += "/preinstall";
        depends.clear();
        commands.clear();
        commands.push_back(lg->GetRecursiveMakeCall(makefileName, localName));
        lg->WriteMakeRule(ruleFileStream,
                          "Pre-install relink rule for target.", localName,
                          depends, commands, true);

        if (!this->IsExcluded(gtarget)) {
          depends.clear();
          depends.push_back(localName);
          commands.clear();
          lg->WriteMakeRule(ruleFileStream, "Prepare target for install.",
                            "preinstall", depends, commands, true);
        }
      }

      // add the clean rule
      localName = lg->GetRelativeTargetDirectory(gtarget);
      makeTargetName = localName;
      makeTargetName += "/clean";
      depends.clear();
      commands.clear();
      commands.push_back(
        lg->GetRecursiveMakeCall(makefileName, makeTargetName));
      lg->WriteMakeRule(ruleFileStream, "clean rule for target.",
                        makeTargetName, depends, commands, true);
      commands.clear();
      depends.push_back(makeTargetName);
      lg->WriteMakeRule(ruleFileStream, "clean rule for target.", "clean",
                        depends, commands, true);
    }
  }
}

// Build a map that contains the set of targets used by each local
// generator directory level.
void cmGlobalUnixMakefileGenerator3::InitializeProgressMarks()
{
  this->DirectoryTargetsMap.clear();
  // Loop over all targets in all local generators.
  for (cmLocalGenerator* lg : this->LocalGenerators) {
    const std::vector<cmGeneratorTarget*>& targets = lg->GetGeneratorTargets();
    for (cmGeneratorTarget* gt : targets) {
      cmLocalGenerator* tlg = gt->GetLocalGenerator();

      if (gt->GetType() == cmStateEnums::INTERFACE_LIBRARY ||
          gt->GetPropertyAsBool("EXCLUDE_FROM_ALL")) {
        continue;
      }

      cmStateSnapshot csnp = lg->GetStateSnapshot();
      cmStateSnapshot tsnp = tlg->GetStateSnapshot();

      // Consider the directory containing the target and all its
      // parents until something excludes the target.
      for (; csnp.IsValid() && !this->IsExcluded(csnp, tsnp);
           csnp = csnp.GetBuildsystemDirectoryParent()) {
        // This local generator includes the target.
        std::set<cmGeneratorTarget const*>& targetSet =
          this->DirectoryTargetsMap[csnp];
        targetSet.insert(gt);

        // Add dependencies of the included target.  An excluded
        // target may still be included if it is a dependency of a
        // non-excluded target.
        TargetDependSet const& tgtdeps = this->GetTargetDirectDepends(gt);
        for (cmTargetDepend const& tgtdep : tgtdeps) {
          targetSet.insert(tgtdep);
        }
      }
    }
  }
}

size_t cmGlobalUnixMakefileGenerator3::CountProgressMarksInTarget(
  cmGeneratorTarget const* target, std::set<cmGeneratorTarget const*>& emitted)
{
  size_t count = 0;
  if (emitted.insert(target).second) {
    count = this->ProgressMap[target].Marks.size();
    TargetDependSet const& depends = this->GetTargetDirectDepends(target);
    for (cmTargetDepend const& depend : depends) {
      if (depend->GetType() == cmStateEnums::INTERFACE_LIBRARY) {
        continue;
      }
      count += this->CountProgressMarksInTarget(depend, emitted);
    }
  }
  return count;
}

size_t cmGlobalUnixMakefileGenerator3::CountProgressMarksInAll(
  cmLocalGenerator* lg)
{
  size_t count = 0;
  std::set<cmGeneratorTarget const*> emitted;
  std::set<cmGeneratorTarget const*> const& targets =
    this->DirectoryTargetsMap[lg->GetStateSnapshot()];
  for (cmGeneratorTarget const* target : targets) {
    count += this->CountProgressMarksInTarget(target, emitted);
  }
  return count;
}

void cmGlobalUnixMakefileGenerator3::RecordTargetProgress(
  cmMakefileTargetGenerator* tg)
{
  TargetProgress& tp = this->ProgressMap[tg->GetGeneratorTarget()];
  tp.NumberOfActions = tg->GetNumberOfProgressActions();
  tp.VariableFile = tg->GetProgressFileNameFull();
}

void cmGlobalUnixMakefileGenerator3::TargetProgress::WriteProgressVariables(
  unsigned long total, unsigned long& current)
{
  cmGeneratedFileStream fout(this->VariableFile);
  for (unsigned long i = 1; i <= this->NumberOfActions; ++i) {
    fout << "CMAKE_PROGRESS_" << i << " = ";
    if (total <= 100) {
      unsigned long num = i + current;
      fout << num;
      this->Marks.push_back(num);
    } else if (((i + current) * 100) / total >
               ((i - 1 + current) * 100) / total) {
      unsigned long num = ((i + current) * 100) / total;
      fout << num;
      this->Marks.push_back(num);
    }
    fout << "\n";
  }
  fout << "\n";
  current += this->NumberOfActions;
}

void cmGlobalUnixMakefileGenerator3::AppendGlobalTargetDepends(
  std::vector<std::string>& depends, cmGeneratorTarget* target)
{
  TargetDependSet const& depends_set = this->GetTargetDirectDepends(target);
  for (cmTargetDepend const& i : depends_set) {
    // Create the target-level dependency.
    cmGeneratorTarget const* dep = i;
    if (dep->GetType() == cmStateEnums::INTERFACE_LIBRARY) {
      continue;
    }
    cmLocalUnixMakefileGenerator3* lg3 =
      static_cast<cmLocalUnixMakefileGenerator3*>(dep->GetLocalGenerator());
    std::string tgtName =
      lg3->GetRelativeTargetDirectory(const_cast<cmGeneratorTarget*>(dep));
    tgtName += "/all";
    depends.push_back(tgtName);
  }
}

void cmGlobalUnixMakefileGenerator3::WriteHelpRule(
  std::ostream& ruleFileStream, cmLocalUnixMakefileGenerator3* lg)
{
  // add the help target
  std::string path;
  std::vector<std::string> no_depends;
  std::vector<std::string> commands;
  lg->AppendEcho(commands,
                 "The following are some of the valid targets "
                 "for this Makefile:");
  lg->AppendEcho(commands, "... all (the default if no target is provided)");
  lg->AppendEcho(commands, "... clean");
  if (!this->GlobalSettingIsOn("CMAKE_SUPPRESS_REGENERATION")) {
    lg->AppendEcho(commands, "... depend");
  }

  // Keep track of targets already listed.
  std::set<std::string> emittedTargets;

  // for each local generator
  for (cmLocalGenerator* localGen : this->LocalGenerators) {
    cmLocalUnixMakefileGenerator3* lg2 =
      static_cast<cmLocalUnixMakefileGenerator3*>(localGen);
    // for the passed in makefile or if this is the top Makefile wripte out
    // the targets
    if (lg2 == lg || lg->IsRootMakefile()) {
      // for each target Generate the rule files for each target.
      const std::vector<cmGeneratorTarget*>& targets =
        lg2->GetGeneratorTargets();
      for (cmGeneratorTarget* target : targets) {
        cmStateEnums::TargetType type = target->GetType();
        if ((type == cmStateEnums::EXECUTABLE) ||
            (type == cmStateEnums::STATIC_LIBRARY) ||
            (type == cmStateEnums::SHARED_LIBRARY) ||
            (type == cmStateEnums::MODULE_LIBRARY) ||
            (type == cmStateEnums::OBJECT_LIBRARY) ||
            (type == cmStateEnums::GLOBAL_TARGET) ||
            (type == cmStateEnums::UTILITY)) {
          std::string const& name = target->GetName();
          if (emittedTargets.insert(name).second) {
            path = "... ";
            path += name;
            lg->AppendEcho(commands, path);
          }
        }
      }
    }
  }
  for (std::string const& o : lg->GetLocalHelp()) {
    path = "... ";
    path += o;
    lg->AppendEcho(commands, path);
  }
  lg->WriteMakeRule(ruleFileStream, "Help Target", "help", no_depends,
                    commands, true);
  ruleFileStream << "\n\n";
}
