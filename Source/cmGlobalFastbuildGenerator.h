/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include <algorithm>
#include <iterator>
#include <map>
#include <set>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include <cm/memory>
#include <cm/optional>

#include <assert.h>

#include "cmBuildOptions.h"
#include "cmGeneratedFileStream.h"
#include "cmGlobalCommonGenerator.h"
class cmFastbuildTargetGenerator;
class cmGeneratorTarget;
class cmGlobalGeneratorFactory;
class cmMakefile;
class cmake;
struct cmDocumentationEntry;

#define FASTBUILD_DOLLAR_TAG "FASTBUILD_DOLLAR_TAG"
#define FASTBUILD_1_INPUT_PLACEHOLDER                                         \
  FASTBUILD_DOLLAR_TAG "FB_INPUT_1_PLACEHOLDER" FASTBUILD_DOLLAR_TAG
#define FASTBUILD_1_0_INPUT_PLACEHOLDER                                       \
  FASTBUILD_DOLLAR_TAG "FB_INPUT_1_0_PLACEHOLDER" FASTBUILD_DOLLAR_TAG
#define FASTBUILD_1_1_INPUT_PLACEHOLDER                                       \
  FASTBUILD_DOLLAR_TAG "FB_INPUT_1_1_PLACEHOLDER" FASTBUILD_DOLLAR_TAG

#define FASTBUILD_2_INPUT_PLACEHOLDER                                         \
  FASTBUILD_DOLLAR_TAG "FB_INPUT_2_PLACEHOLDER" FASTBUILD_DOLLAR_TAG

#define FASTBUILD_3_INPUT_PLACEHOLDER                                         \
  FASTBUILD_DOLLAR_TAG "FB_INPUT_3_PLACEHOLDER" FASTBUILD_DOLLAR_TAG

// Alias to artifacts that can be consumed by the linker (DLL or Library).
#define FASTBUILD_LINK_ARTIFACTS_ALIAS_POSTFIX "-link-artifacts"
// Alias to all the ObjectList nodes.
#define FASTBUILD_OBJECTS_ALIAS_POSTFIX "-objects"
// Alias to all the dependencies of the target.
#define FASTBUILD_DEPS_ARTIFACTS_ALIAS_POSTFIX "-deps"
#define FASTBUILD_PRE_BUILD_ALIAS_POSTFIX "-pre-build"
#define FASTBUILD_PRE_LINK_ALIAS_POSTFIX "-pre-link"
#define FASTBUILD_POST_BUILD_ALIAS_POSTFIX "-post-build"
// Alias to all other custom commands in the target.
#define FASTBUILD_CUSTOM_COMMAND_ALIAS_POSTFIX "-custom-commands"
// Alias to outputs produced by a custom command (since FASTBuild exec node
// does not support more than 1 output).
#define FASTBUILD_OUTPUTS_ALIAS_POSTFIX "-outputs"
// Alias to byproducts produced by a custom command (since FASTBuild exec node
// does not support more than 1 output).
#define FASTBUILD_BYPRODUCTS_ALIAS_POSTFIX "-byproducts"

#define FASTBUILD_COMPILER_PREFIX "Compiler_"
#define FASTBUILD_LAUNCHER_PREFIX "Launcher_"
#define FASTBUILD_LINKER_LAUNCHER_PREFIX "LinkerLauncher_"

#define FASTBUILD_RESTAT_FILE "FASTBUILD_RESTAT"

#define FASTBUILD_UTIL_CONCURRENCY_GROUP_NAME "Utils"

#define FASTBUILD_ALL_TARGET_NAME "all"
#define FASTBUILD_CLEAN_TARGET_NAME "clean"

#define FASTBUILD_NOOP_FILE_NAME "fbuild_noop"
#define FASTBUILD_CLEAN_FILE_NAME "fbuild_clean-out"

#define FASTBUILD_BUILD_FILE "fbuild.bff"

#define FASTBUILD_DUMMY_OUTPUT_EXTENSION ".fbuild-cc-out"

#if defined(_WIN32)
#  define FASTBUILD_SCRIPT_FILE_EXTENSION ".bat"
#  define FASTBUILD_SCRIPT_FILE_ARG "/C "
#  define FASTBUILD_SCRIPT_CD "cd /D "
#  define FASTBUILD_CLEAN_SCRIPT_NAME "clean" FASTBUILD_SCRIPT_FILE_EXTENSION
#else
#  define FASTBUILD_SCRIPT_FILE_EXTENSION ".sh"
#  define FASTBUILD_SCRIPT_FILE_ARG ""
#  define FASTBUILD_SCRIPT_CD "cd "
#  define FASTBUILD_CLEAN_SCRIPT_NAME "clean" FASTBUILD_SCRIPT_FILE_EXTENSION
#endif

enum class FastbuildTargetDepType
{
  // Order-only dependency that is not going to appear in the generated file.
  ORDER_ONLY,
  // Regular target dep.
  REGULAR,
  // Utility target dep.
  UTIL,
};
struct FastbuildTargetDep
{
  std::string Name;
  FastbuildTargetDepType Type = FastbuildTargetDepType::REGULAR;
  FastbuildTargetDep(std::string n)
    : Name(std::move(n))
  {
  }
  bool operator==(FastbuildTargetDep const& rhs) const
  {
    return this->Name == rhs.Name;
  }
  bool operator<(FastbuildTargetDep const& rhs) const
  {
    return this->Name < rhs.Name;
  }
};

enum class FastbuildTargetType
{
  ALIAS, // Alias node
  EXEC,  // Exec node
  LINK,  // Library, DLL or Executable
  OBJECTLIST,
  UNITY,
};

struct FastbuildTargetBase
{
  // Target name with config postfix.
  std::string Name;
  // Target name without config postfix, we use it to locate IDE project for
  // the given target and add +1 config to it.
  std::string BaseName;
  std::string BasePath;
  std::set<FastbuildTargetDep> PreBuildDependencies;
  bool Hidden = true;
  FastbuildTargetType Type;
  explicit FastbuildTargetBase(FastbuildTargetType TargetType)
    : Type(TargetType)
  {
  }
};
using FastbuildTargetPtrT = std::unique_ptr<FastbuildTargetBase>;

struct FastbuildAliasNode : public FastbuildTargetBase
{
  bool ExcludeFromAll = false;
  FastbuildAliasNode()
    : FastbuildTargetBase(FastbuildTargetType::ALIAS)
  {
  }
};

struct FastbuildExecNode : public FastbuildTargetBase
{
  std::string ExecExecutable;
  std::string ExecArguments;
  std::string ScriptFile;
  std::string ExecWorkingDir;
  bool ExecUseStdOutAsOutput = false;
  std::string ExecOutput;
  std::vector<std::string> ExecInput;
  std::vector<std::string> ExecInputPath;
  std::vector<std::string> ExecInputPattern;
  bool ExecInputPathRecurse = false;
  bool ExecAlways = false;
  FastbuildAliasNode OutputsAlias;
  FastbuildAliasNode ByproductsAlias;
  std::string ConcurrencyGroupName;
  bool ExcludeFromAll = false;
  FastbuildExecNode()
    : FastbuildTargetBase(FastbuildTargetType::EXEC)
  {
  }

  bool NeedsDepsCheckExec = false;
};

struct FastbuildCompiler
{
  std::map<std::string, std::string> ExtraVariables;
  std::string Name;
  std::string Executable;
  std::string CmakeCompilerID;
  std::string CompilerFamily = "custom";
  std::string CmakeCompilerVersion;
  std::string Language;
  std::vector<std::string> ExtraFiles;
  bool UseLightCache = false;
  bool ClangRewriteIncludes = true;
  bool ClangGCCUpdateXLanguageArg = false;
  bool AllowResponseFile = false;
  bool ForceResponseFile = false;
  bool UseRelativePaths = false;
  bool UseDeterministicPaths = false;
  std::string SourceMapping;
  // Only used for launchers.
  std::string Args;
  bool DontUseEnv = false;
};

struct FastbuildObjectListNode : public FastbuildTargetBase
{
  std::string Compiler;
  std::string CompilerOptions;
  std::string CompilerOutputPath;
  std::string CompilerOutputExtension;
  std::vector<std::string> CompilerInputUnity;
  std::string PCHInputFile;
  std::string PCHOutputFile;
  std::string PCHOptions;

  std::vector<std::string> CompilerInputFiles;
  bool AllowCaching = true;
  bool AllowDistribution = true;

  std::set<std::string> ObjectOutputs;
  std::set<std::string> ObjectDepends;

  // Apple only.
  std::string arch;
  FastbuildObjectListNode()
    : FastbuildTargetBase(FastbuildTargetType::OBJECTLIST)
  {
  }
};

struct FastbuildUnityNode : public FastbuildTargetBase
{
  std::string UnityOutputPath;
  std::vector<std::string> UnityInputFiles;
  std::string UnityOutputPattern;
  std::vector<std::string> UnityInputIsolatedFiles;
  FastbuildUnityNode()
    : FastbuildTargetBase(FastbuildTargetType::UNITY)
  {
  }
};

struct IDEProjectConfig
{
  std::string Config;
  std::string Target;
  // VS only.
  std::string Platform;

  std::string XCodeBaseSDK;
  std::string XCodeDebugWorkingDir;
  std::string XCodeIphoneOSDeploymentTarget;
};

struct IDEProjectCommon
{
  std::string Alias;
  std::string ProjectOutput;
  std::string ProjectBasePath;

  std::vector<IDEProjectConfig> ProjectConfigs;
};

struct XCodeProject : public IDEProjectCommon
{
};

struct VCXProject : public IDEProjectCommon
{
  std::string folder;
  std::set<FastbuildTargetDep> deps;
};

struct FastbuildLinkerNode
{
  enum
  {
    EXECUTABLE,
    SHARED_LIBRARY,
    STATIC_LIBRARY,
    NONE
  } Type = NONE;

  std::string Name;
  std::string Compiler;
  std::string CompilerOptions;
  std::string Linker;
  std::string LinkerType;
  std::string LinkerOutput;
  std::string LinkerOptions;
  std::vector<std::string> LibrarianAdditionalInputs;
  // We only use Libraries2 for tracking dependencies.
  std::vector<std::string> Libraries2;
  std::set<std::string> PreBuildDependencies;
  bool LinkerLinkObjects = false;
  std::string LinkerStampExe;
  std::string LinkerStampExeArgs;
  // Apple only.
  std::string Arch;
};

struct FastbuildCopyNode
{
  std::string Name;
  std::string Source;
  std::string Dest;
  std::string PreBuildDependencies;
  bool CopyDir = false;
};

struct FastbuildExecNodes
{
  std::vector<FastbuildExecNode> Nodes;
  FastbuildAliasNode Alias;
};

struct FastbuildTarget : public FastbuildTargetBase
{
  std::map<std::string, std::string> Variables;
  std::vector<FastbuildObjectListNode> ObjectListNodes;
  std::vector<FastbuildUnityNode> UnityNodes;
  // Potentially multiple libs for different archs (apple only)
  std::vector<FastbuildLinkerNode> CudaDeviceLinkNode;
  std::vector<FastbuildLinkerNode> LinkerNode;
  std::string RealOutput;
  FastbuildAliasNode PreBuildExecNodes, ExecNodes;
  std::vector<FastbuildAliasNode> AliasNodes;
  // This alias must be written before all other nodes, since they might need
  // to refer to it.
  FastbuildAliasNode DependenciesAlias;
  std::vector<FastbuildCopyNode> CopyNodes;
  FastbuildExecNodes PreLinkExecNodes;
  FastbuildExecNodes PostBuildExecNodes;
  bool IsGlobal = false;
  bool ExcludeFromAll = false;
  bool AllowDistribution = true;
  FastbuildTarget()
    : FastbuildTargetBase(FastbuildTargetType::LINK)
  {
  }

  void GenerateAliases();
};

class cmGlobalFastbuildGenerator : public cmGlobalCommonGenerator
{
public:
  cmGlobalFastbuildGenerator(cmake* cm);

  void ReadCompilerOptions(FastbuildCompiler& compiler, cmMakefile* mf);
  void ProcessEnvironment();

  static std::unique_ptr<cmGlobalGeneratorFactory> NewFactory();
  void Generate() override;

  bool FindMakeProgram(cmMakefile* mf) override;

  void EnableLanguage(std::vector<std::string> const& lang, cmMakefile* mf,
                      bool optional) override;

  bool IsFastbuild() const override { return true; }

  std::vector<GeneratedMakeCommand> GenerateBuildCommand(
    std::string const& makeProgram, std::string const& projectName,
    std::string const& projectDir, std::vector<std::string> const& targetNames,
    std::string const& config, int jobs, bool verbose,
    cmBuildOptions buildOptions = cmBuildOptions(),
    std::vector<std::string> const& makeOptions = std::vector<std::string>(),
    BuildTryCompile isInTryCompile = BuildTryCompile::No) override;

  std::unique_ptr<cmLocalGenerator> CreateLocalGenerator(
    cmMakefile* makefile) override;
  std::string GetName() const override
  {
    return cmGlobalFastbuildGenerator::GetActualName();
  }

  bool IsMultiConfig() const override { return false; }

  bool SupportsCustomObjectNames() const override { return false; }

  void ComputeTargetObjectDirectory(cmGeneratorTarget*) const override;
  void AppendDirectoryForConfig(std::string const& prefix,
                                std::string const& config,
                                std::string const& suffix,
                                std::string& dir) override;

  static std::string GetActualName() { return "FASTBuild"; }
  static std::string RequiredFastbuildVersion() { return "1.14"; }

  // Setup target names
  char const* GetAllTargetName() const override
  {
    return FASTBUILD_ALL_TARGET_NAME;
  }
  char const* GetInstallTargetName() const override { return "install"; }
  char const* GetCleanTargetName() const override
  {
    return FASTBUILD_CLEAN_TARGET_NAME;
  }
  char const* GetInstallLocalTargetName() const override
  {
    return "install/local";
  }
  char const* GetInstallStripTargetName() const override
  {
    return "install/strip";
  }
  char const* GetInstallParallelTargetName() const
  {
    return "install/parallel";
  }
  char const* GetTestTargetName() const override { return "RUN_TESTS"; }
  char const* GetPackageTargetName() const override { return "package"; }
  char const* GetPackageSourceTargetName() const override
  {
    return "package_source";
  }
  char const* GetRebuildCacheTargetName() const override
  {
    return "rebuild_cache";
  }
  char const* GetCMakeCFGIntDir() const override { return "."; }

  /// Overloaded methods. @see cmGlobalGenerator::GetDocumentation()
  static cmDocumentationEntry GetDocumentation();

  static bool SupportsToolset() { return false; }

  static bool SupportsPlatform() { return false; }

  bool IsIPOSupported() const override { return true; }

  void OpenBuildFileStream();
  void CloseBuildFileStream();

  std::vector<std::string> const& GetConfigNames() const;

  bool Open(std::string const& bindir, std::string const& projectName,
            bool dryRun) override;

  std::string ConvertToFastbuildPath(std::string const& path) const;
  std::unique_ptr<cmLinkLineComputer> CreateLinkLineComputer(
    cmOutputConverter* outputConverter,
    cmStateDirectory const& /* stateDir */) const override;

  bool SupportsCustomCommandDepfile() const override { return true; }
  cm::optional<cmDepfileFormat> DepfileFormat() const override
  {
    return cm::nullopt;
  }

  static std::string Quote(std::string const& str,
                           std::string const& quotation = "'");
  static std::string QuoteIfHasSpaces(std::string str);

  template <class T>
  static std::vector<std::string> Wrap(T const& in,
                                       std::string const& prefix = "'",
                                       std::string const& suffix = "'",
                                       bool escape_dollar = true);

  void WriteDivider();
  void WriteComment(std::string const& comment, int indent = 0);

  /// Write @a count times INDENT level to output stream @a os.
  void Indent(int count);

  void WriteVariable(std::string const& key, std::string const& value,
                     std::string const& op, int indent = 0);
  void WriteVariable(std::string const& key, std::string const& value,
                     int indent = 0);
  void WriteCommand(std::string const& command,
                    std::string const& value = std::string(), int indent = 0);
  void WriteArray(std::string const& key,
                  std::vector<std::string> const& values, int indent = 0);
  void WriteStruct(
    std::string const& name,
    std::vector<std::pair<std::string, std::string>> const& variables,
    int indent = 0);
  void WriteArray(std::string const& key,
                  std::vector<std::string> const& values,
                  std::string const& op, int indent = 0);

  template <typename T>
  std::vector<std::string> ConvertToFastbuildPath(T const& container) const
  {
    std::vector<std::string> ret;
    ret.reserve(container.size());
    for (auto const& path : container) {
      ret.push_back(ConvertToFastbuildPath(path));
    }
    return ret;
  }

  // Wrapper to sort array of conforming structs (which have .Name
  // and .PreBuildDependencies fields).
  template <class T>
  static void TopologicalSort(std::vector<T>& nodes)
  {
    static_assert(std::is_base_of<FastbuildTargetBase, T>::value,
                  "T must be derived from FastbuildTargetBase");
    std::vector<FastbuildTargetPtrT> tmp;
    tmp.reserve(nodes.size());
    for (auto& node : nodes) {
      tmp.emplace_back(cm::make_unique<T>(std::move(node)));
    }
    nodes.clear();
    TopologicalSort(tmp);
    for (auto& node : tmp) {
      nodes.emplace_back(std::move(static_cast<T&>(*node)));
    }
  }
  // Stable topological sort.
  static void TopologicalSort(std::vector<FastbuildTargetPtrT>& nodes);

  void WriteDisclaimer();
  void WriteEnvironment();
  void WriteSettings();
  void WriteCompilers();
  void WriteTargets();

  void WriteTarget(FastbuildTarget const& target);
  void WriteExec(FastbuildExecNode const& Exec, int indent = 1);
  void WriteUnity(FastbuildUnityNode const& Unity);
  void WriteObjectList(FastbuildObjectListNode const& ObjectList,
                       bool allowDistribution);
  void WriteLinker(FastbuildLinkerNode const&, bool);
  void WriteAlias(FastbuildAliasNode const& Alias, int indent = 1);
  void WriteCopy(FastbuildCopyNode const& Copy);

  void WriteIDEProjects();
  std::string GetIDEBuildArgs() const;
  void WriteVSBuildCommands();
  void WriteXCodeBuildCommands();
  void WriteIDEProjectCommon(IDEProjectCommon const& project);
  void WriteIDEProjectConfig(std::vector<IDEProjectConfig> const& configs,
                             std::string const& keyName = "ProjectConfigs");

  void WriteSolution();
  void WriteXCodeTopLevelProject();
  void WriteTargetRebuildBFF();
  void WriteCleanScript();
  void WriteTargetClean();

  void AddTargetAll();
  void AddGlobCheckExec();

  void AddCompiler(std::string const& lang, cmMakefile* mf);
  void AddLauncher(std::string const& prefix, std::string const& launcher,
                   std::string const& lang, std::string const& args);
  void AddIDEProject(FastbuildTargetBase const& target,
                     std::string const& config);

  template <class T>
  void AddTarget(T target)
  {
    // Sometimes equivalent CCs are added to different targets. We try to
    // de-dup it by assigning all execs a name which is a hash computed based
    // on various properties (like input, output, deps.). Apparently, there are
    // still some CCs intersection between different targets.
    auto val = AllGeneratedCommands.emplace(target.Name);
    if (val.second) {
      FastbuildTargets.emplace_back(cm::make_unique<T>(std::move(target)));
    }
    // Get the intersection of CC's deps. Just mimicking what
    // cmLocalNinjaGenerator::WriteCustomCommandBuildStatement does. (I don't
    // think it's right in general case, each CC should be added only to 1
    // target, not to multiple )
    else {
      auto it =
        std::find_if(FastbuildTargets.begin(), FastbuildTargets.end(),
                     [&target](FastbuildTargetPtrT const& existingTarget) {
                       return existingTarget->Name == target.Name;
                     });
      assert(it != FastbuildTargets.end());

      std::set<FastbuildTargetDep> intersection;
      std::set_intersection(
        target.PreBuildDependencies.begin(), target.PreBuildDependencies.end(),
        (*it)->PreBuildDependencies.begin(), (*it)->PreBuildDependencies.end(),
        std::inserter(intersection, intersection.end()));
      (*it)->PreBuildDependencies = std::move(intersection);
    }
  }

  static std::string GetExternalShellExecutable();
  std::string GetTargetName(cmGeneratorTarget const* GeneratorTarget) const;
  cm::optional<FastbuildTarget> GetTargetByOutputName(
    std::string const& output) const;
  void AskCMakeToMakeRebuildBFFUpToDate(std::string const& workingDir) const;
  void ExecuteFastbuildTarget(
    std::string const& dir, std::string const& target, std::string& output,
    std::vector<std::string> const& fbuildOptions = {}) const;

  bool IsExcluded(cmGeneratorTarget* target);

  void LogMessage(std::string const& m) const;

  void AddFileToClean(std::string const& file);

  /// The set of compilers added to the generated build system.
  std::map<std::string, FastbuildCompiler> Compilers;

  std::vector<FastbuildTargetPtrT> FastbuildTargets;

  /// The file containing the build statement.
  std::unique_ptr<cmGeneratedFileStream> BuildFileStream;

  std::string FastbuildCommand;
  std::string FastbuildVersion;

  std::map<std::string, std::unique_ptr<cmFastbuildTargetGenerator>> Targets;

  std::unordered_set<std::string> AllFoldersToClean;
  // Sometime we need to keep some files that are generated only during
  // configuration (like .objs files used to create module definition from
  // objects).
  std::unordered_set<std::string> AllFilesToKeep;
  bool UsingRelativePaths = false;

private:
  std::unordered_set<std::string> AllFilesToClean;
  // https://cmake.org/cmake/help/latest/module/ExternalProject.html#command:externalproject_add_steptargets
  std::unordered_set<std::string /*exec name*/> AllGeneratedCommands;

  std::unordered_map<std::string /*base target name (without -config)*/,
                     std::pair<VCXProject, XCodeProject>>
    IDEProjects;
  // Env that we're going to embed to the generated file.
  std::vector<std::string> LocalEnvironment;
};
