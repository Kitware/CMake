/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

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

#include "cm_codecvt_Encoding.hxx"

#include "cmBuildOptions.h"
#include "cmGeneratedFileStream.h"
#include "cmGlobalCommonGenerator.h"
#include "cmGlobalGeneratorFactory.h"
#include "cmNinjaTypes.h"
#include "cmStringAlgorithms.h"
#include "cmTransformDepfile.h"

class cmCustomCommand;
class cmGeneratorTarget;
class cmLinkLineComputer;
class cmLocalGenerator;
class cmMakefile;
class cmOutputConverter;
class cmStateDirectory;
class cmake;
struct cmCxxModuleExportInfo;

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
  static char const* NINJA_BUILD_FILE;

  /// The default name of Ninja's rules file. Typically: rules.ninja.
  /// It is included in the main build.ninja file.
  static char const* NINJA_RULES_FILE;

  /// The indentation string used when generating Ninja's build file.
  static char const* INDENT;

  /// The shell command used for a no-op.
  static std::string const SHELL_NOOP;

  /// Write @a count times INDENT level to output stream @a os.
  static void Indent(std::ostream& os, int count);

  /// Write a divider in the given output stream @a os.
  static void WriteDivider(std::ostream& os);

  static std::string EncodeRuleName(std::string const& name);
  std::string& EncodeLiteral(std::string& lit) override;
  std::string GetEncodedLiteral(std::string const& lit);
  std::string EncodePath(std::string const& path);

  std::unique_ptr<cmLinkLineComputer> CreateLinkLineComputer(
    cmOutputConverter* outputConverter,
    cmStateDirectory const& stateDir) const override;

  /**
   * Write the given @a comment to the output stream @a os. It
   * handles new line character properly.
   */
  static void WriteComment(std::ostream& os, std::string const& comment);

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

  class CCOutputs
  {
    cmGlobalNinjaGenerator* GG;

  public:
    CCOutputs(cmGlobalNinjaGenerator* gg)
      : GG(gg)
    {
    }
    void Add(std::vector<std::string> const& outputs);
    cmNinjaDeps ExplicitOuts;
    cmNinjaDeps WorkDirOuts;
  };

  void WriteCustomCommandBuild(std::string const& command,
                               std::string const& description,
                               std::string const& comment,
                               std::string const& depfile,
                               std::string const& pool, bool uses_terminal,
                               bool restat, std::string const& config,
                               CCOutputs outputs,
                               cmNinjaDeps explicitDeps = cmNinjaDeps(),
                               cmNinjaDeps orderOnlyDeps = cmNinjaDeps());

  void WriteMacOSXContentBuild(std::string input, std::string output,
                               std::string const& config);

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
  static void WriteVariable(std::ostream& os, std::string const& name,
                            std::string const& value,
                            std::string const& comment = "", int indent = 0);

  /**
   * Write an include statement including @a filename with an optional
   * @a comment to the @a os stream.
   */
  static void WriteInclude(std::ostream& os, std::string const& filename,
                           std::string const& comment = "");

  /**
   * Write a default target statement specifying @a targets as
   * the default targets.
   */
  static void WriteDefault(std::ostream& os, cmNinjaDeps const& targets,
                           std::string const& comment = "");

  bool IsGCCOnWindows() const { return this->UsingGCCOnWindows; }
  void MarkAsGCCOnWindows() { this->UsingGCCOnWindows = true; }

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

  bool IsNinja() const override { return true; }

  /** Get encoding used by generator for ninja files */
  codecvt_Encoding GetMakefileEncoding() const override;

  static cmDocumentationEntry GetDocumentation();

  void EnableLanguage(std::vector<std::string> const& languages,
                      cmMakefile* mf, bool optional) override;

  std::vector<GeneratedMakeCommand> GenerateBuildCommand(
    std::string const& makeProgram, std::string const& projectName,
    std::string const& projectDir, std::vector<std::string> const& targetNames,
    std::string const& config, int jobs, bool verbose,
    cmBuildOptions const& buildOptions = cmBuildOptions(),
    std::vector<std::string> const& makeOptions =
      std::vector<std::string>()) override;

  // Setup target names
  char const* GetAllTargetName() const override { return "all"; }
  char const* GetInstallTargetName() const override { return "install"; }
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
  char const* GetTestTargetName() const override { return "test"; }
  char const* GetPackageTargetName() const override { return "package"; }
  char const* GetPackageSourceTargetName() const override
  {
    return "package_source";
  }
  char const* GetRebuildCacheTargetName() const override
  {
    return "rebuild_cache";
  }
  char const* GetCleanTargetName() const override { return "clean"; }

  bool SupportsCustomCommandDepfile() const override { return true; }
  cm::optional<cmDepfileFormat> DepfileFormat() const override
  {
    return cmDepfileFormat::GccDepfile;
  }

  bool SupportsLinkerDependencyFile() const override { return true; }

  virtual cmGeneratedFileStream* GetImplFileStream(
    std::string const& /*config*/) const
  {
    return this->BuildFileStream.get();
  }

  virtual cmGeneratedFileStream* GetConfigFileStream(
    std::string const& /*config*/) const
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

  std::string const& ConvertToNinjaPath(std::string const& path) const;
  std::string ConvertToNinjaAbsPath(std::string path) const;

  struct MapToNinjaPathImpl
  {
    cmGlobalNinjaGenerator* GG;
    MapToNinjaPathImpl(cmGlobalNinjaGenerator* gg)
      : GG(gg)
    {
    }
    std::string operator()(std::string const& path) const
    {
      return this->GG->ConvertToNinjaPath(path);
    }
  };
  MapToNinjaPathImpl MapToNinjaPath() { return { this }; }

#ifdef _WIN32
  std::string const& GetComspec() const { return this->Comspec; }
#endif

  // -- Additional clean files
  void AddAdditionalCleanFile(std::string fileName, std::string const& config);
  char const* GetAdditionalCleanTargetName() const
  {
    return "CMakeFiles/clean.additional";
  }

  static char const* GetByproductsForCleanTargetName()
  {
    return "CMakeFiles/cmake_byproducts_for_clean_target";
  }

  void AddCXXCompileCommand(std::string const& commandLine,
                            std::string const& sourceFile,
                            std::string const& objPath);

  /**
   * Add a rule to the generated build system.
   * Call WriteRule() behind the scene but perform some check before like:
   * - Do not add twice the same rule.
   */
  void AddRule(cmNinjaRule const& rule);

  bool HasRule(std::string const& name);

  void AddCustomCommandRule();
  void AddMacOSXContentRule();

  bool HasCustomCommandOutput(std::string const& output)
  {
    return this->CustomCommandOutputs.find(output) !=
      this->CustomCommandOutputs.end();
  }

  /// Called when we have seen the given custom command.  Returns true
  /// if we has seen it before.
  bool SeenCustomCommand(cmCustomCommand const* cc, std::string const& config)
  {
    return !this->Configs[config].CustomCommands.insert(cc).second;
  }

  /// Called when we have seen the given custom command output.
  void SeenCustomCommandOutput(std::string const& output)
  {
    this->CustomCommandOutputs.insert(output);
    // We don't need the assumed dependencies anymore, because we have
    // an output.
    this->AssumedSourceDependencies.erase(output);
  }

  void AddAssumedSourceDependencies(std::string const& source,
                                    cmNinjaDeps const& deps)
  {
    std::set<std::string>& ASD = this->AssumedSourceDependencies[source];
    // Because we may see the same source file multiple times (same source
    // specified in multiple targets), compute the union of any assumed
    // dependencies.
    ASD.insert(deps.begin(), deps.end());
  }

  virtual std::string OrderDependsTargetForTarget(
    cmGeneratorTarget const* target, std::string const& config) const;

  std::string OrderDependsTargetForTargetPrivate(
    cmGeneratorTarget const* target, std::string const& config) const;

  void AppendTargetOutputs(cmGeneratorTarget const* target,
                           cmNinjaDeps& outputs, std::string const& config,
                           cmNinjaTargetDepends depends) const;
  void AppendTargetDepends(cmGeneratorTarget const* target,
                           cmNinjaDeps& outputs, std::string const& config,
                           std::string const& fileConfig,
                           cmNinjaTargetDepends depends);
  void AppendTargetDependsClosure(cmGeneratorTarget const* target,
                                  std::unordered_set<std::string>& outputs,
                                  std::string const& config,
                                  std::string const& fileConfig,
                                  bool genexOutput, bool omit_self = true);

  void AppendDirectoryForConfig(std::string const& prefix,
                                std::string const& config,
                                std::string const& suffix,
                                std::string& dir) override;

  virtual void AppendNinjaFileArgument(GeneratedMakeCommand& /*command*/,
                                       std::string const& /*config*/) const
  {
  }

  virtual void AddRebuildManifestOutputs(cmNinjaDeps& outputs) const
  {
    outputs.push_back(this->NinjaOutputPath(NINJA_BUILD_FILE));
    this->AddCMakeFilesToRebuild(outputs);
  }

  int GetRuleCmdLength(std::string const& name)
  {
    return this->RuleCmdLength[name];
  }

  void AddTargetAlias(std::string const& alias, cmGeneratorTarget* target,
                      std::string const& config);

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
  static std::string RequiredNinjaVersionForDyndepsCxx() { return "1.11"; }
  static std::string RequiredNinjaVersionForDyndepsFortran() { return "1.10"; }
  static std::string RequiredNinjaVersionForRestatTool() { return "1.10"; }
  static std::string RequiredNinjaVersionForUnconditionalRecompactTool()
  {
    return "1.10";
  }
  static std::string RequiredNinjaVersionForMultipleOutputs()
  {
    return "1.10";
  }
  static std::string RequiredNinjaVersionForMetadataOnRegeneration()
  {
    return "1.10.2";
  }
  static std::string RequiredNinjaVersionForCodePage() { return "1.11"; }
  static std::string RequiredNinjaVersionForCWDDepend() { return "1.7"; }
  bool SupportsDirectConsole() const override;
  bool SupportsImplicitOuts() const;
  bool SupportsManifestRestat() const;
  bool SupportsMultilineDepfile() const;
  bool SupportsCWDDepend() const;

  std::string NinjaOutputPath(std::string const& path) const;
  bool HasOutputPathPrefix() const { return !this->OutputPathPrefix.empty(); }
  void StripNinjaOutputPathPrefixAsSuffix(std::string& path);

  bool WriteDyndepFile(
    std::string const& dir_top_src, std::string const& dir_top_bld,
    std::string const& dir_cur_src, std::string const& dir_cur_bld,
    std::string const& arg_dd, std::vector<std::string> const& arg_ddis,
    std::string const& module_dir,
    std::vector<std::string> const& linked_target_dirs,
    std::vector<std::string> const& forward_modules_from_target_dirs,
    std::string const& arg_lang, std::string const& arg_modmapfmt,
    cmCxxModuleExportInfo const& export_info);

  virtual std::string BuildAlias(std::string const& alias,
                                 std::string const& /*config*/) const
  {
    return alias;
  }

  virtual std::string ConfigDirectory(std::string const& /*config*/) const
  {
    return "";
  }

  cmNinjaDeps& GetByproductsForCleanTarget()
  {
    return this->ByproductsForCleanTarget;
  }

  cmNinjaDeps& GetByproductsForCleanTarget(std::string const& config)
  {
    return this->Configs[config].ByproductsForCleanTarget;
  }

  bool EnableCrossConfigBuild() const;

  std::set<std::string> GetCrossConfigs(std::string const& config) const;

  std::set<std::string> const& GetDefaultConfigs() const override
  {
    return this->DefaultConfigs;
  }

  std::set<std::string> const& GetPerConfigUtilityTargets() const
  {
    return this->PerConfigUtilityTargets;
  }

  void AddPerConfigUtilityTarget(std::string const& name)
  {
    this->PerConfigUtilityTargets.insert(name);
  }

  bool IsSingleConfigUtility(cmGeneratorTarget const* target) const;

  bool CheckCxxModuleSupport(CxxModuleSupportQuery query) override;
  bool SupportsBuildDatabase() const override { return true; }

  std::string ConvertToOutputPath(std::string path) const override;

protected:
  std::vector<std::string> const& GetConfigNames() const;

  void Generate() override;

  bool CheckALLOW_DUPLICATE_CUSTOM_TARGETS() const override { return true; }

  virtual bool OpenBuildFileStreams();
  virtual void CloseBuildFileStreams();

  bool OpenFileStream(std::unique_ptr<cmGeneratedFileStream>& stream,
                      std::string const& name);

  static cm::optional<std::set<std::string>> ListSubsetWithAll(
    std::set<std::string> const& all, std::set<std::string> const& defaults,
    std::vector<std::string> const& items);

  std::set<std::string> CrossConfigs;
  std::set<std::string> DefaultConfigs;
  std::string DefaultFileConfig;

private:
  bool FindMakeProgram(cmMakefile* mf) override;
  void CheckNinjaFeatures();
  void CheckNinjaCodePage();
  bool CheckLanguages(std::vector<std::string> const& languages,
                      cmMakefile* mf) const override;
  bool CheckFortran(cmMakefile* mf) const;
  bool CheckISPC(cmMakefile* mf) const;

  void CloseCompileCommandsStream();

  bool OpenRulesFileStream();
  void CloseRulesFileStream();
  void CleanMetaData();

  /// Write the common disclaimer text at the top of each build file.
  void WriteDisclaimer(std::ostream& os) const;

  void WriteAssumedSourceDependencies();

  void WriteTargetAliases(std::ostream& os);
  void WriteFolderTargets(std::ostream& os);

  void WriteBuiltinTargets(std::ostream& os);
  void WriteTargetDefault(std::ostream& os);
  void WriteTargetRebuildManifest(std::ostream& os);
  bool WriteTargetCleanAdditional(std::ostream& os);
  void WriteTargetClean(std::ostream& os);
#if !defined(CMAKE_BOOTSTRAP) && !defined(_WIN32)
  // FIXME(#26668) This does not work on Windows
  void WriteTargetInstrument(std::ostream& os);
#endif
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

  /// The mapping from source file to assumed dependencies.
  std::map<std::string, std::set<std::string>> AssumedSourceDependencies;

  /// Utility targets which have per-config outputs
  std::set<std::string> PerConfigUtilityTargets;

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
  bool NinjaSupportsDyndepsCxx = false;
  bool NinjaSupportsDyndepsFortran = false;
  bool NinjaSupportsRestatTool = false;
  bool NinjaSupportsUnconditionalRecompactTool = false;
  bool NinjaSupportsMultipleOutputs = false;
  bool NinjaSupportsMetadataOnRegeneration = false;
  bool NinjaSupportsCodePage = false;
  bool NinjaSupportsCWDDepend = false;

  codecvt_Encoding NinjaExpectedEncoding = codecvt_Encoding::None;

#ifdef _WIN32
  // Windows Command shell.
  std::string Comspec;
#endif

  bool DiagnosedCxxModuleNinjaSupport = false;

  void InitOutputPathPrefix();

  std::string OutputPathPrefix;
  std::string TargetAll;
  std::string CMakeCacheFile;

  struct ByConfig
  {
    std::set<std::string> AdditionalCleanFiles;

    /// The set of custom commands we have seen.
    std::set<cmCustomCommand const*> CustomCommands;

    struct TargetDependsClosureKey
    {
      cmGeneratorTarget const* Target;
      std::string Config;
      bool GenexOutput;
    };

    std::map<TargetDependsClosureKey, std::unordered_set<std::string>>
      TargetDependsClosures;

    TargetAliasMap TargetAliases;

    cmNinjaDeps ByproductsForCleanTarget;
  };
  std::map<std::string, ByConfig> Configs;

  cmNinjaDeps ByproductsForCleanTarget;

  friend bool operator==(ByConfig::TargetDependsClosureKey const& lhs,
                         ByConfig::TargetDependsClosureKey const& rhs);
  friend bool operator!=(ByConfig::TargetDependsClosureKey const& lhs,
                         ByConfig::TargetDependsClosureKey const& rhs);
  friend bool operator<(ByConfig::TargetDependsClosureKey const& lhs,
                        ByConfig::TargetDependsClosureKey const& rhs);
  friend bool operator>(ByConfig::TargetDependsClosureKey const& lhs,
                        ByConfig::TargetDependsClosureKey const& rhs);
  friend bool operator<=(ByConfig::TargetDependsClosureKey const& lhs,
                         ByConfig::TargetDependsClosureKey const& rhs);
  friend bool operator>=(ByConfig::TargetDependsClosureKey const& lhs,
                         ByConfig::TargetDependsClosureKey const& rhs);
};

class cmGlobalNinjaMultiGenerator : public cmGlobalNinjaGenerator
{
public:
  /// The default name of Ninja's common file. Typically: common.ninja.
  static char const* NINJA_COMMON_FILE;
  /// The default file extension to use for per-config Ninja files.
  static char const* NINJA_FILE_EXTENSION;

  cmGlobalNinjaMultiGenerator(cmake* cm);
  bool IsMultiConfig() const override { return true; }
  static std::unique_ptr<cmGlobalGeneratorFactory> NewFactory()
  {
    return std::unique_ptr<cmGlobalGeneratorFactory>(
      new cmGlobalGeneratorSimpleFactory<cmGlobalNinjaMultiGenerator>());
  }

  static cmDocumentationEntry GetDocumentation();

  std::string GetName() const override
  {
    return cmGlobalNinjaMultiGenerator::GetActualName();
  }

  static std::string GetActualName() { return "Ninja Multi-Config"; }

  std::string BuildAlias(std::string const& alias,
                         std::string const& config) const override
  {
    if (config.empty()) {
      return alias;
    }
    return cmStrCat(alias, ":", config);
  }

  std::string ConfigDirectory(std::string const& config) const override
  {
    if (!config.empty()) {
      return cmStrCat('/', config);
    }
    return "";
  }

  char const* GetCMakeCFGIntDir() const override { return "${CONFIGURATION}"; }

  std::string ExpandCFGIntDir(std::string const& str,
                              std::string const& config) const override;

  cmGeneratedFileStream* GetImplFileStream(
    std::string const& config) const override
  {
    return this->ImplFileStreams.at(config).get();
  }

  cmGeneratedFileStream* GetConfigFileStream(
    std::string const& config) const override
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
                               std::string const& config) const override;

  static std::string GetNinjaImplFilename(std::string const& config);
  static std::string GetNinjaConfigFilename(std::string const& config);

  void AddRebuildManifestOutputs(cmNinjaDeps& outputs) const override;

  void GetQtAutoGenConfigs(std::vector<std::string>& configs) const override;

  bool InspectConfigTypeVariables() override;

  std::string GetDefaultBuildConfig() const override;

  bool SupportsDefaultBuildType() const override { return true; }
  bool SupportsCrossConfigs() const override { return true; }
  bool SupportsDefaultConfigs() const override { return true; }

  std::string OrderDependsTargetForTarget(
    cmGeneratorTarget const* target, std::string const& config) const override;

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
