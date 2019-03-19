/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmLocalNinjaGenerator.h"

#include <algorithm>
#include <assert.h>
#include <iterator>
#include <memory> // IWYU pragma: keep
#include <sstream>
#include <stdio.h>
#include <utility>

#include "cmCryptoHash.h"
#include "cmCustomCommand.h"
#include "cmCustomCommandGenerator.h"
#include "cmGeneratedFileStream.h"
#include "cmGeneratorTarget.h"
#include "cmGlobalGenerator.h"
#include "cmGlobalNinjaGenerator.h"
#include "cmLocalGenerator.h"
#include "cmMakefile.h"
#include "cmNinjaTargetGenerator.h"
#include "cmRulePlaceholderExpander.h"
#include "cmSourceFile.h"
#include "cmState.h"
#include "cmStateTypes.h"
#include "cmSystemTools.h"
#include "cmake.h"
#include "cmsys/FStream.hxx"

cmLocalNinjaGenerator::cmLocalNinjaGenerator(cmGlobalGenerator* gg,
                                             cmMakefile* mf)
  : cmLocalCommonGenerator(gg, mf, mf->GetState()->GetBinaryDirectory())
  , HomeRelativeOutputPath("")
{
}

// Virtual public methods.

cmRulePlaceholderExpander*
cmLocalNinjaGenerator::CreateRulePlaceholderExpander() const
{
  cmRulePlaceholderExpander* ret =
    this->cmLocalGenerator::CreateRulePlaceholderExpander();
  ret->SetTargetImpLib("$TARGET_IMPLIB");
  return ret;
}

cmLocalNinjaGenerator::~cmLocalNinjaGenerator() = default;

void cmLocalNinjaGenerator::Generate()
{
  // Compute the path to use when referencing the current output
  // directory from the top output directory.
  this->HomeRelativeOutputPath = this->MaybeConvertToRelativePath(
    this->GetBinaryDirectory(), this->GetCurrentBinaryDirectory());
  if (this->HomeRelativeOutputPath == ".") {
    this->HomeRelativeOutputPath.clear();
  }

  this->WriteProcessedMakefile(this->GetBuildFileStream());
#ifdef NINJA_GEN_VERBOSE_FILES
  this->WriteProcessedMakefile(this->GetRulesFileStream());
#endif

  // We do that only once for the top CMakeLists.txt file.
  if (this->IsRootMakefile()) {
    this->WriteBuildFileTop();

    this->WritePools(this->GetRulesFileStream());

    const std::string showIncludesPrefix =
      this->GetMakefile()->GetSafeDefinition("CMAKE_CL_SHOWINCLUDES_PREFIX");
    if (!showIncludesPrefix.empty()) {
      cmGlobalNinjaGenerator::WriteComment(this->GetRulesFileStream(),
                                           "localized /showIncludes string");
      this->GetRulesFileStream()
        << "msvc_deps_prefix = " << showIncludesPrefix << "\n\n";
    }
  }

  const std::vector<cmGeneratorTarget*>& targets = this->GetGeneratorTargets();
  for (cmGeneratorTarget* target : targets) {
    if (target->GetType() == cmStateEnums::INTERFACE_LIBRARY) {
      continue;
    }
    cmNinjaTargetGenerator* tg = cmNinjaTargetGenerator::New(target);
    if (tg) {
      tg->Generate();
      // Add the target to "all" if required.
      if (!this->GetGlobalNinjaGenerator()->IsExcluded(target)) {
        this->GetGlobalNinjaGenerator()->AddDependencyToAll(target);
      }
      delete tg;
    }
  }

  this->WriteCustomCommandBuildStatements();
}

// TODO: Picked up from cmLocalUnixMakefileGenerator3.  Refactor it.
std::string cmLocalNinjaGenerator::GetTargetDirectory(
  cmGeneratorTarget const* target) const
{
  std::string dir = "CMakeFiles/";
  dir += target->GetName();
#if defined(__VMS)
  dir += "_dir";
#else
  dir += ".dir";
#endif
  return dir;
}

// Non-virtual public methods.

const cmGlobalNinjaGenerator* cmLocalNinjaGenerator::GetGlobalNinjaGenerator()
  const
{
  return static_cast<const cmGlobalNinjaGenerator*>(
    this->GetGlobalGenerator());
}

cmGlobalNinjaGenerator* cmLocalNinjaGenerator::GetGlobalNinjaGenerator()
{
  return static_cast<cmGlobalNinjaGenerator*>(this->GetGlobalGenerator());
}

// Virtual protected methods.

std::string cmLocalNinjaGenerator::ConvertToIncludeReference(
  std::string const& path, cmOutputConverter::OutputFormat format,
  bool forceFullPaths)
{
  if (forceFullPaths) {
    return this->ConvertToOutputFormat(cmSystemTools::CollapseFullPath(path),
                                       format);
  }
  return this->ConvertToOutputFormat(
    this->MaybeConvertToRelativePath(this->GetBinaryDirectory(), path),
    format);
}

// Private methods.

cmGeneratedFileStream& cmLocalNinjaGenerator::GetBuildFileStream() const
{
  return *this->GetGlobalNinjaGenerator()->GetBuildFileStream();
}

cmGeneratedFileStream& cmLocalNinjaGenerator::GetRulesFileStream() const
{
  return *this->GetGlobalNinjaGenerator()->GetRulesFileStream();
}

const cmake* cmLocalNinjaGenerator::GetCMakeInstance() const
{
  return this->GetGlobalGenerator()->GetCMakeInstance();
}

cmake* cmLocalNinjaGenerator::GetCMakeInstance()
{
  return this->GetGlobalGenerator()->GetCMakeInstance();
}

void cmLocalNinjaGenerator::WriteBuildFileTop()
{
  // For the build file.
  this->WriteProjectHeader(this->GetBuildFileStream());
  this->WriteNinjaRequiredVersion(this->GetBuildFileStream());
  this->WriteNinjaFilesInclusion(this->GetBuildFileStream());

  // For the rule file.
  this->WriteProjectHeader(this->GetRulesFileStream());
}

void cmLocalNinjaGenerator::WriteProjectHeader(std::ostream& os)
{
  cmGlobalNinjaGenerator::WriteDivider(os);
  os << "# Project: " << this->GetProjectName() << std::endl
     << "# Configuration: " << this->ConfigName << std::endl;
  cmGlobalNinjaGenerator::WriteDivider(os);
}

void cmLocalNinjaGenerator::WriteNinjaRequiredVersion(std::ostream& os)
{
  // Default required version
  std::string requiredVersion = cmGlobalNinjaGenerator::RequiredNinjaVersion();

  // Ninja generator uses the 'console' pool if available (>= 1.5)
  if (this->GetGlobalNinjaGenerator()->SupportsConsolePool()) {
    requiredVersion =
      cmGlobalNinjaGenerator::RequiredNinjaVersionForConsolePool();
  }

  // The Ninja generator writes rules which require support for restat
  // when rebuilding build.ninja manifest (>= 1.8)
  if (this->GetGlobalNinjaGenerator()->SupportsManifestRestat() &&
      this->GetCMakeInstance()->DoWriteGlobVerifyTarget() &&
      !this->GetGlobalNinjaGenerator()->GlobalSettingIsOn(
        "CMAKE_SUPPRESS_REGENERATION")) {
    requiredVersion =
      cmGlobalNinjaGenerator::RequiredNinjaVersionForManifestRestat();
  }

  cmGlobalNinjaGenerator::WriteComment(
    os, "Minimal version of Ninja required by this file");
  os << "ninja_required_version = " << requiredVersion << std::endl
     << std::endl;
}

void cmLocalNinjaGenerator::WritePools(std::ostream& os)
{
  cmGlobalNinjaGenerator::WriteDivider(os);

  const char* jobpools =
    this->GetCMakeInstance()->GetState()->GetGlobalProperty("JOB_POOLS");
  if (!jobpools) {
    jobpools = this->GetMakefile()->GetDefinition("CMAKE_JOB_POOLS");
  }
  if (jobpools) {
    cmGlobalNinjaGenerator::WriteComment(
      os, "Pools defined by global property JOB_POOLS");
    std::vector<std::string> pools;
    cmSystemTools::ExpandListArgument(jobpools, pools);
    for (std::string const& pool : pools) {
      const std::string::size_type eq = pool.find('=');
      unsigned int jobs;
      if (eq != std::string::npos &&
          sscanf(pool.c_str() + eq, "=%u", &jobs) == 1) {
        os << "pool " << pool.substr(0, eq) << std::endl;
        os << "  depth = " << jobs << std::endl;
        os << std::endl;
      } else {
        cmSystemTools::Error("Invalid pool defined by property 'JOB_POOLS': " +
                             pool);
      }
    }
  }
}

void cmLocalNinjaGenerator::WriteNinjaFilesInclusion(std::ostream& os)
{
  cmGlobalNinjaGenerator::WriteDivider(os);
  os << "# Include auxiliary files.\n"
     << "\n";
  cmGlobalNinjaGenerator* ng = this->GetGlobalNinjaGenerator();
  std::string const ninjaRulesFile =
    ng->NinjaOutputPath(cmGlobalNinjaGenerator::NINJA_RULES_FILE);
  std::string const rulesFilePath = ng->EncodePath(ninjaRulesFile);
  cmGlobalNinjaGenerator::WriteInclude(os, rulesFilePath,
                                       "Include rules file.");
  os << "\n";
}

void cmLocalNinjaGenerator::WriteProcessedMakefile(std::ostream& os)
{
  cmGlobalNinjaGenerator::WriteDivider(os);
  os << "# Write statements declared in CMakeLists.txt:" << std::endl
     << "# " << this->Makefile->GetDefinition("CMAKE_CURRENT_LIST_FILE")
     << std::endl;
  if (this->IsRootMakefile()) {
    os << "# Which is the root file." << std::endl;
  }
  cmGlobalNinjaGenerator::WriteDivider(os);
  os << std::endl;
}

void cmLocalNinjaGenerator::AppendTargetOutputs(cmGeneratorTarget* target,
                                                cmNinjaDeps& outputs)
{
  this->GetGlobalNinjaGenerator()->AppendTargetOutputs(target, outputs);
}

void cmLocalNinjaGenerator::AppendTargetDepends(cmGeneratorTarget* target,
                                                cmNinjaDeps& outputs,
                                                cmNinjaTargetDepends depends)
{
  this->GetGlobalNinjaGenerator()->AppendTargetDepends(target, outputs,
                                                       depends);
}

void cmLocalNinjaGenerator::AppendCustomCommandDeps(
  cmCustomCommandGenerator const& ccg, cmNinjaDeps& ninjaDeps)
{
  const std::vector<std::string>& deps = ccg.GetDepends();
  for (std::string const& i : deps) {
    std::string dep;
    if (this->GetRealDependency(i, this->GetConfigName(), dep)) {
      ninjaDeps.push_back(
        this->GetGlobalNinjaGenerator()->ConvertToNinjaPath(dep));
    }
  }
}

std::string cmLocalNinjaGenerator::WriteCommandScript(
  std::vector<std::string> const& cmdLines, std::string const& customStep,
  cmGeneratorTarget const* target) const
{
  std::string scriptPath;
  if (target) {
    scriptPath = target->GetSupportDirectory();
  } else {
    scriptPath = this->GetCurrentBinaryDirectory();
    scriptPath += "/CMakeFiles";
  }
  cmSystemTools::MakeDirectory(scriptPath);
  scriptPath += '/';
  scriptPath += customStep;
#ifdef _WIN32
  scriptPath += ".bat";
#else
  scriptPath += ".sh";
#endif

  cmsys::ofstream script(scriptPath.c_str());

#ifdef _WIN32
  script << "@echo off\n";
  int line = 1;
#else
  script << "set -e\n\n";
#endif

  for (auto const& i : cmdLines) {
    std::string cmd = i;
    // The command line was built assuming it would be written to
    // the build.ninja file, so it uses '$$' for '$'.  Remove this
    // for the raw shell script.
    cmSystemTools::ReplaceString(cmd, "$$", "$");
#ifdef _WIN32
    script << cmd << " || (set FAIL_LINE=" << ++line << "& goto :ABORT)"
           << '\n';
#else
    script << cmd << '\n';
#endif
  }

#ifdef _WIN32
  script << "goto :EOF\n\n"
            ":ABORT\n"
            "set ERROR_CODE=%ERRORLEVEL%\n"
            "echo Batch file failed at line %FAIL_LINE% "
            "with errorcode %ERRORLEVEL%\n"
            "exit /b %ERROR_CODE%";
#endif

  return scriptPath;
}

std::string cmLocalNinjaGenerator::BuildCommandLine(
  std::vector<std::string> const& cmdLines, std::string const& customStep,
  cmGeneratorTarget const* target) const
{
  // If we have no commands but we need to build a command anyway, use noop.
  // This happens when building a POST_BUILD value for link targets that
  // don't use POST_BUILD.
  if (cmdLines.empty()) {
    return cmGlobalNinjaGenerator::SHELL_NOOP;
  }

  // If this is a custom step then we will have no '$VAR' ninja placeholders.
  // This means we can deal with long command sequences by writing to a script.
  // Do this if the command lines are on the scale of the OS limit.
  if (!customStep.empty()) {
    size_t cmdLinesTotal = 0;
    for (std::string const& cmd : cmdLines) {
      cmdLinesTotal += cmd.length() + 6;
    }
    if (cmdLinesTotal > cmSystemTools::CalculateCommandLineLengthLimit() / 2) {
      std::string const scriptPath =
        this->WriteCommandScript(cmdLines, customStep, target);
      std::string cmd
#ifndef _WIN32
        = "/bin/sh "
#endif
        ;
      cmd += this->ConvertToOutputFormat(
        this->GetGlobalNinjaGenerator()->ConvertToNinjaPath(scriptPath),
        cmOutputConverter::SHELL);

      // Add an unused argument based on script content so that Ninja
      // knows when the command lines change.
      cmd += " ";
      cmCryptoHash hash(cmCryptoHash::AlgoSHA256);
      cmd += hash.HashFile(scriptPath).substr(0, 16);
      return cmd;
    }
  }

  std::ostringstream cmd;
  for (std::vector<std::string>::const_iterator li = cmdLines.begin();
       li != cmdLines.end(); ++li)
#ifdef _WIN32
  {
    if (li != cmdLines.begin()) {
      cmd << " && ";
    } else if (cmdLines.size() > 1) {
      cmd << "cmd.exe /C \"";
    }
    // Put current cmdLine in brackets if it contains "||" because it has
    // higher precedence than "&&" in cmd.exe
    if (li->find("||") != std::string::npos) {
      cmd << "( " << *li << " )";
    } else {
      cmd << *li;
    }
  }
  if (cmdLines.size() > 1) {
    cmd << "\"";
  }
#else
  {
    if (li != cmdLines.begin()) {
      cmd << " && ";
    }
    cmd << *li;
  }
#endif
  return cmd.str();
}

void cmLocalNinjaGenerator::AppendCustomCommandLines(
  cmCustomCommandGenerator const& ccg, std::vector<std::string>& cmdLines)
{
  if (ccg.GetNumberOfCommands() > 0) {
    std::string wd = ccg.GetWorkingDirectory();
    if (wd.empty()) {
      wd = this->GetCurrentBinaryDirectory();
    }

    std::ostringstream cdCmd;
#ifdef _WIN32
    std::string cdStr = "cd /D ";
#else
    std::string cdStr = "cd ";
#endif
    cdCmd << cdStr
          << this->ConvertToOutputFormat(wd, cmOutputConverter::SHELL);
    cmdLines.push_back(cdCmd.str());
  }

  std::string launcher = this->MakeCustomLauncher(ccg);

  for (unsigned i = 0; i != ccg.GetNumberOfCommands(); ++i) {
    cmdLines.push_back(launcher +
                       this->ConvertToOutputFormat(ccg.GetCommand(i),
                                                   cmOutputConverter::SHELL));

    std::string& cmd = cmdLines.back();
    ccg.AppendArguments(i, cmd);
  }
}

void cmLocalNinjaGenerator::WriteCustomCommandBuildStatement(
  cmCustomCommand const* cc, const cmNinjaDeps& orderOnlyDeps)
{
  if (this->GetGlobalNinjaGenerator()->SeenCustomCommand(cc)) {
    return;
  }

  cmCustomCommandGenerator ccg(*cc, this->GetConfigName(), this);

  const std::vector<std::string>& outputs = ccg.GetOutputs();
  const std::vector<std::string>& byproducts = ccg.GetByproducts();
  cmNinjaDeps ninjaOutputs(outputs.size() + byproducts.size()), ninjaDeps;

  bool symbolic = false;
  for (std::string const& output : outputs) {
    if (cmSourceFile* sf = this->Makefile->GetSource(output)) {
      symbolic = sf->GetPropertyAsBool("SYMBOLIC");
      if (symbolic) {
        break;
      }
    }
  }

#if 0
#  error TODO: Once CC in an ExternalProject target must provide the \
    file of each imported target that has an add_dependencies pointing \
    at us.  How to know which ExternalProject step actually provides it?
#endif
  std::transform(outputs.begin(), outputs.end(), ninjaOutputs.begin(),
                 this->GetGlobalNinjaGenerator()->MapToNinjaPath());
  std::transform(byproducts.begin(), byproducts.end(),
                 ninjaOutputs.begin() + outputs.size(),
                 this->GetGlobalNinjaGenerator()->MapToNinjaPath());
  this->AppendCustomCommandDeps(ccg, ninjaDeps);

  for (std::string const& ninjaOutput : ninjaOutputs) {
    this->GetGlobalNinjaGenerator()->SeenCustomCommandOutput(ninjaOutput);
  }

  std::vector<std::string> cmdLines;
  this->AppendCustomCommandLines(ccg, cmdLines);

  if (cmdLines.empty()) {
    this->GetGlobalNinjaGenerator()->WritePhonyBuild(
      this->GetBuildFileStream(),
      "Phony custom command for " + ninjaOutputs[0], ninjaOutputs, ninjaDeps,
      cmNinjaDeps(), orderOnlyDeps, cmNinjaVars());
  } else {
    std::string customStep = cmSystemTools::GetFilenameName(ninjaOutputs[0]);
    // Hash full path to make unique.
    customStep += '-';
    cmCryptoHash hash(cmCryptoHash::AlgoSHA256);
    customStep += hash.HashString(ninjaOutputs[0]).substr(0, 7);

    this->GetGlobalNinjaGenerator()->WriteCustomCommandBuild(
      this->BuildCommandLine(cmdLines, customStep),
      this->ConstructComment(ccg), "Custom command for " + ninjaOutputs[0],
      cc->GetDepfile(), cc->GetUsesTerminal(),
      /*restat*/ !symbolic || !byproducts.empty(), ninjaOutputs, ninjaDeps,
      orderOnlyDeps);
  }
}

void cmLocalNinjaGenerator::AddCustomCommandTarget(cmCustomCommand const* cc,
                                                   cmGeneratorTarget* target)
{
  CustomCommandTargetMap::value_type v(cc, std::set<cmGeneratorTarget*>());
  std::pair<CustomCommandTargetMap::iterator, bool> ins =
    this->CustomCommandTargets.insert(v);
  if (ins.second) {
    this->CustomCommands.push_back(cc);
  }
  ins.first->second.insert(target);
}

void cmLocalNinjaGenerator::WriteCustomCommandBuildStatements()
{
  for (cmCustomCommand const* customCommand : this->CustomCommands) {
    CustomCommandTargetMap::iterator i =
      this->CustomCommandTargets.find(customCommand);
    assert(i != this->CustomCommandTargets.end());

    // A custom command may appear on multiple targets.  However, some build
    // systems exist where the target dependencies on some of the targets are
    // overspecified, leading to a dependency cycle.  If we assume all target
    // dependencies are a superset of the true target dependencies for this
    // custom command, we can take the set intersection of all target
    // dependencies to obtain a correct dependency list.
    //
    // FIXME: This won't work in certain obscure scenarios involving indirect
    // dependencies.
    std::set<cmGeneratorTarget*>::iterator j = i->second.begin();
    assert(j != i->second.end());
    std::vector<std::string> ccTargetDeps;
    this->GetGlobalNinjaGenerator()->AppendTargetDependsClosure(*j,
                                                                ccTargetDeps);
    std::sort(ccTargetDeps.begin(), ccTargetDeps.end());
    ++j;

    for (; j != i->second.end(); ++j) {
      std::vector<std::string> jDeps, depsIntersection;
      this->GetGlobalNinjaGenerator()->AppendTargetDependsClosure(*j, jDeps);
      std::sort(jDeps.begin(), jDeps.end());
      std::set_intersection(ccTargetDeps.begin(), ccTargetDeps.end(),
                            jDeps.begin(), jDeps.end(),
                            std::back_inserter(depsIntersection));
      ccTargetDeps = depsIntersection;
    }

    this->WriteCustomCommandBuildStatement(i->first, ccTargetDeps);
  }
}

std::string cmLocalNinjaGenerator::MakeCustomLauncher(
  cmCustomCommandGenerator const& ccg)
{
  const char* property_value =
    this->Makefile->GetProperty("RULE_LAUNCH_CUSTOM");

  if (!property_value || !*property_value) {
    return std::string();
  }

  // Expand rule variables referenced in the given launcher command.
  cmRulePlaceholderExpander::RuleVariables vars;

  std::string output;
  const std::vector<std::string>& outputs = ccg.GetOutputs();
  if (!outputs.empty()) {
    output = outputs[0];
    if (ccg.GetWorkingDirectory().empty()) {
      output = this->MaybeConvertToRelativePath(
        this->GetCurrentBinaryDirectory(), output);
    }
    output = this->ConvertToOutputFormat(output, cmOutputConverter::SHELL);
  }
  vars.Output = output.c_str();

  std::unique_ptr<cmRulePlaceholderExpander> rulePlaceholderExpander(
    this->CreateRulePlaceholderExpander());

  std::string launcher = property_value;
  rulePlaceholderExpander->ExpandRuleVariables(this, launcher, vars);
  if (!launcher.empty()) {
    launcher += " ";
  }

  return launcher;
}
