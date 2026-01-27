/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include <memory>
#include <set>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "cmsys/FStream.hxx"

#include "cmCommonTargetGenerator.h"
#include "cmGeneratorTarget.h"
#include "cmGlobalFastbuildGenerator.h"
#include "cmGlobalGenerator.h"
#include "cmOSXBundleGenerator.h"

class cmCustomCommand;
class cmCustomCommandGenerator;
class cmLocalFastbuildGenerator;
class cmMakefile;

enum class FastbuildBuildStep
{
  PRE_BUILD,
  PRE_LINK,
  POST_BUILD,
  REST,
};

class cmFastbuildTargetGenerator : public cmCommonTargetGenerator
{
public:
  /// Create a cmFastbuildTargetGenerator according to the @a target's type and
  /// config.
  static cmFastbuildTargetGenerator* New(cmGeneratorTarget* target,
                                         std::string config);

  cmFastbuildTargetGenerator(cmGeneratorTarget* target, std::string config);

  virtual void Generate() {}

  std::string GetClangTidyReplacementsFilePath(
    std::string const& directory, cmSourceFile const& source,
    std::string const& Config) const override;

  void AddIncludeFlags(std::string& languageFlags, std::string const& language,
                       std::string const&) override;

  cmGeneratorTarget::Names DetectOutput() const;

  void AddObjectDependencies(FastbuildTarget& fastbuildTarget,
                             std::vector<std::string>& allObjectDepends) const;
  void AddLinkerNodeDependencies(FastbuildTarget& fastbuildTarget);

  std::string ConvertToFastbuildPath(std::string const& path) const;

  cmGlobalFastbuildGenerator* GetGlobalGenerator() const;

  std::string GetName();

  cmMakefile* GetMakefile() const { return this->Makefile; }

  cmGeneratorTarget* GetGeneratorTarget() const
  {
    return this->GeneratorTarget;
  }

  void LogMessage(std::string const& m) const;

private:
  std::string GetUtilityAliasFromBuildStep(FastbuildBuildStep step) const;

protected:
  cmLocalFastbuildGenerator* GetLocalGenerator() const
  {
    return this->LocalGenerator;
  }

  std::string GetTargetName() const;
  std::string GetCdCommand(cmCustomCommandGenerator const& ccg) const;
  std::string GetScriptWorkingDir(cmCustomCommandGenerator const& ccg) const;
  std::string GetScriptFilename(std::string const& utilityTargetName) const;
  void GetDepends(cmCustomCommandGenerator const& ccg,
                  std::string const& currentCCName,
                  std::vector<std::string>& fileLevelDeps,
                  std::set<FastbuildTargetDep>& targetDep) const;

  void AddCommentPrinting(std::vector<std::string>& cmdLines,
                          cmCustomCommandGenerator const& ccg) const;

  void WriteCmdsToFile(cmsys::ofstream& file,
                       std::vector<std::string> const& cmds) const;

  void AddOutput(cmCustomCommandGenerator const& ccg, FastbuildExecNode& exec);

  void AddExecArguments(FastbuildExecNode& exec,
                        std::string const& scriptFilename) const;

  void ReplaceProblematicMakeVars(std::string& command) const;

  FastbuildExecNodes GenerateCommands(FastbuildBuildStep buildStep);
  FastbuildExecNode GetAppleTextStubCommand() const;
  FastbuildExecNode GetDepsCheckExec(FastbuildExecNode const& depender);

  std::string MakeCustomLauncher(cmCustomCommandGenerator const& ccg);

  std::string GetCustomCommandTargetName(cmCustomCommand const& cc,
                                         FastbuildBuildStep step) const;

  void WriteScriptProlog(cmsys::ofstream& file) const;
  void WriteScriptEpilog(cmsys::ofstream& file) const;

  void AdditionalCleanFiles();

  // write rules for Mac OS X Application Bundle content.
  struct MacOSXContentGeneratorType
    : cmOSXBundleGenerator::MacOSXContentGeneratorType
  {
    MacOSXContentGeneratorType(cmFastbuildTargetGenerator* g, std::string cfg)
      : Generator(g)
      , Config(std::move(cfg))
    {
    }

    void operator()(cmSourceFile const& source, char const* pkgloc,
                    std::string const& config) override;

  private:
    cmFastbuildTargetGenerator* Generator;
    std::string const Config;
  };
  friend struct MacOSXContentGeneratorType;

  // "MacOSXContentGenerator" has to be per-config once multiconfig generator
  // is implemented.
  std::unique_ptr<MacOSXContentGeneratorType> MacOSXContentGenerator;
  std::unique_ptr<cmOSXBundleGenerator> OSXBundleGenerator;
  std::set<std::string> MacContentFolders;

  std::vector<FastbuildCopyNode> CopyNodes;

  cmLocalFastbuildGenerator* LocalGenerator;
  cmGlobalGenerator::TargetDependSet const TargetDirectDependencies;
  std::string const Config;

  // Sometimes CMake adds equivalent custom commands to different targets.
  // Not really supported by FBuild (can't have different exec nodes producing
  // same output). So, in such cases we need to "re-map" the exec to produce a
  // "dummy" output (and update all deps within the target).
  // TODO: potentially 1 map should be enough?
  std::unordered_map<std::string, std::string> OutputsToReplace;
  std::unordered_map<std::string, std::string> OutputToExecName;
};
