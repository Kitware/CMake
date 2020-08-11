/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmGlobalNinjaGenerator_h
#define cmGlobalNinjaGenerator_h

#include "cmConfigure.h" // IWYU pragma: keep

#include <iosfwd>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include <cm/optional>

#include "cm_codecvt.hxx"

#include "cmGeneratedFileStream.h"
#include "cmGlobalCommonGenerator.h"
#include "cmGlobalGeneratorFactory.h"
#include "cmNinjaTypes.h"
#include "cmPolicies.h"
#include "cmStringAlgorithms.h"

class cmCustomCommand;
class cmGeneratorTarget;
class cmLinkLineComputer;
class cmLocalGenerator;
class cmMakefile;
class cmOutputConverter;
class cmState;
class cmStateDirectory;
class cmake;
struct cmDocumentationEntry;

/**
 * \class cmGlobalNinjaGenerator
 * \brief Write a build.ninja file.
 *
 * The main differences between this generator and the UnixMakefile
 * generator family are:
 * - We don't care about VERBOSE variable or RULE_MESSAGES property since
 *   it is handle by Ninja's -v option.
 * - We don't care about computing any progress status since Ninja manages
 *   it itself.
 * - We generate one build.ninja and one rules.ninja per project.
 * - We try to minimize the number of generated rules: one per target and
 *   language.
 * - We use Ninja special variable $in and $out to produce nice output.
 * - We extensively use Ninja variable overloading system to minimize the
 *   number of generated rules.
 */
class cmGlobalNinjaGenerator : public cmGlobalCommonGenerator
{
public:
  /// The default name of Ninja's build file. Typically: build.ninja.
  static const char* NINJA_BUILD_FILE;

  /// The default name of Ninja's rules file. Typically: rules.ninja.
  /// It is included in the main build.ninja file.
  static const char* NINJA_RULES_FILE;

  /// The indentation string used when generating Ninja's build file.
  static const char* INDENT;

  /// The shell command used for a no-op.
  static std::string const SHELL_NOOP;

  /// Write @a count times INDENT level to output stream @a os.
  static void Indent(std::ostream& os, int count);

  /// Write a divider in the given output stream @a os.
  static void WriteDivider(std::ostream& os);

  static std::string EncodeRuleName(std::string const& name);
  std::string EncodeLiteral(const std::string& lit);
  std::string EncodePath(const std::string& path);

  std::unique_ptr<cmLinkLineComputer> CreateLinkLineComputer(
    cmOutputConverter* outputConverter,
    cmStateDirectory const& stateDir) const override;

  /**
   * Write the given @a comment to the output stream @a os. It
   * handles new line character properly.
   */
  static void WriteComment(std::ostream& os, const std::string& comment);

  /**
   * Utilized by the generator factory to determine if this generator
   * supports toolsets.
   */
  static bool SupportsToolset() { return false; }

  /**
   * Utilized by the generator factory to determine if this generator
   * supports platforms.
   */
  static bool SupportsPlatform() { return false; }

  bool IsIPOSupported() const override { return true; }

  /**
   * Write a build statement @a build to @a os.
   * @warning no escaping of any kind is done here.
   */
  void WriteBuild(std::ostream& os, cmNinjaBuild const& build,
                  int cmdLineLimit = 0, bool* usedResponseFile = nullptr);

  void WriteCustomCommandBuild(
    const std::string& command, const std::string& description,
    const std::string& comment, const std::string& depfile,
    const std::string& pool, bool uses_terminal, bool restat,
    const cmNinjaDeps& outputs, const std::string& config,
    const cmNinjaDeps& explicitDeps = cmNinjaDeps(),
    const cmNinjaDeps& orderOnlyDeps = cmNinjaDeps());

  void WriteMacOSXContentBuild(std::string input, std::string output,
                               const std::string& config);

  /**
   * Write a rule statement to @a os.
   * @warning no escaping of any kind is done here.
   */
  static void WriteRule(std::ostream& os, cmNinjaRule const& rule);

  /**
   * Write a variable named @a name to @a os with value @a value and an
   * optional @a comment. An @a indent level can be specified.
   * @warning no escaping of any kind is done here.
   */
  static void WriteVariable(std::ostream& os, const std::string& name,
                            const std::string& value,
                            const std::string& comment = "", int indent = 0);

  /**
   * Write an include statement including @a filename with an optional
   * @a comment to the @a os stream.
   */
  static void WriteInclude(std::ostream& os, const std::string& filename,
                           const std::string& comment = "");

  /**
   * Write a default target statement specifying @a targets as
   * the default targets.
   */
  static void WriteDefault(std::ostream& os, const cmNinjaDeps& targets,
                           const std::string& comment = "");

  bool IsGCCOnWindows() const { return UsingGCCOnWindows; }

public:
  cmGlobalNinjaGenerator(cmake* cm);

  static std::unique_ptr<cmGlobalGeneratorFactory> NewFactory()
  {
    return std::unique_ptr<cmGlobalGeneratorFactory>(
      new cmGlobalGeneratorSimpleFactory<cmGlobalNinjaGenerator>());
  }

  std::unique_ptr<cmLocalGenerator> CreateLocalGenerator(
    cmMakefile* mf) override;

  std::string GetName() const override
  {
    return cmGlobalNinjaGenerator::GetActualName();
  }

  static std::string GetActualName() { return "Ninja"; }

  /** Get encoding used by generator for ninja files */
  codecvt::Encoding GetMakefileEncoding() const override;

  static void GetDocumentation(cmDocumentationEntry& entry);

  void EnableLanguage(std::vector<std::string> const& languages,
                      cmMakefile* mf, bool optional) override;

  std::vector<GeneratedMakeCommand> GenerateBuildCommand(
    const std::string& makeProgram, const std::string& projectName,
    const std::string& projectDir, std::vector<std::string> const& targetNames,
    const std::string& config, bool fast, int jobs, bool verbose,
    std::vector<std::string> const& makeOptions =
      std::vector<std::string>()) override;

  // Setup target names
  const char* GetAllTargetName() const override { return "all"; }
  const char* GetInstallTargetName() const override { return "install"; }
  const char* GetInstallLocalTargetName() const override
  {
    return "install/local";
  }
  const char* GetInstallStripTargetName() const override
  {
    return "install/strip";
  }
  const char* GetTestTargetName() const override { return "test"; }
  const char* GetPackageTargetName() const override { return "package"; }
  const char* GetPackageSourceTargetName() const override
  {
    return "package_source";
  }
  const char* GetEditCacheTargetName() const override { return "edit_cache"; }
  const char* GetRebuildCacheTargetName() const override
  {
    return "rebuild_cache";
  }
  const char* GetCleanTargetName() const override { return "clean"; }

  bool SupportsCustomCommandDepfile() const override { return true; }

  virtual cmGeneratedFileStream* GetImplFileStream(
    const std::string& /*config*/) const
  {
    return this->BuildFileStream.get();
  }

  virtual cmGeneratedFileStream* GetConfigFileStream(
    const std::string& /*config*/) const
  {
    return this->BuildFileStream.get();
  }

  virtual cmGeneratedFileStream* GetDefaultFileStream() const
  {
    return this->BuildFileStream.get();
  }

  virtual cmGeneratedFileStream* GetCommonFileStream() const
  {
    return this->BuildFileStream.get();
  }

  cmGeneratedFileStream* GetRulesFileStream() const
  {
    return this->RulesFileStream.get();
  }

  std::string const& ConvertToNinjaPath(const std::string& path) const;

  struct MapToNinjaPathImpl
  {
    cmGlobalNinjaGenerator* GG;
    MapToNinjaPathImpl(cmGlobalNinjaGenerator* gg)
      : GG(gg)
    {
    }
    std::string operator()(std::string const& path)
    {
      return this->GG->ConvertToNinjaPath(path);
    }
  };
  MapToNinjaPathImpl MapToNinjaPath() { return { this }; }

  // -- Additional clean files
  void AddAdditionalCleanFile(std::string fileName, const std::string& config);
  const char* GetAdditionalCleanTargetName() const
  {
    return "CMakeFiles/clean.additional";
  }

  static const char* GetByproductsForCleanTargetName()
  {
    return "CMakeFiles/cmake_byproducts_for_clean_target";
  }

  void AddCXXCompileCommand(const std::string& commandLine,
                            const std::string& sourceFile);

  /**
   * Add a rule to the generated build system.
   * Call WriteRule() behind the scene but perform some check before like:
   * - Do not add twice the same rule.
   */
  void AddRule(cmNinjaRule const& rule);

  bool HasRule(const std::string& name);

  void AddCustomCommandRule();
  void AddMacOSXContentRule();

  bool HasCustomCommandOutput(const std::string& output)
  {
    return this->CustomCommandOutputs.find(output) !=
      this->CustomCommandOutputs.end();
  }

  /// Called when we have seen the given custom command.  Returns true
  /// if we has seen it before.
  bool SeenCustomCommand(cmCustomCommand const* cc, const std::string& config)
  {
    return !this->Configs[config].CustomCommands.insert(cc).second;
  }

  /// Called when we have seen the given custom command output.
  void SeenCustomCommandOutput(const std::string& output)
  {
    this->CustomCommandOutputs.insert(output);
    // We don't need the assumed dependencies anymore, because we have
    // an output.
    this->AssumedSourceDependencies.erase(output);
  }

  void AddAssumedSourceDependencies(const std::string& source,
                                    const cmNinjaDeps& deps)
  {
    std::set<std::string>& ASD = this->AssumedSourceDependencies[source];
    // Because we may see the same source file multiple times (same source
    // specified in multiple targets), compute the union of any assumed
    // dependencies.
    ASD.insert(deps.begin(), deps.end());
  }

  virtual std::string OrderDependsTargetForTarget(
    cmGeneratorTarget const* target, const std::string& config) const;

  void AppendTargetOutputs(
    cmGeneratorTarget const* target, cmNinjaDeps& outputs,
    const std::string& config,
    cmNinjaTargetDepends depends = DependOnTargetArtifact);
  void AppendTargetDepends(
    cmGeneratorTarget const* target, cmNinjaDeps& outputs,
    const std::string& config, const std::string& fileConfig,
    cmNinjaTargetDepends depends = DependOnTargetArtifact);
  void AppendTargetDependsClosure(cmGeneratorTarget const* target,
                                  cmNinjaDeps& outputs,
                                  const std::string& config);
  void AppendTargetDependsClosure(cmGeneratorTarget const* target,
                                  cmNinjaOuts& outputs,
                                  const std::string& config, bool omit_self);

  void AppendDirectoryForConfig(const std::string& prefix,
                                const std::string& config,
                                const std::string& suffix,
                                std::string& dir) override;

  virtual void AppendNinjaFileArgument(GeneratedMakeCommand& /*command*/,
                                       const std::string& /*config*/) const
  {
  }

  virtual void AddRebuildManifestOutputs(cmNinjaDeps& outputs) const
  {
    outputs.push_back(this->NinjaOutputPath(NINJA_BUILD_FILE));
  }

  int GetRuleCmdLength(const std::string& name) { return RuleCmdLength[name]; }

  void AddTargetAlias(const std::string& alias, cmGeneratorTarget* target,
                      const std::string& config);

  void ComputeTargetObjectDirectory(cmGeneratorTarget* gt) const override;

  // Ninja generator uses 'deps' and 'msvc_deps_prefix' introduced in 1.3
  static std::string RequiredNinjaVersion() { return "1.3"; }
  static std::string RequiredNinjaVersionForConsolePool() { return "1.5"; }
  static std::string RequiredNinjaVersionForImplicitOuts() { return "1.7"; }
  static std::string RequiredNinjaVersionForManifestRestat() { return "1.8"; }
  static std::string RequiredNinjaVersionForMultilineDepfile()
  {
    return "1.9";
  }
  static std::string RequiredNinjaVersionForDyndeps() { return "1.10"; }
  static std::string RequiredNinjaVersionForRestatTool() { return "1.10"; }
  static std::string RequiredNinjaVersionForUnconditionalRecompactTool()
  {
    return "1.10";
  }
  static std::string RequiredNinjaVersionForCleanDeadTool() { return "1.10"; }
  bool SupportsConsolePool() const;
  bool SupportsImplicitOuts() const;
  bool SupportsManifestRestat() const;
  bool SupportsMultilineDepfile() const;

  std::string NinjaOutputPath(std::string const& path) const;
  bool HasOutputPathPrefix() const { return !this->OutputPathPrefix.empty(); }
  void StripNinjaOutputPathPrefixAsSuffix(std::string& path);

  bool WriteDyndepFile(std::string const& dir_top_src,
                       std::string const& dir_top_bld,
                       std::string const& dir_cur_src,
                       std::string const& dir_cur_bld,
                       std::string const& arg_dd,
                       std::vector<std::string> const& arg_ddis,
                       std::string const& module_dir,
                       std::vector<std::string> const& linked_target_dirs,
                       std::string const& arg_lang);

  virtual std::string BuildAlias(const std::string& alias,
                                 const std::string& /*config*/) const
  {
    return alias;
  }

  virtual std::string ConfigDirectory(const std::string& /*config*/) const
  {
    return "";
  }

  cmNinjaDeps& GetByproductsForCleanTarget()
  {
    return this->ByproductsForCleanTarget;
  }

  cmNinjaDeps& GetByproductsForCleanTarget(const std::string& config)
  {
    return this->Configs[config].ByproductsForCleanTarget;
  }

  bool EnableCrossConfigBuild() const;

  std::set<std::string> GetCrossConfigs(const std::string& config) const;

  const std::set<std::string>& GetDefaultConfigs() const
  {
    return this->DefaultConfigs;
  }

protected:
  void Generate() override;

  bool CheckALLOW_DUPLICATE_CUSTOM_TARGETS() const override { return true; }

  virtual bool OpenBuildFileStreams();
  virtual void CloseBuildFileStreams();

  bool OpenFileStream(std::unique_ptr<cmGeneratedFileStream>& stream,
                      const std::string& name);

  static cm::optional<std::set<std::string>> ListSubsetWithAll(
    const std::set<std::string>& all, const std::set<std::string>& defaults,
    const std::vector<std::string>& items);

  virtual bool InspectConfigTypeVariables() { return true; }

  std::set<std::string> CrossConfigs;
  std::set<std::string> DefaultConfigs;
  std::string DefaultFileConfig;

private:
  std::string GetEditCacheCommand() const override;
  bool FindMakeProgram(cmMakefile* mf) override;
  void CheckNinjaFeatures();
  bool CheckLanguages(std::vector<std::string> const& languages,
                      cmMakefile* mf) const override;
  bool CheckFortran(cmMakefile* mf) const;

  void CloseCompileCommandsStream();

  bool OpenRulesFileStream();
  void CloseRulesFileStream();
  void CleanMetaData();

  /// Write the common disclaimer text at the top of each build file.
  void WriteDisclaimer(std::ostream& os);

  void WriteAssumedSourceDependencies();

  void WriteTargetAliases(std::ostream& os);
  void WriteFolderTargets(std::ostream& os);
  void WriteUnknownExplicitDependencies(std::ostream& os);

  void WriteBuiltinTargets(std::ostream& os);
  void WriteTargetDefault(std::ostream& os);
  void WriteTargetRebuildManifest(std::ostream& os);
  bool WriteTargetCleanAdditional(std::ostream& os);
  void WriteTargetClean(std::ostream& os);
  void WriteTargetHelp(std::ostream& os);

  void ComputeTargetDependsClosure(
    cmGeneratorTarget const* target,
    std::set<cmGeneratorTarget const*>& depends);

  std::string CMakeCmd() const;
  std::string NinjaCmd() const;

  /// The file containing the build statement. (the relationship of the
  /// compilation DAG).
  std::unique_ptr<cmGeneratedFileStream> BuildFileStream;
  /// The file containing the rule statements. (The action attached to each
  /// edge of the compilation DAG).
  std::unique_ptr<cmGeneratedFileStream> RulesFileStream;
  std::unique_ptr<cmGeneratedFileStream> CompileCommandsStream;

  /// The set of rules added to the generated build system.
  std::unordered_set<std::string> Rules;

  /// Length of rule command, used by rsp file evaluation
  std::unordered_map<std::string, int> RuleCmdLength;

  bool UsingGCCOnWindows = false;

  /// The set of custom command outputs we have seen.
  std::set<std::string> CustomCommandOutputs;

  /// Whether we are collecting known build outputs and needed
  /// dependencies to determine unknown dependencies.
  bool ComputingUnknownDependencies = false;
  cmPolicies::PolicyStatus PolicyCMP0058 = cmPolicies::WARN;

  /// The combined explicit dependencies of custom build commands
  std::set<std::string> CombinedCustomCommandExplicitDependencies;

  /// When combined with CombinedCustomCommandExplicitDependencies it allows
  /// us to detect the set of explicit dependencies that have
  std::set<std::string> CombinedBuildOutputs;

  /// The mapping from source file to assumed dependencies.
  std::map<std::string, std::set<std::string>> AssumedSourceDependencies;

  struct TargetAlias
  {
    cmGeneratorTarget* GeneratorTarget;
    std::string Config;
  };
  using TargetAliasMap = std::map<std::string, TargetAlias>;
  TargetAliasMap TargetAliases;
  TargetAliasMap DefaultTargetAliases;

  /// the local cache for calls to ConvertToNinjaPath
  mutable std::unordered_map<std::string, std::string> ConvertToNinjaPathCache;

  std::string NinjaCommand;
  std::string NinjaVersion;
  bool NinjaSupportsConsolePool = false;
  bool NinjaSupportsImplicitOuts = false;
  bool NinjaSupportsManifestRestat = false;
  bool NinjaSupportsMultilineDepfile = false;
  bool NinjaSupportsDyndeps = false;
  bool NinjaSupportsRestatTool = false;
  bool NinjaSupportsUnconditionalRecompactTool = false;
  bool NinjaSupportsCleanDeadTool = false;

private:
  void InitOutputPathPrefix();

  std::string OutputPathPrefix;
  std::string TargetAll;
  std::string CMakeCacheFile;

  struct ByConfig
  {
    std::set<std::string> AdditionalCleanFiles;

    /// The set of custom commands we have seen.
    std::set<cmCustomCommand const*> CustomCommands;

    std::map<cmGeneratorTarget const*, cmNinjaOuts> TargetDependsClosures;

    TargetAliasMap TargetAliases;

    cmNinjaDeps ByproductsForCleanTarget;
  };
  std::map<std::string, ByConfig> Configs;

  cmNinjaDeps ByproductsForCleanTarget;
};

class cmGlobalNinjaMultiGenerator : public cmGlobalNinjaGenerator
{
public:
  /// The default name of Ninja's common file. Typically: common.ninja.
  static const char* NINJA_COMMON_FILE;
  /// The default file extension to use for per-config Ninja files.
  static const char* NINJA_FILE_EXTENSION;

  cmGlobalNinjaMultiGenerator(cmake* cm);
  bool IsMultiConfig() const override { return true; }
  static std::unique_ptr<cmGlobalGeneratorFactory> NewFactory()
  {
    return std::unique_ptr<cmGlobalGeneratorFactory>(
      new cmGlobalGeneratorSimpleFactory<cmGlobalNinjaMultiGenerator>());
  }

  static void GetDocumentation(cmDocumentationEntry& entry);

  std::string GetName() const override
  {
    return cmGlobalNinjaMultiGenerator::GetActualName();
  }

  static std::string GetActualName() { return "Ninja Multi-Config"; }

  std::string BuildAlias(const std::string& alias,
                         const std::string& config) const override
  {
    if (config.empty()) {
      return alias;
    }
    return cmStrCat(alias, ":", config);
  }

  std::string ConfigDirectory(const std::string& config) const override
  {
    if (!config.empty()) {
      return cmStrCat('/', config);
    }
    return "";
  }

  const char* GetCMakeCFGIntDir() const override { return "${CONFIGURATION}"; }

  std::string ExpandCFGIntDir(const std::string& str,
                              const std::string& config) const override;

  cmGeneratedFileStream* GetImplFileStream(
    const std::string& config) const override
  {
    return this->ImplFileStreams.at(config).get();
  }

  cmGeneratedFileStream* GetConfigFileStream(
    const std::string& config) const override
  {
    return this->ConfigFileStreams.at(config).get();
  }

  cmGeneratedFileStream* GetDefaultFileStream() const override
  {
    return this->DefaultFileStream.get();
  }

  cmGeneratedFileStream* GetCommonFileStream() const override
  {
    return this->CommonFileStream.get();
  }

  void AppendNinjaFileArgument(GeneratedMakeCommand& command,
                               const std::string& config) const override;

  static std::string GetNinjaImplFilename(const std::string& config);
  static std::string GetNinjaConfigFilename(const std::string& config);

  void AddRebuildManifestOutputs(cmNinjaDeps& outputs) const override;

  void GetQtAutoGenConfigs(std::vector<std::string>& configs) const override;

  bool InspectConfigTypeVariables() override;

  std::string GetDefaultBuildConfig() const override;

  bool ReadCacheEntriesForBuild(const cmState& state) override;

  bool SupportsDefaultBuildType() const override { return true; }
  bool SupportsCrossConfigs() const override { return true; }
  bool SupportsDefaultConfigs() const override { return true; }

  std::string OrderDependsTargetForTarget(
    cmGeneratorTarget const* target, const std::string& config) const override;

protected:
  bool OpenBuildFileStreams() override;
  void CloseBuildFileStreams() override;

private:
  std::map<std::string, std::unique_ptr<cmGeneratedFileStream>>
    ImplFileStreams;
  std::map<std::string, std::unique_ptr<cmGeneratedFileStream>>
    ConfigFileStreams;
  std::unique_ptr<cmGeneratedFileStream> CommonFileStream;
  std::unique_ptr<cmGeneratedFileStream> DefaultFileStream;
};

#endif // ! cmGlobalNinjaGenerator_h
