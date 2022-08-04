/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmGlobalUnixMakefileGenerator3.h"

#include <algorithm>
#include <functional>
#include <sstream>
#include <utility>

#include <cm/memory>
#include <cmext/algorithm>
#include <cmext/memory>

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
#include "cmStateTypes.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmTargetDepend.h"
#include "cmValue.h"
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

  this->IncludeDirective = "include";
  this->LineContinueDirective = "\\\n";
  this->DefineWindowsNULL = false;
  this->PassMakeflags = false;
  this->UnixCD = true;
}

cmGlobalUnixMakefileGenerator3::~cmGlobalUnixMakefileGenerator3() = default;

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

//! Create a local generator appropriate to this Global Generator
std::unique_ptr<cmLocalGenerator>
cmGlobalUnixMakefileGenerator3::CreateLocalGenerator(cmMakefile* mf)
{
  return std::unique_ptr<cmLocalGenerator>(
    cm::make_unique<cmLocalUnixMakefileGenerator3>(this, mf));
}

void cmGlobalUnixMakefileGenerator3::GetDocumentation(
  cmDocumentationEntry& entry)
{
  entry.Name = cmGlobalUnixMakefileGenerator3::GetActualName();
  entry.Brief = "Generates standard UNIX makefiles.";
}

void cmGlobalUnixMakefileGenerator3::ComputeTargetObjectDirectory(
  cmGeneratorTarget* gt) const
{
  // Compute full path to object file directory for this target.
  std::string dir =
    cmStrCat(gt->LocalGenerator->GetCurrentBinaryDirectory(), '/',
             gt->LocalGenerator->GetTargetDirectory(gt), '/');
  gt->ObjectDirectory = dir;
}

bool cmGlobalUnixMakefileGenerator3::CanEscapeOctothorpe() const
{
  // Make tools that use UNIX-style '/' paths also support '\' escaping.
  return this->ForceUnixPaths;
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

  // write each target's progress.make this loop is done twice. Basically the
  // Generate pass counts all the actions, the first loop below determines
  // how many actions have progress updates for each target and writes to
  // correct variable values for everything except the all targets. The
  // second loop actually writes out correct values for the all targets as
  // well. This is because the all targets require more information that is
  // computed in the first loop.
  unsigned long current = 0;
  for (auto& pmi : this->ProgressMap) {
    pmi.second.WriteProgressVariables(total, current);
  }
  for (const auto& lg : this->LocalGenerators) {
    std::string markFileName =
      cmStrCat(lg->GetCurrentBinaryDirectory(), "/CMakeFiles/progress.marks");
    cmGeneratedFileStream markFile(markFileName);
    markFile << this->CountProgressMarksInAll(*lg) << "\n";
  }

  // write the main makefile
  this->WriteMainMakefile2();
  this->WriteMainCMakefile();

  if (this->CommandDatabase) {
    *this->CommandDatabase << "\n]";
    this->CommandDatabase.reset();
  }
}

void cmGlobalUnixMakefileGenerator3::AddCXXCompileCommand(
  const std::string& sourceFile, const std::string& workingDirectory,
  const std::string& compileCommand)
{
  if (!this->CommandDatabase) {
    std::string commandDatabaseName =
      this->GetCMakeInstance()->GetHomeOutputDirectory() +
      "/compile_commands.json";
    this->CommandDatabase =
      cm::make_unique<cmGeneratedFileStream>(commandDatabaseName);
    *this->CommandDatabase << "[\n";
  } else {
    *this->CommandDatabase << ",\n";
  }
  *this->CommandDatabase << "{\n"
                         << R"(  "directory": ")"
                         << cmGlobalGenerator::EscapeJSON(workingDirectory)
                         << "\",\n"
                         << R"(  "command": ")"
                         << cmGlobalGenerator::EscapeJSON(compileCommand)
                         << "\",\n"
                         << R"(  "file": ")"
                         << cmGlobalGenerator::EscapeJSON(sourceFile)
                         << "\"\n}";
}

void cmGlobalUnixMakefileGenerator3::WriteMainMakefile2()
{
  // Open the output file.  This should not be copy-if-different
  // because the check-build-system step compares the makefile time to
  // see if the build system must be regenerated.
  std::string makefileName =
    cmStrCat(this->GetCMakeInstance()->GetHomeOutputDirectory(),
             "/CMakeFiles/Makefile2");
  cmGeneratedFileStream makefileStream(makefileName, false,
                                       this->GetMakefileEncoding());
  if (!makefileStream) {
    return;
  }

  // The global dependency graph is expressed via the root local generator.
  auto& rootLG = cm::static_reference_cast<cmLocalUnixMakefileGenerator3>(
    this->LocalGenerators[0]);

  // Write the do not edit header.
  rootLG.WriteDisclaimer(makefileStream);

  // Write the main entry point target.  This must be the VERY first
  // target so that make with no arguments will run it.
  // Just depend on the all target to drive the build.
  std::vector<std::string> depends;
  std::vector<std::string> no_commands;
  depends.emplace_back("all");

  // Write the rule.
  rootLG.WriteMakeRule(makefileStream,
                       "Default target executed when no arguments are "
                       "given to make.",
                       "default_target", depends, no_commands, true);

  depends.clear();

  // The all and preinstall rules might never have any dependencies
  // added to them.
  if (!this->EmptyRuleHackDepends.empty()) {
    depends.push_back(this->EmptyRuleHackDepends);
  }

  // Write out the "special" stuff
  rootLG.WriteSpecialTargetsTop(makefileStream);

  // Write the directory level rules.
  for (auto const& it : this->ComputeDirectoryTargets()) {
    this->WriteDirectoryRules2(makefileStream, rootLG, it.second);
  }

  // Write the target convenience rules
  for (const auto& localGen : this->LocalGenerators) {
    this->WriteConvenienceRules2(
      makefileStream, rootLG,
      cm::static_reference_cast<cmLocalUnixMakefileGenerator3>(localGen));
  }

  // Write special bottom targets
  rootLG.WriteSpecialTargetsBottom(makefileStream);
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
    cmStrCat(this->GetCMakeInstance()->GetHomeOutputDirectory(),
             "/CMakeFiles/Makefile.cmake");
  cmGeneratedFileStream cmakefileStream(cmakefileName);
  if (!cmakefileStream) {
    return;
  }

  std::string makefileName =
    cmStrCat(this->GetCMakeInstance()->GetHomeOutputDirectory(), "/Makefile");

  {
    // get a local generator for some useful methods
    auto& lg = cm::static_reference_cast<cmLocalUnixMakefileGenerator3>(
      this->LocalGenerators[0]);

    // Write the do not edit header.
    lg.WriteDisclaimer(cmakefileStream);
  }

  // Save the generator name
  cmakefileStream << "# The generator used is:\n"
                  << "set(CMAKE_DEPENDS_GENERATOR \"" << this->GetName()
                  << "\")\n\n";

  // for each cmMakefile get its list of dependencies
  std::vector<std::string> lfiles;
  for (const auto& localGen : this->LocalGenerators) {
    // Get the list of files contributing to this generation step.
    cm::append(lfiles, localGen->GetMakefile()->GetListFiles());
  }

  cmake* cm = this->GetCMakeInstance();
  if (cm->DoWriteGlobVerifyTarget()) {
    lfiles.push_back(cm->GetGlobVerifyScript());
    lfiles.push_back(cm->GetGlobVerifyStamp());
  }

  // Sort the list and remove duplicates.
  std::sort(lfiles.begin(), lfiles.end(), std::less<std::string>());
#if !defined(__VMS) // The Compaq STL on VMS crashes, so accept duplicates.
  auto new_end = std::unique(lfiles.begin(), lfiles.end());
  lfiles.erase(new_end, lfiles.end());
#endif

  {
    // reset lg to the first makefile
    const auto& lg = cm::static_reference_cast<cmLocalUnixMakefileGenerator3>(
      this->LocalGenerators[0]);

    // Save the list to the cmake file.
    cmakefileStream
      << "# The top level Makefile was generated from the following files:\n"
      << "set(CMAKE_MAKEFILE_DEPENDS\n"
      << "  \"CMakeCache.txt\"\n";
    for (std::string const& f : lfiles) {
      cmakefileStream << "  \"" << lg.MaybeRelativeToCurBinDir(f) << "\"\n";
    }
    cmakefileStream << "  )\n\n";

    // Build the path to the cache check file.
    std::string check =
      cmStrCat(this->GetCMakeInstance()->GetHomeOutputDirectory(),
               "/CMakeFiles/cmake.check_cache");

    // Set the corresponding makefile in the cmake file.
    cmakefileStream << "# The corresponding makefile is:\n"
                    << "set(CMAKE_MAKEFILE_OUTPUTS\n"
                    << "  \"" << lg.MaybeRelativeToCurBinDir(makefileName)
                    << "\"\n"
                    << "  \"" << lg.MaybeRelativeToCurBinDir(check) << "\"\n";
    cmakefileStream << "  )\n\n";

    // CMake must rerun if a byproduct is missing.
    cmakefileStream << "# Byproducts of CMake generate step:\n"
                    << "set(CMAKE_MAKEFILE_PRODUCTS\n";

    // add in any byproducts and all the directory information files
    std::string tmpStr;
    for (const auto& localGen : this->LocalGenerators) {
      for (std::string const& outfile :
           localGen->GetMakefile()->GetOutputFiles()) {
        cmakefileStream << "  \"" << lg.MaybeRelativeToTopBinDir(outfile)
                        << "\"\n";
      }
      tmpStr = cmStrCat(localGen->GetCurrentBinaryDirectory(),
                        "/CMakeFiles/CMakeDirectoryInformation.cmake");
      cmakefileStream << "  \"" << localGen->MaybeRelativeToTopBinDir(tmpStr)
                      << "\"\n";
    }
    cmakefileStream << "  )\n\n";
  }

  this->WriteMainCMakefileLanguageRules(cmakefileStream,
                                        this->LocalGenerators);
}

void cmGlobalUnixMakefileGenerator3::WriteMainCMakefileLanguageRules(
  cmGeneratedFileStream& cmakefileStream,
  std::vector<std::unique_ptr<cmLocalGenerator>>& lGenerators)
{
  // now list all the target info files
  cmakefileStream << "# Dependency information for all targets:\n";
  cmakefileStream << "set(CMAKE_DEPEND_INFO_FILES\n";
  for (const auto& lGenerator : lGenerators) {
    const auto& lg =
      cm::static_reference_cast<cmLocalUnixMakefileGenerator3>(lGenerator);
    // for all of out targets
    for (const auto& tgt : lg.GetGeneratorTargets()) {
      if (tgt->IsInBuildSystem() &&
          tgt->GetType() != cmStateEnums::GLOBAL_TARGET) {
        std::string tname = cmStrCat(lg.GetRelativeTargetDirectory(tgt.get()),
                                     "/DependInfo.cmake");
        cmSystemTools::ConvertToUnixSlashes(tname);
        cmakefileStream << "  \"" << tname << "\"\n";
      }
    }
  }
  cmakefileStream << "  )\n";
}

void cmGlobalUnixMakefileGenerator3::WriteDirectoryRule2(
  std::ostream& ruleFileStream, cmLocalUnixMakefileGenerator3& rootLG,
  DirectoryTarget const& dt, const char* pass, bool check_all,
  bool check_relink, std::vector<std::string> const& commands)
{
  auto* lg = static_cast<cmLocalUnixMakefileGenerator3*>(dt.LG);
  std::string makeTarget =
    cmStrCat(lg->GetCurrentBinaryDirectory(), '/', pass);

  // The directory-level rule should depend on the target-level rules
  // for all targets in the directory.
  std::vector<std::string> depends;
  for (DirectoryTarget::Target const& t : dt.Targets) {
    // Add this to the list of depends rules in this directory.
    if ((!check_all || t.ExcludedFromAllInConfigs.empty()) &&
        (!check_relink ||
         t.GT->NeedRelinkBeforeInstall(lg->GetConfigName()))) {
      // The target may be from a different directory; use its local gen.
      auto const* tlg = static_cast<cmLocalUnixMakefileGenerator3 const*>(
        t.GT->GetLocalGenerator());
      std::string tname =
        cmStrCat(tlg->GetRelativeTargetDirectory(t.GT), '/', pass);
      depends.push_back(std::move(tname));
    }
  }

  // The directory-level rule should depend on the directory-level
  // rules of the subdirectories.
  for (DirectoryTarget::Dir const& d : dt.Children) {
    if (check_all && d.ExcludeFromAll) {
      continue;
    }
    std::string subdir = cmStrCat(d.Path, '/', pass);
    depends.push_back(std::move(subdir));
  }

  // Work-around for makes that drop rules that have no dependencies
  // or commands.
  if (depends.empty() && !this->EmptyRuleHackDepends.empty()) {
    depends.push_back(this->EmptyRuleHackDepends);
  }

  // Write the rule.
  std::string doc;
  if (lg->IsRootMakefile()) {
    doc = cmStrCat("The main recursive \"", pass, "\" target.");
  } else {
    doc = cmStrCat("Recursive \"", pass, "\" directory target.");
  }

  rootLG.WriteMakeRule(ruleFileStream, doc.c_str(), makeTarget, depends,
                       commands, true);
}

void cmGlobalUnixMakefileGenerator3::WriteDirectoryRules2(
  std::ostream& ruleFileStream, cmLocalUnixMakefileGenerator3& rootLG,
  DirectoryTarget const& dt)
{
  auto* lg = static_cast<cmLocalUnixMakefileGenerator3*>(dt.LG);
  // Begin the directory-level rules section.
  {
    std::string dir = cmSystemTools::ConvertToOutputPath(
      rootLG.MaybeRelativeToTopBinDir(lg->GetCurrentBinaryDirectory()));
    rootLG.WriteDivider(ruleFileStream);
    if (lg->IsRootMakefile()) {
      ruleFileStream << "# Directory level rules for the build root directory";
    } else {
      ruleFileStream << "# Directory level rules for directory " << dir;
    }
    ruleFileStream << "\n\n";
  }

  // Write directory-level rules for "all".
  this->WriteDirectoryRule2(ruleFileStream, rootLG, dt, "all", true, false);

  // Write directory-level rules for "preinstall".
  this->WriteDirectoryRule2(ruleFileStream, rootLG, dt, "preinstall", true,
                            true);

  // Write directory-level rules for "clean".
  {
    std::vector<std::string> cmds;
    lg->AppendDirectoryCleanCommand(cmds);
    this->WriteDirectoryRule2(ruleFileStream, rootLG, dt, "clean", false,
                              false, cmds);
  }
}

namespace {
std::string ConvertToMakefilePathForUnix(std::string const& path)
{
  std::string result;
  result.reserve(path.size());
  for (char c : path) {
    switch (c) {
      case '=':
        // We provide 'EQUALS = =' to encode '=' in a non-assignment case.
        result.append("$(EQUALS)");
        break;
      case '$':
        result.append("$$");
        break;
      case '\\':
      case ' ':
      case '#':
        result.push_back('\\');
        CM_FALLTHROUGH;
      default:
        result.push_back(c);
        break;
    }
  }
  return result;
}

#if defined(_WIN32) && !defined(__CYGWIN__)
std::string ConvertToMakefilePathForWindows(std::string const& path)
{
  bool const quote = path.find_first_of(" #") != std::string::npos;
  std::string result;
  result.reserve(path.size() + (quote ? 2 : 0));
  if (quote) {
    result.push_back('"');
  }
  for (char c : path) {
    switch (c) {
      case '=':
        // We provide 'EQUALS = =' to encode '=' in a non-assignment case.
        result.append("$(EQUALS)");
        break;
      case '$':
        result.append("$$");
        break;
      case '/':
        result.push_back('\\');
        break;
      default:
        result.push_back(c);
        break;
    }
  }
  if (quote) {
    result.push_back('"');
  }
  return result;
}
#endif
}

std::string cmGlobalUnixMakefileGenerator3::ConvertToMakefilePath(
  std::string const& path) const
{
#if defined(_WIN32) && !defined(__CYGWIN__)
  if (!this->ForceUnixPaths) {
    return ConvertToMakefilePathForWindows(path);
  }
#endif
  return ConvertToMakefilePathForUnix(path);
}

std::vector<cmGlobalGenerator::GeneratedMakeCommand>
cmGlobalUnixMakefileGenerator3::GenerateBuildCommand(
  const std::string& makeProgram, const std::string& /*projectName*/,
  const std::string& /*projectDir*/,
  std::vector<std::string> const& targetNames, const std::string& /*config*/,
  int jobs, bool verbose, const cmBuildOptions& buildOptions,
  std::vector<std::string> const& makeOptions)
{
  GeneratedMakeCommand makeCommand;

  // Make it possible to set verbosity also from command line
  if (verbose) {
    makeCommand.Add(cmSystemTools::GetCMakeCommand());
    makeCommand.Add("-E");
    makeCommand.Add("env");
    makeCommand.Add("VERBOSE=1");
  }
  makeCommand.Add(this->SelectMakeProgram(makeProgram));

  // Explicitly tell the make tool to use the Makefile written by
  // cmLocalUnixMakefileGenerator3::WriteLocalMakefile
  makeCommand.Add("-f");
  makeCommand.Add("Makefile");

  if (jobs != cmake::NO_BUILD_PARALLEL_LEVEL) {
    if (jobs == cmake::DEFAULT_BUILD_PARALLEL_LEVEL) {
      makeCommand.Add("-j");
    } else {
      makeCommand.Add("-j" + std::to_string(jobs));
    }
  }

  makeCommand.Add(makeOptions.begin(), makeOptions.end());
  for (auto tname : targetNames) {
    if (!tname.empty()) {
      if (buildOptions.Fast) {
        tname += "/fast";
      }
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
  for (const auto& localGen : this->LocalGenerators) {
    auto& lg =
      cm::static_reference_cast<cmLocalUnixMakefileGenerator3>(localGen);
    // for each target Generate the rule files for each target.
    for (const auto& gtarget : lg.GetGeneratorTargets()) {
      // Don't emit the same rule twice (e.g. two targets with the same
      // simple name)
      std::string name = gtarget->GetName();
      if (!name.empty() && emitted.insert(name).second &&
          // Handle user targets here.  Global targets are handled in
          // the local generator on a per-directory basis.
          (gtarget->IsInBuildSystem() &&
           gtarget->GetType() != cmStateEnums::GLOBAL_TARGET)) {
        // Add a rule to build the target by name.
        lg.WriteDivider(ruleFileStream);
        ruleFileStream << "# Target rules for targets named " << name
                       << "\n\n";

        // Write the rule.
        commands.clear();
        std::string tmp = "CMakeFiles/Makefile2";
        commands.push_back(lg.GetRecursiveMakeCall(tmp, name));
        depends.clear();
        if (regenerate) {
          depends.emplace_back("cmake_check_build_system");
        }
        lg.WriteMakeRule(ruleFileStream, "Build rule for target.", name,
                         depends, commands, true);

        // Add a fast rule to build the target
        std::string localName = lg.GetRelativeTargetDirectory(gtarget.get());
        std::string makefileName;
        makefileName = cmStrCat(localName, "/build.make");
        depends.clear();
        commands.clear();
        std::string makeTargetName = cmStrCat(localName, "/build");
        localName = cmStrCat(name, "/fast");
        commands.push_back(
          lg.GetRecursiveMakeCall(makefileName, makeTargetName));
        lg.WriteMakeRule(ruleFileStream, "fast build rule for target.",
                         localName, depends, commands, true);

        // Add a local name for the rule to relink the target before
        // installation.
        if (gtarget->NeedRelinkBeforeInstall(lg.GetConfigName())) {
          makeTargetName = cmStrCat(
            lg.GetRelativeTargetDirectory(gtarget.get()), "/preinstall");
          localName = cmStrCat(name, "/preinstall");
          depends.clear();
          commands.clear();
          commands.push_back(
            lg.GetRecursiveMakeCall(makefileName, makeTargetName));
          lg.WriteMakeRule(ruleFileStream,
                           "Manual pre-install relink rule for target.",
                           localName, depends, commands, true);
        }
      }
    }
  }
}

void cmGlobalUnixMakefileGenerator3::WriteConvenienceRules2(
  std::ostream& ruleFileStream, cmLocalUnixMakefileGenerator3& rootLG,
  cmLocalUnixMakefileGenerator3& lg)
{
  std::vector<std::string> depends;
  std::vector<std::string> commands;
  std::string localName;
  std::string makeTargetName;

  bool regenerate = !this->GlobalSettingIsOn("CMAKE_SUPPRESS_REGENERATION");
  if (regenerate) {
    depends.emplace_back("cmake_check_build_system");
  }

  // for each target Generate the rule files for each target.
  for (const auto& gtarget : lg.GetGeneratorTargets()) {
    std::string name = gtarget->GetName();
    if (!name.empty() &&
        (gtarget->IsInBuildSystem() &&
         gtarget->GetType() != cmStateEnums::GLOBAL_TARGET)) {
      std::string makefileName;
      // Add a rule to build the target by name.
      localName = lg.GetRelativeTargetDirectory(gtarget.get());
      makefileName = cmStrCat(localName, "/build.make");

      lg.WriteDivider(ruleFileStream);
      ruleFileStream << "# Target rules for target " << localName << "\n\n";

      commands.clear();
      makeTargetName = cmStrCat(localName, "/depend");
      commands.push_back(
        lg.GetRecursiveMakeCall(makefileName, makeTargetName));

      makeTargetName = cmStrCat(localName, "/build");
      commands.push_back(
        lg.GetRecursiveMakeCall(makefileName, makeTargetName));

      // Write the rule.
      localName += "/all";
      depends.clear();

      cmLocalUnixMakefileGenerator3::EchoProgress progress;
      progress.Dir = cmStrCat(lg.GetBinaryDirectory(), "/CMakeFiles");
      {
        std::ostringstream progressArg;
        const char* sep = "";
        for (unsigned long progFile : this->ProgressMap[gtarget.get()].Marks) {
          progressArg << sep << progFile;
          sep = ",";
        }
        progress.Arg = progressArg.str();
      }

      bool targetMessages = true;
      if (cmValue tgtMsg =
            this->GetCMakeInstance()->GetState()->GetGlobalProperty(
              "TARGET_MESSAGES")) {
        targetMessages = cmIsOn(*tgtMsg);
      }

      if (targetMessages) {
        lg.AppendEcho(commands, "Built target " + name,
                      cmLocalUnixMakefileGenerator3::EchoNormal, &progress);
      }

      this->AppendGlobalTargetDepends(depends, gtarget.get());
      rootLG.WriteMakeRule(ruleFileStream, "All Build rule for target.",
                           localName, depends, commands, true);

      // Write the rule.
      commands.clear();

      {
        // TODO: Convert the total progress count to a make variable.
        std::ostringstream progCmd;
        progCmd << "$(CMAKE_COMMAND) -E cmake_progress_start ";
        // # in target
        progCmd << lg.ConvertToOutputFormat(progress.Dir,
                                            cmOutputConverter::SHELL);
        //
        std::set<cmGeneratorTarget const*> emitted;
        progCmd << " "
                << this->CountProgressMarksInTarget(gtarget.get(), emitted);
        commands.push_back(progCmd.str());
      }
      std::string tmp = "CMakeFiles/Makefile2";
      commands.push_back(lg.GetRecursiveMakeCall(tmp, localName));
      {
        std::ostringstream progCmd;
        progCmd << "$(CMAKE_COMMAND) -E cmake_progress_start "; // # 0
        progCmd << lg.ConvertToOutputFormat(progress.Dir,
                                            cmOutputConverter::SHELL);
        progCmd << " 0";
        commands.push_back(progCmd.str());
      }
      depends.clear();
      if (regenerate) {
        depends.emplace_back("cmake_check_build_system");
      }
      localName =
        cmStrCat(lg.GetRelativeTargetDirectory(gtarget.get()), "/rule");
      rootLG.WriteMakeRule(ruleFileStream,
                           "Build rule for subdir invocation for target.",
                           localName, depends, commands, true);

      // Add a target with the canonical name (no prefix, suffix or path).
      commands.clear();
      depends.clear();
      depends.push_back(localName);
      rootLG.WriteMakeRule(ruleFileStream, "Convenience name for target.",
                           name, depends, commands, true);

      // Add rules to prepare the target for installation.
      if (gtarget->NeedRelinkBeforeInstall(lg.GetConfigName())) {
        localName = cmStrCat(lg.GetRelativeTargetDirectory(gtarget.get()),
                             "/preinstall");
        depends.clear();
        commands.clear();
        commands.push_back(lg.GetRecursiveMakeCall(makefileName, localName));
        rootLG.WriteMakeRule(ruleFileStream,
                             "Pre-install relink rule for target.", localName,
                             depends, commands, true);
      }

      // add the clean rule
      localName = lg.GetRelativeTargetDirectory(gtarget.get());
      makeTargetName = cmStrCat(localName, "/clean");
      depends.clear();
      commands.clear();
      commands.push_back(
        lg.GetRecursiveMakeCall(makefileName, makeTargetName));
      rootLG.WriteMakeRule(ruleFileStream, "clean rule for target.",
                           makeTargetName, depends, commands, true);
      commands.clear();
    }
  }
}

// Build a map that contains the set of targets used by each local
// generator directory level.
void cmGlobalUnixMakefileGenerator3::InitializeProgressMarks()
{
  this->DirectoryTargetsMap.clear();
  // Loop over all targets in all local generators.
  for (const auto& lg : this->LocalGenerators) {
    for (const auto& gt : lg->GetGeneratorTargets()) {
      cmLocalGenerator* tlg = gt->GetLocalGenerator();

      if (!gt->IsInBuildSystem() || this->IsExcluded(lg.get(), gt.get())) {
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
        targetSet.insert(gt.get());

        // Add dependencies of the included target.  An excluded
        // target may still be included if it is a dependency of a
        // non-excluded target.
        for (cmTargetDepend const& tgtdep :
             this->GetTargetDirectDepends(gt.get())) {
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
    for (cmTargetDepend const& depend : this->GetTargetDirectDepends(target)) {
      if (!depend->IsInBuildSystem()) {
        continue;
      }
      count += this->CountProgressMarksInTarget(depend, emitted);
    }
  }
  return count;
}

size_t cmGlobalUnixMakefileGenerator3::CountProgressMarksInAll(
  const cmLocalGenerator& lg)
{
  size_t count = 0;
  std::set<cmGeneratorTarget const*> emitted;
  for (cmGeneratorTarget const* target :
       this->DirectoryTargetsMap[lg.GetStateSnapshot()]) {
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
  for (cmTargetDepend const& i : this->GetTargetDirectDepends(target)) {
    // Create the target-level dependency.
    cmGeneratorTarget const* dep = i;
    if (!dep->IsInBuildSystem()) {
      continue;
    }
    cmLocalUnixMakefileGenerator3* lg3 =
      static_cast<cmLocalUnixMakefileGenerator3*>(dep->GetLocalGenerator());
    std::string tgtName = cmStrCat(
      lg3->GetRelativeTargetDirectory(const_cast<cmGeneratorTarget*>(dep)),
      "/all");
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
  std::set<std::string> utility_targets;
  std::set<std::string> globals_targets;
  std::set<std::string> project_targets;

  // for each local generator
  for (const auto& localGen : this->LocalGenerators) {
    const auto& lg2 =
      cm::static_reference_cast<cmLocalUnixMakefileGenerator3>(localGen);
    // for the passed in makefile or if this is the top Makefile wripte out
    // the targets
    if (&lg2 == lg || lg->IsRootMakefile()) {
      // for each target Generate the rule files for each target.
      for (const auto& target : lg2.GetGeneratorTargets()) {
        cmStateEnums::TargetType type = target->GetType();
        if ((type == cmStateEnums::EXECUTABLE) ||
            (type == cmStateEnums::STATIC_LIBRARY) ||
            (type == cmStateEnums::SHARED_LIBRARY) ||
            (type == cmStateEnums::MODULE_LIBRARY) ||
            (type == cmStateEnums::OBJECT_LIBRARY) ||
            (type == cmStateEnums::INTERFACE_LIBRARY &&
             target->IsInBuildSystem())) {
          project_targets.insert(target->GetName());
        } else if (type == cmStateEnums::GLOBAL_TARGET) {
          globals_targets.insert(target->GetName());
        } else if (type == cmStateEnums::UTILITY) {
          utility_targets.insert(target->GetName());
        }
      }
    }
  }

  for (std::string const& name : globals_targets) {
    path = cmStrCat("... ", name);
    lg->AppendEcho(commands, path);
  }
  for (std::string const& name : utility_targets) {
    path = cmStrCat("... ", name);
    lg->AppendEcho(commands, path);
  }
  for (std::string const& name : project_targets) {
    path = cmStrCat("... ", name);
    lg->AppendEcho(commands, path);
  }

  for (std::string const& o : lg->GetLocalHelp()) {
    path = cmStrCat("... ", o);
    lg->AppendEcho(commands, path);
  }
  lg->WriteMakeRule(ruleFileStream, "Help Target", "help", no_depends,
                    commands, true);
  ruleFileStream << "\n\n";
}
