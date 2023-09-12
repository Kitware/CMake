/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmLocalNinjaGenerator.h"

#include <algorithm>
#include <cassert>
#include <cstdio>
#include <memory>
#include <sstream>
#include <utility>

#include <cm/unordered_set>
#include <cmext/string_view>

#include "cmsys/FStream.hxx"

#include "cm_codecvt_Encoding.hxx"

#include "cmCryptoHash.h"
#include "cmCustomCommand.h"
#include "cmCustomCommandGenerator.h"
#include "cmGeneratedFileStream.h"
#include "cmGeneratorExpression.h"
#include "cmGeneratorTarget.h"
#include "cmGlobalGenerator.h"
#include "cmGlobalNinjaGenerator.h"
#include "cmList.h"
#include "cmLocalGenerator.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmNinjaTargetGenerator.h"
#include "cmNinjaTypes.h"
#include "cmPolicies.h"
#include "cmRulePlaceholderExpander.h"
#include "cmSourceFile.h"
#include "cmState.h"
#include "cmStateTypes.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmTarget.h"
#include "cmValue.h"
#include "cmake.h"

cmLocalNinjaGenerator::cmLocalNinjaGenerator(cmGlobalGenerator* gg,
                                             cmMakefile* mf)
  : cmLocalCommonGenerator(gg, mf)
{
}

// Virtual public methods.

std::unique_ptr<cmRulePlaceholderExpander>
cmLocalNinjaGenerator::CreateRulePlaceholderExpander() const
{
  auto ret = this->cmLocalGenerator::CreateRulePlaceholderExpander();
  ret->SetTargetImpLib("$TARGET_IMPLIB");
  return std::unique_ptr<cmRulePlaceholderExpander>(std::move(ret));
}

cmLocalNinjaGenerator::~cmLocalNinjaGenerator() = default;

void cmLocalNinjaGenerator::Generate()
{
  // Compute the path to use when referencing the current output
  // directory from the top output directory.
  this->HomeRelativeOutputPath =
    this->MaybeRelativeToTopBinDir(this->GetCurrentBinaryDirectory());
  if (this->HomeRelativeOutputPath == ".") {
    this->HomeRelativeOutputPath.clear();
  }

  if (this->GetGlobalGenerator()->IsMultiConfig()) {
    for (auto const& config : this->GetConfigNames()) {
      this->WriteProcessedMakefile(this->GetImplFileStream(config));
    }
  }
  this->WriteProcessedMakefile(this->GetCommonFileStream());
#ifdef NINJA_GEN_VERBOSE_FILES
  this->WriteProcessedMakefile(this->GetRulesFileStream());
#endif

  // We do that only once for the top CMakeLists.txt file.
  if (this->IsRootMakefile()) {
    this->WriteBuildFileTop();

    this->WritePools(this->GetRulesFileStream());

    const std::string& showIncludesPrefix =
      this->GetMakefile()->GetSafeDefinition("CMAKE_CL_SHOWINCLUDES_PREFIX");
    if (!showIncludesPrefix.empty()) {
      cmGlobalNinjaGenerator::WriteComment(this->GetRulesFileStream(),
                                           "localized /showIncludes string");
      this->GetRulesFileStream() << "msvc_deps_prefix = ";
      // 'cl /showIncludes' encodes output in the console output code page.
      // It may differ from the encoding used for file paths in 'build.ninja'.
      // Ninja matches the showIncludes prefix using its raw byte sequence.
      this->GetRulesFileStream().WriteAltEncoding(
        showIncludesPrefix, cmGeneratedFileStream::Encoding::ConsoleOutput);
      this->GetRulesFileStream() << "\n\n";
    }
  }

  for (const auto& target : this->GetGeneratorTargets()) {
    if (!target->IsInBuildSystem()) {
      continue;
    }
    auto tg = cmNinjaTargetGenerator::New(target.get());
    if (tg) {
      if (target->Target->IsPerConfig()) {
        for (auto const& config : this->GetConfigNames()) {
          tg->Generate(config);
          if (target->GetType() == cmStateEnums::GLOBAL_TARGET &&
              this->GetGlobalGenerator()->IsMultiConfig()) {
            cmNinjaBuild phonyAlias("phony");
            this->GetGlobalNinjaGenerator()->AppendTargetOutputs(
              target.get(), phonyAlias.Outputs, "", DependOnTargetArtifact);
            this->GetGlobalNinjaGenerator()->AppendTargetOutputs(
              target.get(), phonyAlias.ExplicitDeps, config,
              DependOnTargetArtifact);
            this->GetGlobalNinjaGenerator()->WriteBuild(
              *this->GetGlobalNinjaGenerator()->GetConfigFileStream(config),
              phonyAlias);
          }
        }
        if (target->GetType() == cmStateEnums::GLOBAL_TARGET &&
            this->GetGlobalGenerator()->IsMultiConfig()) {
          if (!this->GetGlobalNinjaGenerator()->GetDefaultConfigs().empty()) {
            cmNinjaBuild phonyAlias("phony");
            this->GetGlobalNinjaGenerator()->AppendTargetOutputs(
              target.get(), phonyAlias.Outputs, "", DependOnTargetArtifact);
            for (auto const& config :
                 this->GetGlobalNinjaGenerator()->GetDefaultConfigs()) {
              this->GetGlobalNinjaGenerator()->AppendTargetOutputs(
                target.get(), phonyAlias.ExplicitDeps, config,
                DependOnTargetArtifact);
            }
            this->GetGlobalNinjaGenerator()->WriteBuild(
              *this->GetGlobalNinjaGenerator()->GetDefaultFileStream(),
              phonyAlias);
          }
          cmNinjaBuild phonyAlias("phony");
          this->GetGlobalNinjaGenerator()->AppendTargetOutputs(
            target.get(), phonyAlias.Outputs, "all", DependOnTargetArtifact);
          for (auto const& config : this->GetConfigNames()) {
            this->GetGlobalNinjaGenerator()->AppendTargetOutputs(
              target.get(), phonyAlias.ExplicitDeps, config,
              DependOnTargetArtifact);
          }
          this->GetGlobalNinjaGenerator()->WriteBuild(
            *this->GetGlobalNinjaGenerator()->GetDefaultFileStream(),
            phonyAlias);
        }
      } else {
        tg->Generate("");
      }
    }
  }

  for (auto const& config : this->GetConfigNames()) {
    this->WriteCustomCommandBuildStatements(config);
    this->AdditionalCleanFiles(config);
  }
}

// TODO: Picked up from cmLocalUnixMakefileGenerator3.  Refactor it.
std::string cmLocalNinjaGenerator::GetTargetDirectory(
  cmGeneratorTarget const* target) const
{
  std::string dir = cmStrCat("CMakeFiles/", target->GetName());
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

std::string const& cmLocalNinjaGenerator::GetWorkingDirectory() const
{
  return this->GetState()->GetBinaryDirectory();
}

std::string cmLocalNinjaGenerator::MaybeRelativeToWorkDir(
  std::string const& path) const
{
  return this->GetGlobalNinjaGenerator()->NinjaOutputPath(
    this->MaybeRelativeToTopBinDir(path));
}

std::string cmLocalNinjaGenerator::GetLinkDependencyFile(
  cmGeneratorTarget* target, std::string const& config) const
{
  return cmStrCat(target->GetSupportDirectory(),
                  this->GetGlobalNinjaGenerator()->ConfigDirectory(config),
                  "/link.d");
}

// Virtual protected methods.

std::string cmLocalNinjaGenerator::ConvertToIncludeReference(
  std::string const& path, cmOutputConverter::OutputFormat format)
{
  return this->ConvertToOutputFormat(path, format);
}

// Private methods.

cmGeneratedFileStream& cmLocalNinjaGenerator::GetImplFileStream(
  const std::string& config) const
{
  return *this->GetGlobalNinjaGenerator()->GetImplFileStream(config);
}

cmGeneratedFileStream& cmLocalNinjaGenerator::GetCommonFileStream() const
{
  return *this->GetGlobalNinjaGenerator()->GetCommonFileStream();
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
  this->WriteProjectHeader(this->GetCommonFileStream());

  if (this->GetGlobalGenerator()->IsMultiConfig()) {
    for (auto const& config : this->GetConfigNames()) {
      auto& stream = this->GetImplFileStream(config);
      this->WriteProjectHeader(stream);
      this->WriteNinjaRequiredVersion(stream);
      this->WriteNinjaConfigurationVariable(stream, config);
      this->WriteNinjaFilesInclusionConfig(stream);
    }
  } else {
    this->WriteNinjaRequiredVersion(this->GetCommonFileStream());
    this->WriteNinjaConfigurationVariable(this->GetCommonFileStream(),
                                          this->GetConfigNames().front());
  }
  this->WriteNinjaFilesInclusionCommon(this->GetCommonFileStream());
  this->WriteNinjaWorkDir(this->GetCommonFileStream());

  // For the rule file.
  this->WriteProjectHeader(this->GetRulesFileStream());
}

void cmLocalNinjaGenerator::WriteProjectHeader(std::ostream& os)
{
  cmGlobalNinjaGenerator::WriteDivider(os);
  os << "# Project: " << this->GetProjectName() << '\n'
     << "# Configurations: " << cmJoin(this->GetConfigNames(), ", ") << '\n';
  cmGlobalNinjaGenerator::WriteDivider(os);
}

void cmLocalNinjaGenerator::WriteNinjaRequiredVersion(std::ostream& os)
{
  // Default required version
  std::string requiredVersion = cmGlobalNinjaGenerator::RequiredNinjaVersion();

  // Ninja generator uses the 'console' pool if available (>= 1.5)
  if (this->GetGlobalNinjaGenerator()->SupportsDirectConsole()) {
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
  os << "ninja_required_version = " << requiredVersion << "\n\n";
}

void cmLocalNinjaGenerator::WriteNinjaConfigurationVariable(
  std::ostream& os, const std::string& config)
{
  cmGlobalNinjaGenerator::WriteVariable(
    os, "CONFIGURATION", config,
    "Set configuration variable for custom commands.");
}

void cmLocalNinjaGenerator::WritePools(std::ostream& os)
{
  cmGlobalNinjaGenerator::WriteDivider(os);

  cmValue jobpools =
    this->GetCMakeInstance()->GetState()->GetGlobalProperty("JOB_POOLS");
  if (!jobpools) {
    jobpools = this->GetMakefile()->GetDefinition("CMAKE_JOB_POOLS");
  }
  if (jobpools) {
    cmGlobalNinjaGenerator::WriteComment(
      os, "Pools defined by global property JOB_POOLS");
    cmList pools{ *jobpools };
    for (std::string const& pool : pools) {
      const std::string::size_type eq = pool.find('=');
      unsigned int jobs;
      if (eq != std::string::npos &&
          sscanf(pool.c_str() + eq, "=%u", &jobs) == 1) {
        os << "pool " << pool.substr(0, eq) << "\n  depth = " << jobs
           << "\n\n";
      } else {
        cmSystemTools::Error("Invalid pool defined by property 'JOB_POOLS': " +
                             pool);
      }
    }
  }
}

void cmLocalNinjaGenerator::WriteNinjaFilesInclusionConfig(std::ostream& os)
{
  cmGlobalNinjaGenerator::WriteDivider(os);
  os << "# Include auxiliary files.\n\n";
  cmGlobalNinjaGenerator* ng = this->GetGlobalNinjaGenerator();
  std::string const ninjaCommonFile =
    ng->NinjaOutputPath(cmGlobalNinjaMultiGenerator::NINJA_COMMON_FILE);
  std::string const commonFilePath = ng->EncodePath(ninjaCommonFile);
  cmGlobalNinjaGenerator::WriteInclude(os, commonFilePath,
                                       "Include common file.");
  os << "\n";
}

void cmLocalNinjaGenerator::WriteNinjaFilesInclusionCommon(std::ostream& os)
{
  cmGlobalNinjaGenerator::WriteDivider(os);
  os << "# Include auxiliary files.\n\n";
  cmGlobalNinjaGenerator* ng = this->GetGlobalNinjaGenerator();
  std::string const ninjaRulesFile =
    ng->NinjaOutputPath(cmGlobalNinjaGenerator::NINJA_RULES_FILE);
  std::string const rulesFilePath = ng->EncodePath(ninjaRulesFile);
  cmGlobalNinjaGenerator::WriteInclude(os, rulesFilePath,
                                       "Include rules file.");
  os << "\n";
}

void cmLocalNinjaGenerator::WriteNinjaWorkDir(std::ostream& os)
{
  cmGlobalNinjaGenerator::WriteDivider(os);
  cmGlobalNinjaGenerator::WriteComment(
    os, "Logical path to working directory; prefix for absolute paths.");
  cmGlobalNinjaGenerator* ng = this->GetGlobalNinjaGenerator();
  std::string ninja_workdir = this->GetBinaryDirectory();
  ng->StripNinjaOutputPathPrefixAsSuffix(ninja_workdir); // Also appends '/'.
  os << "cmake_ninja_workdir = " << ng->EncodePath(ninja_workdir) << "\n";
}

void cmLocalNinjaGenerator::WriteProcessedMakefile(std::ostream& os)
{
  cmGlobalNinjaGenerator::WriteDivider(os);
  os << "# Write statements declared in CMakeLists.txt:\n"
     << "# " << this->Makefile->GetSafeDefinition("CMAKE_CURRENT_LIST_FILE")
     << '\n';
  if (this->IsRootMakefile()) {
    os << "# Which is the root file.\n";
  }
  cmGlobalNinjaGenerator::WriteDivider(os);
  os << '\n';
}

void cmLocalNinjaGenerator::AppendTargetOutputs(cmGeneratorTarget* target,
                                                cmNinjaDeps& outputs,
                                                const std::string& config)
{
  this->GetGlobalNinjaGenerator()->AppendTargetOutputs(target, outputs, config,
                                                       DependOnTargetArtifact);
}

void cmLocalNinjaGenerator::AppendTargetDepends(cmGeneratorTarget* target,
                                                cmNinjaDeps& outputs,
                                                const std::string& config,
                                                const std::string& fileConfig,
                                                cmNinjaTargetDepends depends)
{
  this->GetGlobalNinjaGenerator()->AppendTargetDepends(target, outputs, config,
                                                       fileConfig, depends);
}

void cmLocalNinjaGenerator::AppendCustomCommandDeps(
  cmCustomCommandGenerator const& ccg, cmNinjaDeps& ninjaDeps,
  const std::string& config)
{
  for (std::string const& i : ccg.GetDepends()) {
    std::string dep;
    if (this->GetRealDependency(i, config, dep)) {
      ninjaDeps.push_back(
        this->GetGlobalNinjaGenerator()->ConvertToNinjaPath(dep));
    }
  }
}

std::string cmLocalNinjaGenerator::WriteCommandScript(
  std::vector<std::string> const& cmdLines, std::string const& outputConfig,
  std::string const& commandConfig, std::string const& customStep,
  cmGeneratorTarget const* target) const
{
  std::string scriptPath;
  if (target) {
    scriptPath = target->GetSupportDirectory();
  } else {
    scriptPath = cmStrCat(this->GetCurrentBinaryDirectory(), "/CMakeFiles");
  }
  scriptPath += this->GetGlobalNinjaGenerator()->ConfigDirectory(outputConfig);
  cmSystemTools::MakeDirectory(scriptPath);
  scriptPath += '/';
  scriptPath += customStep;
  if (this->GlobalGenerator->IsMultiConfig()) {
    scriptPath += cmStrCat('-', commandConfig);
  }
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

#ifdef _WIN32
namespace {
bool RuleNeedsCMD(std::string const& cmd)
{
  std::vector<std::string> args;
  cmSystemTools::ParseWindowsCommandLine(cmd.c_str(), args);
  auto it = std::find_if(args.cbegin(), args.cend(),
                         [](std::string const& arg) -> bool {
                           // FIXME: Detect more windows shell operators.
                           return cmHasLiteralPrefix(arg, ">");
                         });
  return it != args.cend();
}
}
#endif

std::string cmLocalNinjaGenerator::BuildCommandLine(
  std::vector<std::string> const& cmdLines, std::string const& outputConfig,
  std::string const& commandConfig, std::string const& customStep,
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
      std::string const scriptPath = this->WriteCommandScript(
        cmdLines, outputConfig, commandConfig, customStep, target);
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
#ifdef _WIN32
  cmGlobalNinjaGenerator const* gg = this->GetGlobalNinjaGenerator();
  bool const needCMD =
    cmdLines.size() > 1 || (customStep.empty() && RuleNeedsCMD(cmdLines[0]));
  for (auto li = cmdLines.begin(); li != cmdLines.end(); ++li) {
    if (li != cmdLines.begin()) {
      cmd << " && ";
    } else if (needCMD) {
      cmd << gg->GetComspec() << " /C \"";
    }
    // Put current cmdLine in brackets if it contains "||" because it has
    // higher precedence than "&&" in cmd.exe
    if (li->find("||") != std::string::npos) {
      cmd << "( " << *li << " )";
    } else {
      cmd << *li;
    }
  }
  if (needCMD) {
    cmd << "\"";
  }
#else
  for (auto li = cmdLines.begin(); li != cmdLines.end(); ++li) {
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
  auto* gg = this->GetGlobalNinjaGenerator();

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
    std::string c = ccg.GetCommand(i);
    if (c.empty()) {
      continue;
    }
    cmdLines.push_back(launcher +
                       this->ConvertToOutputFormat(
                         c,
                         gg->IsMultiConfig() ? cmOutputConverter::NINJAMULTI
                                             : cmOutputConverter::SHELL));

    std::string& cmd = cmdLines.back();
    ccg.AppendArguments(i, cmd);
  }
}

void cmLocalNinjaGenerator::WriteCustomCommandBuildStatement(
  cmCustomCommand const* cc, const std::set<cmGeneratorTarget*>& targets,
  const std::string& fileConfig)
{
  cmGlobalNinjaGenerator* gg = this->GetGlobalNinjaGenerator();
  if (gg->SeenCustomCommand(cc, fileConfig)) {
    return;
  }

  auto ccgs = this->MakeCustomCommandGenerators(*cc, fileConfig);
  for (cmCustomCommandGenerator const& ccg : ccgs) {
    if (ccg.GetOutputs().empty() && ccg.GetByproducts().empty()) {
      // Generator expressions evaluate to no output for this config.
      continue;
    }

    std::unordered_set<std::string> orderOnlyDeps;

    if (!cc->GetDependsExplicitOnly()) {
      // A custom command may appear on multiple targets.  However, some build
      // systems exist where the target dependencies on some of the targets are
      // overspecified, leading to a dependency cycle.  If we assume all target
      // dependencies are a superset of the true target dependencies for this
      // custom command, we can take the set intersection of all target
      // dependencies to obtain a correct dependency list.
      //
      // FIXME: This won't work in certain obscure scenarios involving indirect
      // dependencies.
      auto j = targets.begin();
      assert(j != targets.end());
      this->GetGlobalNinjaGenerator()->AppendTargetDependsClosure(
        *j, orderOnlyDeps, ccg.GetOutputConfig(), fileConfig, ccgs.size() > 1);
      ++j;

      for (; j != targets.end(); ++j) {
        std::unordered_set<std::string> jDeps;
        this->GetGlobalNinjaGenerator()->AppendTargetDependsClosure(
          *j, jDeps, ccg.GetOutputConfig(), fileConfig, ccgs.size() > 1);
        cm::erase_if(orderOnlyDeps, [&jDeps](std::string const& dep) {
          return jDeps.find(dep) == jDeps.end();
        });
      }
    }

    const std::vector<std::string>& outputs = ccg.GetOutputs();
    const std::vector<std::string>& byproducts = ccg.GetByproducts();

    bool symbolic = false;
    for (std::string const& output : outputs) {
      if (cmSourceFile* sf = this->Makefile->GetSource(output)) {
        if (sf->GetPropertyAsBool("SYMBOLIC")) {
          symbolic = true;
          break;
        }
      }
    }

    cmGlobalNinjaGenerator::CCOutputs ccOutputs(gg);
    ccOutputs.Add(outputs);
    ccOutputs.Add(byproducts);

    std::string mainOutput = ccOutputs.ExplicitOuts[0];

    cmNinjaDeps ninjaDeps;
    this->AppendCustomCommandDeps(ccg, ninjaDeps, fileConfig);

    std::vector<std::string> cmdLines;
    this->AppendCustomCommandLines(ccg, cmdLines);

    cmNinjaDeps sortedOrderOnlyDeps(orderOnlyDeps.begin(),
                                    orderOnlyDeps.end());
    std::sort(sortedOrderOnlyDeps.begin(), sortedOrderOnlyDeps.end());

    if (cmdLines.empty()) {
      cmNinjaBuild build("phony");
      build.Comment = cmStrCat("Phony custom command for ", mainOutput);
      build.Outputs = std::move(ccOutputs.ExplicitOuts);
      build.WorkDirOuts = std::move(ccOutputs.WorkDirOuts);
      build.ExplicitDeps = std::move(ninjaDeps);
      build.OrderOnlyDeps = std::move(sortedOrderOnlyDeps);
      gg->WriteBuild(this->GetImplFileStream(fileConfig), build);
    } else {
      std::string customStep = cmSystemTools::GetFilenameName(mainOutput);
      if (this->GlobalGenerator->IsMultiConfig()) {
        customStep += '-';
        customStep += fileConfig;
        customStep += '-';
        customStep += ccg.GetOutputConfig();
      }
      // Hash full path to make unique.
      customStep += '-';
      cmCryptoHash hash(cmCryptoHash::AlgoSHA256);
      customStep += hash.HashString(mainOutput).substr(0, 7);

      std::string depfile = ccg.GetDepfile();
      if (!depfile.empty()) {
        switch (cc->GetCMP0116Status()) {
          case cmPolicies::WARN:
            if (this->GetCurrentBinaryDirectory() !=
                  this->GetBinaryDirectory() ||
                this->Makefile->PolicyOptionalWarningEnabled(
                  "CMAKE_POLICY_WARNING_CMP0116")) {
              this->GetCMakeInstance()->IssueMessage(
                MessageType::AUTHOR_WARNING,
                cmPolicies::GetPolicyWarning(cmPolicies::CMP0116),
                cc->GetBacktrace());
            }
            CM_FALLTHROUGH;
          case cmPolicies::OLD:
            break;
          case cmPolicies::REQUIRED_IF_USED:
          case cmPolicies::REQUIRED_ALWAYS:
          case cmPolicies::NEW:
            depfile = ccg.GetInternalDepfile();
            break;
        }
      }

      std::string comment = cmStrCat("Custom command for ", mainOutput);
      gg->WriteCustomCommandBuild(
        this->BuildCommandLine(cmdLines, ccg.GetOutputConfig(), fileConfig,
                               customStep),
        this->ConstructComment(ccg), comment, depfile, cc->GetJobPool(),
        cc->GetUsesTerminal(),
        /*restat*/ !symbolic || !byproducts.empty(), fileConfig,
        std::move(ccOutputs), std::move(ninjaDeps),
        std::move(sortedOrderOnlyDeps));
    }
  }
}

bool cmLocalNinjaGenerator::HasUniqueByproducts(
  std::vector<std::string> const& byproducts, cmListFileBacktrace const& bt)
{
  std::vector<std::string> configs =
    this->GetMakefile()->GetGeneratorConfigs(cmMakefile::IncludeEmptyConfig);
  cmGeneratorExpression ge(*this->GetCMakeInstance(), bt);
  for (std::string const& p : byproducts) {
    if (cmGeneratorExpression::Find(p) == std::string::npos) {
      return false;
    }
    std::set<std::string> seen;
    std::unique_ptr<cmCompiledGeneratorExpression> cge = ge.Parse(p);
    for (std::string const& config : configs) {
      for (std::string const& b :
           this->ExpandCustomCommandOutputPaths(*cge, config)) {
        if (!seen.insert(b).second) {
          return false;
        }
      }
    }
  }
  return true;
}

namespace {
bool HasUniqueOutputs(std::vector<cmCustomCommandGenerator> const& ccgs)
{
  std::set<std::string> allOutputs;
  std::set<std::string> allByproducts;
  for (cmCustomCommandGenerator const& ccg : ccgs) {
    for (std::string const& output : ccg.GetOutputs()) {
      if (!allOutputs.insert(output).second) {
        return false;
      }
    }
    for (std::string const& byproduct : ccg.GetByproducts()) {
      if (!allByproducts.insert(byproduct).second) {
        return false;
      }
    }
  }
  return true;
}
}

std::string cmLocalNinjaGenerator::CreateUtilityOutput(
  std::string const& targetName, std::vector<std::string> const& byproducts,
  cmListFileBacktrace const& bt)
{
  // In Ninja Multi-Config, we can only produce cross-config utility
  // commands if all byproducts are per-config.
  if (!this->GetGlobalGenerator()->IsMultiConfig() ||
      !this->HasUniqueByproducts(byproducts, bt)) {
    return this->cmLocalGenerator::CreateUtilityOutput(targetName, byproducts,
                                                       bt);
  }

  std::string const base = cmStrCat(this->GetCurrentBinaryDirectory(),
                                    "/CMakeFiles/", targetName, '-');
  // The output is not actually created so mark it symbolic.
  for (std::string const& config :
       this->Makefile->GetGeneratorConfigs(cmMakefile::IncludeEmptyConfig)) {
    std::string const force = cmStrCat(base, config);
    if (cmSourceFile* sf = this->Makefile->GetOrCreateGeneratedSource(force)) {
      sf->SetProperty("SYMBOLIC", "1");
    } else {
      cmSystemTools::Error("Could not get source file entry for " + force);
    }
  }
  this->GetGlobalNinjaGenerator()->AddPerConfigUtilityTarget(targetName);
  return cmStrCat(base, "$<CONFIG>"_s);
}

std::vector<cmCustomCommandGenerator>
cmLocalNinjaGenerator::MakeCustomCommandGenerators(
  cmCustomCommand const& cc, std::string const& fileConfig)
{
  cmGlobalNinjaGenerator const* gg = this->GetGlobalNinjaGenerator();

  bool transformDepfile = false;
  switch (cc.GetCMP0116Status()) {
    case cmPolicies::WARN:
      CM_FALLTHROUGH;
    case cmPolicies::OLD:
      break;
    case cmPolicies::REQUIRED_IF_USED:
    case cmPolicies::REQUIRED_ALWAYS:
    case cmPolicies::NEW:
      transformDepfile = true;
      break;
  }

  // Start with the build graph's configuration.
  std::vector<cmCustomCommandGenerator> ccgs;
  ccgs.emplace_back(cc, fileConfig, this, transformDepfile);

  // Consider adding cross configurations.
  if (!gg->EnableCrossConfigBuild()) {
    return ccgs;
  }

  // Outputs and byproducts must be expressed using generator expressions.
  for (std::string const& output : cc.GetOutputs()) {
    if (cmGeneratorExpression::Find(output) == std::string::npos) {
      return ccgs;
    }
  }
  for (std::string const& byproduct : cc.GetByproducts()) {
    if (cmGeneratorExpression::Find(byproduct) == std::string::npos) {
      return ccgs;
    }
  }

  // Tentatively add the other cross configurations.
  for (std::string const& config : gg->GetCrossConfigs(fileConfig)) {
    if (fileConfig != config) {
      ccgs.emplace_back(cc, fileConfig, this, transformDepfile, config);
    }
  }

  // If outputs and byproducts are not unique to each configuration,
  // drop the cross configurations.
  if (!HasUniqueOutputs(ccgs)) {
    ccgs.erase(ccgs.begin() + 1, ccgs.end());
  }

  return ccgs;
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

void cmLocalNinjaGenerator::WriteCustomCommandBuildStatements(
  const std::string& fileConfig)
{
  for (cmCustomCommand const* customCommand : this->CustomCommands) {
    auto i = this->CustomCommandTargets.find(customCommand);
    assert(i != this->CustomCommandTargets.end());

    this->WriteCustomCommandBuildStatement(i->first, i->second, fileConfig);
  }
}

std::string cmLocalNinjaGenerator::MakeCustomLauncher(
  cmCustomCommandGenerator const& ccg)
{
  cmValue property_value = this->Makefile->GetProperty("RULE_LAUNCH_CUSTOM");

  if (!cmNonempty(property_value)) {
    return std::string();
  }

  // Expand rule variables referenced in the given launcher command.
  cmRulePlaceholderExpander::RuleVariables vars;

  std::string output;
  const std::vector<std::string>& outputs = ccg.GetOutputs();
  if (!outputs.empty()) {
    output = outputs[0];
    if (ccg.GetWorkingDirectory().empty()) {
      output = this->MaybeRelativeToCurBinDir(output);
    }
    output = this->ConvertToOutputFormat(output, cmOutputConverter::SHELL);
  }
  vars.Output = output.c_str();

  auto rulePlaceholderExpander = this->CreateRulePlaceholderExpander();

  std::string launcher = *property_value;
  rulePlaceholderExpander->ExpandRuleVariables(this, launcher, vars);
  if (!launcher.empty()) {
    launcher += " ";
  }

  return launcher;
}

void cmLocalNinjaGenerator::AdditionalCleanFiles(const std::string& config)
{
  if (cmValue prop_value =
        this->Makefile->GetProperty("ADDITIONAL_CLEAN_FILES")) {
    cmList cleanFiles{ cmGeneratorExpression::Evaluate(*prop_value, this,
                                                       config) };
    std::string const& binaryDir = this->GetCurrentBinaryDirectory();
    cmGlobalNinjaGenerator* gg = this->GetGlobalNinjaGenerator();
    for (auto const& cleanFile : cleanFiles) {
      // Support relative paths
      gg->AddAdditionalCleanFile(
        cmSystemTools::CollapseFullPath(cleanFile, binaryDir), config);
    }
  }
}
