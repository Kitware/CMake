/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include <map>
#include <memory>
#include <set>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "cmComputeLinkInformation.h"
#include "cmFastbuildTargetGenerator.h"
#include "cmGeneratorTarget.h"
#include "cmGlobalFastbuildGenerator.h"
#include "cmRulePlaceholderExpander.h"
class cmSourceFile;

class cmFastbuildNormalTargetGenerator : public cmFastbuildTargetGenerator
{
  std::unique_ptr<cmRulePlaceholderExpander> const RulePlaceholderExpander;
  std::string const ObjectOutDir;
  std::set<std::string> const Languages;
  std::unordered_map<std::string, std::string> const CompileObjectCmakeRules;
  std::string const CudaCompileMode;

  // Now we're adding our link deps to command line and using .Libraries2 for
  // tracking deps.
  bool UsingCommandLine = false;

  std::unordered_map<std::string, std::string> TargetIncludesByLanguage;
  std::map<std::pair<std::string, std::string>, std::string>
    CompileFlagsByLangAndArch;

public:
  cmFastbuildNormalTargetGenerator(cmGeneratorTarget* gt, std::string config);

  void Generate() override;

private:
  void GenerateLink(FastbuildTarget& target,
                    std::vector<std::string> const& objectDepends);
  bool DetectBaseLinkerCommand(std::string& command, std::string const& arch,
                               cmGeneratorTarget::Names const& targetNames);

  // Get languages used by the target.
  std::set<std::string> GetLanguages();
  // Returns mapping from language to command how to compile object file for
  // the it.
  // Example return value: {"CXX" : "<CMAKE_CXX_COMPILER> <DEFINES> <INCLUDES>
  // <FLAGS> -o <OBJECT> -c <SOURCE>" }
  std::unordered_map<std::string, std::string> GetCompileObjectCommand() const;
  std::string GetCudaCompileMode() const;
  std::string GetLinkCommand() const;

  void AddCompilerLaunchersForLanguages();
  void AddLinkerLauncher();
  void AddCMakeLauncher();

  void ComputePaths(FastbuildTarget& fastbuildTarget) const;

  std::string ComputeCodeCheckOptions(cmSourceFile const& srcFile);

  cmRulePlaceholderExpander::RuleVariables ComputeRuleVariables() const;

  std::vector<std::string> GetSourceProperty(cmSourceFile const& srcFile,
                                             std::string const& prop) const;

  std::string GetCompileOptions(cmSourceFile const& srcFile,
                                std::string const& arch);

  std::vector<std::string> GetArches() const;

  void GetCudaDeviceLinkLinkerAndArgs(std::string& linker,
                                      std::string& args) const;
  void GenerateCudaDeviceLink(FastbuildTarget& target) const;
  void GenerateObjects(FastbuildTarget& target);
  FastbuildUnityNode GetOneUnity(std::set<std::string> const& isolatedFiles,
                                 std::vector<std::string>& files,
                                 int unitySize) const;

  int GetUnityBatchSize() const;
  std::vector<FastbuildUnityNode> GenerateUnity(
    std::vector<FastbuildObjectListNode>& objects,
    std::set<std::string> const& isolatedSources,
    std::map<std::string, std::vector<std::string>> const& sourcesWithGroups);
  FastbuildUnityNode GenerateGroupedUnityNode(
    std::vector<std::string>& inputFiles,
    std::map<std::string, std::vector<std::string>> const& sourcesWithGroups,
    int& groupId);

  // Computes .CompilerOptions for the ObjectList node.
  void ComputeCompilerAndOptions(std::string const& compilerOptions,
                                 std::string const& staticCheckOptions,
                                 std::string const& language,
                                 FastbuildObjectListNode& outObjectList);

  std::string GetImportedLoc(cmComputeLinkInformation::Item const& item) const;
  std::string ResolveIfAlias(std::string const& targetName) const;

  void AppendExtraResources(std::set<std::string>& deps) const;
  void AppendExternalObject(FastbuildLinkerNode& linkerNode,
                            std::set<std::string>& linkedObjects) const;
  void AppendExeToLink(FastbuildLinkerNode& linkerNode,
                       cmComputeLinkInformation::Item const& item) const;
  void AppendTargetDep(FastbuildLinkerNode& linkerNode,
                       std::set<std::string>& linkedObjects,
                       cmComputeLinkInformation::Item const& item) const;
  void AppendPrebuildDeps(FastbuildLinkerNode& linkerNode,
                          cmComputeLinkInformation::Item const& item) const;
  void AppendTransitivelyLinkedObjects(
    cmGeneratorTarget const& target,
    std::set<std::string>& linkedObjects) const;
  void AppendCommandLineDep(FastbuildLinkerNode& linkerNode,
                            cmComputeLinkInformation::Item const& item) const;
  void AppendToLibraries2IfApplicable(FastbuildLinkerNode& linkerNode,
                                      std::string dep) const;
  void AppendLINK_DEPENDS(FastbuildLinkerNode& linkerNode) const;
  void AppendLinkDep(FastbuildLinkerNode& linkerNode, std::string dep) const;
  void AppendDirectObjectLibs(FastbuildLinkerNode& linkerNode,
                              std::set<std::string>& linkedObjects);

  void AppendLinkDeps(std::set<FastbuildTargetDep>& preBuildDeps,
                      FastbuildLinkerNode& linkerNode,
                      FastbuildLinkerNode& cudaDeviceLinkLinkerNode);
  void AddLipoCommand(FastbuildTarget& target);
  void GenerateModuleDefinitionInfo(FastbuildTarget& target) const;
  std::vector<FastbuildExecNode> GetSymlinkExecs() const;
  void ProcessManifests(FastbuildLinkerNode& linkerNode) const;
  void AddStampExeIfApplicable(FastbuildTarget& fastbuildTarget) const;
  void ProcessPostBuildForStaticLib(FastbuildTarget& fastbuildTarget) const;
  void CollapseAllExecsIntoOneScriptfile(
    std::string const& scriptFileName,
    std::vector<FastbuildExecNode> const& execs) const;

  void AddPrebuildDeps(FastbuildTarget& target) const;

  std::string DetectCompilerFlags(cmSourceFile const& srcFile,
                                  std::string const& arch);

  void SplitLinkerFromArgs(std::string const& command,
                           std::string& outLinkerExecutable,
                           std::string& outLinkerArgs) const;
  void GetLinkerExecutableAndArgs(std::string const& command,
                                  std::string& outLinkerExecutable,
                                  std::string& outLinkerArgs);

  void ApplyLinkRuleLauncher(std::string& command);
  void ApplyLWYUToLinkerCommand(FastbuildLinkerNode& linkerNode);

  std::string ComputeDefines(cmSourceFile const& srcFile);

  void ComputePCH(cmSourceFile const& srcFile, FastbuildObjectListNode& node,
                  std::set<std::string>& createdPCH);

  std::vector<std::string> GetManifestsAsFastbuildPath() const;

  void EnsureDirectoryExists(std::string const& path) const;
  void EnsureParentDirectoryExists(std::string const& path) const;
};
