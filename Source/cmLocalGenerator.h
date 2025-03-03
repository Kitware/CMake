/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <cstddef>
#include <iosfwd>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include <cm/optional>

#include "cmCustomCommandTypes.h"
#include "cmGeneratorOptions.h"
#include "cmGeneratorTarget.h"
#include "cmListFileCache.h"
#include "cmMessageType.h" // IWYU pragma: keep
#include "cmOutputConverter.h"
#include "cmPolicies.h"
#include "cmStateSnapshot.h"
#include "cmValue.h"

class cmCompiledGeneratorExpression;
class cmComputeLinkInformation;
class cmCustomCommand;
class cmCustomCommandGenerator;
class cmCustomCommandLines;
class cmGlobalGenerator;
class cmImplicitDependsList;
class cmLinkLineComputer;
class cmLinkLineDeviceComputer;
class cmMakefile;
class cmRulePlaceholderExpander;
class cmSourceFile;
class cmState;
class cmTarget;
class cmake;

template <typename Iter>
class cmRange;

/** Target and source file which have a specific output.  */
struct cmSourcesWithOutput
{
  /** Target with byproduct.  */
  cmTarget* Target = nullptr;

  /** Source file with output or byproduct.  */
  cmSourceFile* Source = nullptr;
  bool SourceIsByproduct = false;
};

/** \class cmLocalGenerator
 * \brief Create required build files for a directory.
 *
 * Subclasses of this abstract class generate makefiles, DSP, etc for various
 * platforms. This class should never be constructed directly. A
 * GlobalGenerator will create it and invoke the appropriate commands on it.
 */
class cmLocalGenerator : public cmOutputConverter
{
public:
  cmLocalGenerator(cmGlobalGenerator* gg, cmMakefile* makefile);
  ~cmLocalGenerator() override;

  /**
   * Generate the makefile for this directory.
   */
  virtual void Generate() {}

  virtual void ComputeHomeRelativeOutputPath() {}

  /**
   * Calls TraceVSDependencies() on all targets of this generator.
   */
  void TraceDependencies() const;

  virtual void AddHelperCommands() {}

  /**
   * Generate the install rules files in this directory.
   */
  void GenerateInstallRules();

  /**
   * Generate the test files for tests.
   */
  void GenerateTestFiles();

  /**
   * Generate a manifest of target files that will be built.
   */
  void ComputeTargetManifest();

  bool ComputeTargetCompileFeatures();

  bool IsRootMakefile() const;

  //! Get the makefile for this generator
  cmMakefile* GetMakefile() { return this->Makefile; }

  //! Get the makefile for this generator, const version
  cmMakefile const* GetMakefile() const { return this->Makefile; }

  //! Get the GlobalGenerator this is associated with
  cmGlobalGenerator* GetGlobalGenerator() { return this->GlobalGenerator; }
  cmGlobalGenerator const* GetGlobalGenerator() const
  {
    return this->GlobalGenerator;
  }

  virtual std::unique_ptr<cmRulePlaceholderExpander>
  CreateRulePlaceholderExpander(
    cmBuildStep buildStep = cmBuildStep::Compile) const;
  virtual std::unique_ptr<cmRulePlaceholderExpander>
  CreateRulePlaceholderExpander(cmBuildStep buildStep,
                                cmGeneratorTarget const* target,
                                std::string const& language);

  std::string GetExeExportFlags(std::string const& linkLanguage,
                                cmGeneratorTarget& tgt) const;

  cmState* GetState() const;
  cmStateSnapshot GetStateSnapshot() const;

  void AddArchitectureFlags(std::string& flags,
                            cmGeneratorTarget const* target,
                            std::string const& lang, std::string const& config,
                            std::string const& filterArch = std::string());

  void AddLanguageFlags(std::string& flags, cmGeneratorTarget const* target,
                        cmBuildStep compileOrLink, std::string const& lang,
                        std::string const& config);
  void AddLanguageFlagsForLinking(std::string& flags,
                                  cmGeneratorTarget const* target,
                                  std::string const& lang,
                                  std::string const& config);
  void AddFeatureFlags(std::string& flags, cmGeneratorTarget const* target,
                       std::string const& lang, std::string const& config);
  void AddVisibilityPresetFlags(std::string& flags,
                                cmGeneratorTarget const* target,
                                std::string const& lang);
  void AddConfigVariableFlags(std::string& flags, std::string const& var,
                              std::string const& config);
  // Handle prefixes processing (like LINKER:)
  void AddConfigVariableFlags(std::string& flags, std::string const& var,
                              cmGeneratorTarget const* target,
                              cmBuildStep compileOrLink,
                              std::string const& lang,
                              std::string const& config);
  void AddColorDiagnosticsFlags(std::string& flags, std::string const& lang);
  //! Append flags to a string.
  virtual void AppendFlags(std::string& flags,
                           std::string const& newFlags) const;
  virtual void AppendFlags(std::string& flags,
                           std::vector<BT<std::string>> const& newFlags) const;
  virtual void AppendFlagEscape(std::string& flags,
                                std::string const& rawFlag) const;
  /**
   * Append flags after parsing, prefixes processing (like LINKER:) and
   * escaping
   */
  void AppendFlags(std::string& flags, std::string const& newFlags,
                   std::string const& name, cmGeneratorTarget const* target,
                   cmBuildStep compileOrLink, std::string const& lang);
  void AddISPCDependencies(cmGeneratorTarget* target);
  void AddPchDependencies(cmGeneratorTarget* target);
  void AddUnityBuild(cmGeneratorTarget* target);
  virtual void AddXCConfigSources(cmGeneratorTarget* /* target */) {}
  void AppendLinkerTypeFlags(std::string& flags, cmGeneratorTarget* target,
                             std::string const& config,
                             std::string const& linkLanguage);
  void AppendIPOLinkerFlags(std::string& flags, cmGeneratorTarget* target,
                            std::string const& config,
                            std::string const& lang);
  void AppendPositionIndependentLinkerFlags(std::string& flags,
                                            cmGeneratorTarget* target,
                                            std::string const& config,
                                            std::string const& lang);
  void AppendWarningAsErrorLinkerFlags(std::string& flags,
                                       cmGeneratorTarget* target,
                                       std::string const& lang);
  void AppendDependencyInfoLinkerFlags(std::string& flags,
                                       cmGeneratorTarget* target,
                                       std::string const& config,
                                       std::string const& lang);
  virtual std::string GetLinkDependencyFile(cmGeneratorTarget* target,
                                            std::string const& config) const;
  void AppendModuleDefinitionFlag(std::string& flags,
                                  cmGeneratorTarget const* target,
                                  cmLinkLineComputer* linkLineComputer,
                                  std::string const& config);
  bool AppendLWYUFlags(std::string& flags, cmGeneratorTarget const* target,
                       std::string const& lang);

  //! Get the include flags for the current makefile and language
  std::string GetIncludeFlags(std::vector<std::string> const& includes,
                              cmGeneratorTarget* target,
                              std::string const& lang,
                              std::string const& config,
                              bool forResponseFile = false);

  using GeneratorTargetVector =
    std::vector<std::unique_ptr<cmGeneratorTarget>>;
  GeneratorTargetVector const& GetGeneratorTargets() const
  {
    return this->GeneratorTargets;
  }

  GeneratorTargetVector const& GetOwnedImportedGeneratorTargets() const
  {
    return this->OwnedImportedGeneratorTargets;
  }

  void AddGeneratorTarget(std::unique_ptr<cmGeneratorTarget> gt);
  void AddImportedGeneratorTarget(cmGeneratorTarget* gt);
  void AddOwnedImportedGeneratorTarget(std::unique_ptr<cmGeneratorTarget> gt);

  cmGeneratorTarget* FindLocalNonAliasGeneratorTarget(
    std::string const& name) const;
  cmGeneratorTarget* FindGeneratorTargetToUse(std::string const& name) const;

  /**
   * Process a list of include directories
   */
  void AppendIncludeDirectories(std::vector<std::string>& includes,
                                std::string const& includes_list,
                                cmSourceFile const& sourceFile) const;
  void AppendIncludeDirectories(std::vector<std::string>& includes,
                                std::vector<std::string> const& includes_vec,
                                cmSourceFile const& sourceFile) const;

  /**
   * Encode a list of preprocessor definitions for the compiler
   * command line.
   */
  void AppendDefines(std::set<std::string>& defines,
                     std::string const& defines_list) const;
  void AppendDefines(std::set<BT<std::string>>& defines,
                     std::string const& defines_list) const;
  void AppendDefines(std::set<BT<std::string>>& defines,
                     std::vector<BT<std::string>> const& defines_vec) const;

  /**
   * Encode a list of compile options for the compiler
   * command line.
   */
  void AppendCompileOptions(std::string& options,
                            std::string const& options_list,
                            char const* regex = nullptr) const;
  void AppendCompileOptions(std::string& options,
                            std::vector<std::string> const& options_vec,
                            char const* regex = nullptr) const;
  void AppendCompileOptions(std::vector<BT<std::string>>& options,
                            std::vector<BT<std::string>> const& options_vec,
                            char const* regex = nullptr) const;

  /**
   * Join a set of defines into a definesString with a space separator.
   */
  void JoinDefines(std::set<std::string> const& defines,
                   std::string& definesString, std::string const& lang);

  /** Lookup and append options associated with a particular feature.  */
  void AppendFeatureOptions(std::string& flags, std::string const& lang,
                            char const* feature);

  cmValue GetFeature(std::string const& feature, std::string const& config);

  /** \brief Get absolute path to dependency \a name
   *
   * Translate a dependency as given in CMake code to the name to
   * appear in a generated build file.
   * - If \a name is a utility target, returns false.
   * - If \a name is a CMake target, it will be transformed to the real output
   *   location of that target for the given configuration.
   * - If \a name is the full path to a file, it will be returned.
   * - Otherwise \a name is treated as a relative path with respect to
   *   the source directory of this generator.  This should only be
   *   used for dependencies of custom commands.
   */
  bool GetRealDependency(std::string const& name, std::string const& config,
                         std::string& dep);

  /** Called from command-line hook to clear dependencies.  */
  virtual void ClearDependencies(cmMakefile* /* mf */, bool /* verbose */) {}

  /** Called from command-line hook to update dependencies.  */
  virtual bool UpdateDependencies(std::string const& /* tgtInfo */,
                                  bool /*verbose*/, bool /*color*/)
  {
    return true;
  }

  /** @brief Get the include directories for the current makefile and language
   * and optional the compiler implicit include directories.
   *
   * @arg stripImplicitDirs Strip all directories found in
   *      CMAKE_<LANG>_IMPLICIT_INCLUDE_DIRECTORIES from the result.
   * @arg appendAllImplicitDirs Append all directories found in
   *      CMAKE_<LANG>_IMPLICIT_INCLUDE_DIRECTORIES to the result.
   */
  std::vector<BT<std::string>> GetIncludeDirectoriesImplicit(
    cmGeneratorTarget const* target, std::string const& lang = "C",
    std::string const& config = "", bool stripImplicitDirs = true,
    bool appendAllImplicitDirs = false) const;

  /** @brief Get the include directories for the current makefile and language
   * and optional the compiler implicit include directories.
   *
   * @arg dirs Directories are appended to this list
   */
  void GetIncludeDirectoriesImplicit(std::vector<std::string>& dirs,
                                     cmGeneratorTarget const* target,
                                     std::string const& lang = "C",
                                     std::string const& config = "",
                                     bool stripImplicitDirs = true,
                                     bool appendAllImplicitDirs = false) const;

  /** @brief Get the include directories for the current makefile and language.
   * @arg dirs Include directories are appended to this list
   */
  void GetIncludeDirectories(std::vector<std::string>& dirs,
                             cmGeneratorTarget const* target,
                             std::string const& lang = "C",
                             std::string const& config = "") const;

  /** @brief Get the include directories for the current makefile and language.
   * @return The include directory list
   */
  std::vector<BT<std::string>> GetIncludeDirectories(
    cmGeneratorTarget const* target, std::string const& lang = "C",
    std::string const& config = "") const;

  void AddCompileOptions(std::string& flags, cmGeneratorTarget* target,
                         std::string const& lang, std::string const& config);
  void AddCompileOptions(std::vector<BT<std::string>>& flags,
                         cmGeneratorTarget* target, std::string const& lang,
                         std::string const& config);

  /**
   * Add a custom PRE_BUILD, PRE_LINK, or POST_BUILD command to a target.
   */
  cmTarget* AddCustomCommandToTarget(
    std::string const& target, cmCustomCommandType type,
    std::unique_ptr<cmCustomCommand> cc,
    cmObjectLibraryCommands objLibCommands = cmObjectLibraryCommands::Reject);

  /**
   * Add a custom command to a source file.
   */
  cmSourceFile* AddCustomCommandToOutput(std::unique_ptr<cmCustomCommand> cc,
                                         bool replace = false);

  /**
   * Add a utility to the build.  A utility target is a command that is run
   * every time the target is built.
   */
  cmTarget* AddUtilityCommand(std::string const& utilityName,
                              bool excludeFromAll,
                              std::unique_ptr<cmCustomCommand> cc);

  virtual std::string CreateUtilityOutput(
    std::string const& targetName, std::vector<std::string> const& byproducts,
    cmListFileBacktrace const& bt);

  virtual std::vector<cmCustomCommandGenerator> MakeCustomCommandGenerators(
    cmCustomCommand const& cc, std::string const& config);

  std::vector<std::string> ExpandCustomCommandOutputPaths(
    cmCompiledGeneratorExpression const& cge, std::string const& config);
  std::vector<std::string> ExpandCustomCommandOutputGenex(
    std::string const& o, cmListFileBacktrace const& bt);

  /**
   * Add target byproducts.
   */
  void AddTargetByproducts(cmTarget* target,
                           std::vector<std::string> const& byproducts,
                           cmListFileBacktrace const& bt,
                           cmCommandOrigin origin);

  enum class OutputRole
  {
    Primary,
    Byproduct,
  };

  /**
   * Add source file outputs.
   */
  void AddSourceOutputs(cmSourceFile* source,
                        std::vector<std::string> const& outputs,
                        OutputRole role, cmListFileBacktrace const& bt,
                        cmCommandOrigin origin);

  /**
   * Return the target if the provided source name is a byproduct of a utility
   * target or a PRE_BUILD, PRE_LINK, or POST_BUILD command.
   * Return the source file which has the provided source name as output.
   */
  cmSourcesWithOutput GetSourcesWithOutput(std::string const& name) const;

  /**
   * Is there a source file that has the provided source name as an output?
   * If so then return it.
   */
  cmSourceFile* GetSourceFileWithOutput(
    std::string const& name,
    cmSourceOutputKind kind = cmSourceOutputKind::OutputOnly) const;

  std::string GetProjectName() const;

  /** Compute the language used to compile the given source file.  */
  std::string GetSourceFileLanguage(cmSourceFile const& source);

  // Fill the vector with the target names for the object files,
  // preprocessed files and assembly files.
  void GetIndividualFileTargets(std::vector<std::string>&) {}

  /**
   * Get the relative path from the generator output directory to a
   * per-target support directory.
   */
  virtual std::string GetTargetDirectory(
    cmGeneratorTarget const* target) const;

  cmPolicies::PolicyStatus GetPolicyStatus(cmPolicies::PolicyID id) const;

  cmake* GetCMakeInstance() const;

  std::string const& GetSourceDirectory() const;
  std::string const& GetBinaryDirectory() const;

  std::string const& GetCurrentBinaryDirectory() const;
  std::string const& GetCurrentSourceDirectory() const;

  /**
   * Generate a macOS application bundle Info.plist file.
   */
  void GenerateAppleInfoPList(cmGeneratorTarget* target,
                              std::string const& targetName,
                              std::string const& fname);

  /**
   * Generate a macOS framework Info.plist file.
   */
  void GenerateFrameworkInfoPList(cmGeneratorTarget* target,
                                  std::string const& targetName,
                                  std::string const& fname);
  /** Construct a comment for a custom command.  */
  std::string ConstructComment(cmCustomCommandGenerator const& ccg,
                               char const* default_comment = "") const;
  // Compute object file names.
  std::string GetObjectFileNameWithoutTarget(
    cmSourceFile const& source, std::string const& dir_max,
    bool* hasSourceExtension = nullptr,
    char const* customOutputExtension = nullptr);

  /** Fill out the static linker flags for the given target.  */
  void GetStaticLibraryFlags(std::string& flags, std::string const& config,
                             std::string const& linkLanguage,
                             cmGeneratorTarget* target);
  std::vector<BT<std::string>> GetStaticLibraryFlags(
    std::string const& config, std::string const& linkLanguage,
    cmGeneratorTarget* target);

  /** Fill out these strings for the given target.  Libraries to link,
   *  flags, and linkflags. */
  void GetDeviceLinkFlags(cmLinkLineDeviceComputer& linkLineComputer,
                          std::string const& config, std::string& linkLibs,
                          std::string& linkFlags, std::string& frameworkPath,
                          std::string& linkPath, cmGeneratorTarget* target);

  void GetTargetFlags(cmLinkLineComputer* linkLineComputer,
                      std::string const& config, std::string& linkLibs,
                      std::string& flags, std::string& linkFlags,
                      std::string& frameworkPath, std::string& linkPath,
                      cmGeneratorTarget* target);
  void GetTargetFlags(
    cmLinkLineComputer* linkLineComputer, std::string const& config,
    std::vector<BT<std::string>>& linkLibs, std::string& flags,
    std::vector<BT<std::string>>& linkFlags, std::string& frameworkPath,
    std::vector<BT<std::string>>& linkPath, cmGeneratorTarget* target);
  void GetTargetDefines(cmGeneratorTarget const* target,
                        std::string const& config, std::string const& lang,
                        std::set<std::string>& defines) const;
  std::set<BT<std::string>> GetTargetDefines(cmGeneratorTarget const* target,
                                             std::string const& config,
                                             std::string const& lang) const;
  void GetTargetCompileFlags(cmGeneratorTarget* target,
                             std::string const& config,
                             std::string const& lang, std::string& flags,
                             std::string const& arch);
  std::vector<BT<std::string>> GetTargetCompileFlags(
    cmGeneratorTarget* target, std::string const& config,
    std::string const& lang, std::string const& arch = std::string());

  std::string GetFrameworkFlags(std::string const& l,
                                std::string const& config,
                                cmGeneratorTarget* target);
  std::string GetXcFrameworkFlags(std::string const& l,
                                  std::string const& config,
                                  cmGeneratorTarget* target);
  virtual std::string GetTargetFortranFlags(cmGeneratorTarget const* target,
                                            std::string const& config);

  virtual void ComputeObjectFilenames(
    std::map<cmSourceFile const*, std::string>& mapping,
    cmGeneratorTarget const* gt = nullptr);

  bool IsWindowsShell() const;
  bool IsWatcomWMake() const;
  bool IsMinGWMake() const;
  bool IsNMake() const;
  bool IsNinjaMulti() const;

  void IssueMessage(MessageType t, std::string const& text) const;

  void CreateEvaluationFileOutputs();
  void CreateEvaluationFileOutputs(std::string const& config);
  void ProcessEvaluationFiles(std::vector<std::string>& generatedFiles);

  std::string GetRuleLauncher(cmGeneratorTarget* target,
                              std::string const& prop,
                              std::string const& config);

  // Return Swift_COMPILATION_MODE value if CMP0157 is NEW.
  cm::optional<cmSwiftCompileMode> GetSwiftCompileMode(
    cmGeneratorTarget const* target, std::string const& config);

  // Can we build Swift with a separate object build and link step
  // (If CMP0157 is NEW, we can do a split build)
  bool IsSplitSwiftBuild() const;

protected:
  // The default implementation converts to a Windows shortpath to
  // help older toolchains handle spaces and such.  A generator may
  // override this to avoid that conversion.
  virtual std::string ConvertToIncludeReference(
    std::string const& path, cmOutputConverter::OutputFormat format);

  //! put all the libraries for a target on into the given stream
  void OutputLinkLibraries(cmComputeLinkInformation* pcli,
                           cmLinkLineComputer* linkLineComputer,
                           std::string& linkLibraries,
                           std::string& frameworkPath, std::string& linkPath);
  void OutputLinkLibraries(cmComputeLinkInformation* pcli,
                           cmLinkLineComputer* linkLineComputer,
                           std::vector<BT<std::string>>& linkLibraries,
                           std::string& frameworkPath,
                           std::vector<BT<std::string>>& linkPath);

  // Handle old-style install rules stored in the targets.
  void GenerateTargetInstallRules(
    std::ostream& os, std::string const& config,
    std::vector<std::string> const& configurationTypes);

  virtual void AddGeneratorSpecificInstallSetup(std::ostream&) {}

  std::string& CreateSafeUniqueObjectFileName(std::string const& sin,
                                              std::string const& dir_max);

  /** Check whether the native build system supports the given
      definition.  Issues a warning.  */
  virtual bool CheckDefinition(std::string const& define) const;

  cmMakefile* Makefile;
  cmListFileBacktrace DirectoryBacktrace;
  cmGlobalGenerator* GlobalGenerator;
  std::map<std::string, std::string> UniqueObjectNamesMap;
  std::string::size_type ObjectPathMax;
  std::set<std::string> ObjectMaxPathViolations;

  std::vector<std::string> EnvCPATH;

  using GeneratorTargetMap =
    std::unordered_map<std::string, cmGeneratorTarget*>;
  GeneratorTargetMap GeneratorTargetSearchIndex;
  GeneratorTargetVector GeneratorTargets;

  GeneratorTargetMap ImportedGeneratorTargets;
  GeneratorTargetVector OwnedImportedGeneratorTargets;
  std::map<std::string, std::string> AliasTargets;

  std::map<std::string, std::string> Compilers;
  std::map<std::string, std::string> VariableMappings;
  std::string CompilerSysroot;
  std::string LinkerSysroot;
  std::unordered_map<std::string, std::string> AppleArchSysroots;

  bool EmitUniversalBinaryFlags;

private:
  /**
   * See LinearGetSourceFileWithOutput for background information
   */
  cmTarget* LinearGetTargetWithOutput(std::string const& name) const;

  /**
   * Generalized old version of GetSourceFileWithOutput kept for
   * backward-compatibility. It implements a linear search and supports
   * relative file paths. It is used as a fall back by GetSourceFileWithOutput
   * and GetSourcesWithOutput.
   */
  cmSourceFile* LinearGetSourceFileWithOutput(std::string const& name,
                                              cmSourceOutputKind kind,
                                              bool& byproduct) const;
  struct SourceEntry
  {
    cmSourcesWithOutput Sources;
  };

  // A map for fast output to input look up.
  using OutputToSourceMap = std::unordered_map<std::string, SourceEntry>;
  OutputToSourceMap OutputToSource;

  void UpdateOutputToSourceMap(std::string const& byproduct, cmTarget* target,
                               cmListFileBacktrace const& bt,
                               cmCommandOrigin origin);
  void UpdateOutputToSourceMap(std::string const& output, cmSourceFile* source,
                               OutputRole role, cmListFileBacktrace const& bt,
                               cmCommandOrigin origin);

  void AddPositionIndependentFlags(std::string& flags, std::string const& l,
                                   int targetType);

  void ComputeObjectMaxPath();
  bool AllAppleArchSysrootsAreTheSame(std::vector<std::string> const& archs,
                                      cmValue sysroot);

  void CopyPchCompilePdb(std::string const& config,
                         std::string const& language,
                         cmGeneratorTarget* target,
                         std::string const& ReuseFrom,
                         cmGeneratorTarget* reuseTarget,
                         std::vector<std::string> const& extensions);

  // Returns MSVC_DEBUG_INFORMATION_FORMAT value if CMP0141 is NEW.
  cm::optional<std::string> GetMSVCDebugFormatName(
    std::string const& config, cmGeneratorTarget const* target);

  struct UnityBatchedSource
  {
    cmSourceFile* Source = nullptr;
    std::vector<size_t> Configs;
    UnityBatchedSource(cmSourceFile* sf)
      : Source(sf)
    {
    }
  };
  struct UnitySource
  {
    std::string Path;
    bool PerConfig = false;
    UnitySource(std::string path, bool perConfig)
      : Path(std::move(path))
      , PerConfig(perConfig)
    {
    }
  };
  /** Whether to insert relative or absolute paths into unity files */
  enum class UnityPathMode
  {
    Absolute,
    Relative
  };

  UnitySource WriteUnitySource(
    cmGeneratorTarget* target, std::vector<std::string> const& configs,
    cmRange<std::vector<UnityBatchedSource>::const_iterator> sources,
    cmValue beforeInclude, cmValue afterInclude, std::string filename,
    std::string const& unityFileDirectory, UnityPathMode pathMode) const;
  void WriteUnitySourceInclude(std::ostream& unity_file,
                               cm::optional<std::string> const& cond,
                               std::string const& sf_full_path,
                               cmValue beforeInclude, cmValue afterInclude,
                               cmValue uniqueIdName, UnityPathMode pathMode,
                               std::string const& unityFileDirectory) const;
  std::vector<UnitySource> AddUnityFilesModeAuto(
    cmGeneratorTarget* target, std::string const& lang,
    std::vector<std::string> const& configs,
    std::vector<UnityBatchedSource> const& filtered_sources,
    cmValue beforeInclude, cmValue afterInclude,
    std::string const& filename_base, UnityPathMode pathMode,
    size_t batchSize);
  std::vector<UnitySource> AddUnityFilesModeGroup(
    cmGeneratorTarget* target, std::string const& lang,
    std::vector<std::string> const& configs,
    std::vector<UnityBatchedSource> const& filtered_sources,
    cmValue beforeInclude, cmValue afterInclude,
    std::string const& filename_base, UnityPathMode pathMode);
};

namespace detail {
void AddCustomCommandToTarget(cmLocalGenerator& lg, cmCommandOrigin origin,
                              cmTarget* target, cmCustomCommandType type,
                              std::unique_ptr<cmCustomCommand> cc);

cmSourceFile* AddCustomCommandToOutput(cmLocalGenerator& lg,
                                       cmCommandOrigin origin,
                                       std::unique_ptr<cmCustomCommand> cc,
                                       bool replace);

void AppendCustomCommandToOutput(cmLocalGenerator& lg,
                                 cmListFileBacktrace const& lfbt,
                                 std::string const& output,
                                 std::vector<std::string> const& depends,
                                 cmImplicitDependsList const& implicit_depends,
                                 cmCustomCommandLines const& commandLines);

void AddUtilityCommand(cmLocalGenerator& lg, cmCommandOrigin origin,
                       cmTarget* target, std::unique_ptr<cmCustomCommand> cc);

std::vector<std::string> ComputeISPCObjectSuffixes(cmGeneratorTarget* target);
std::vector<std::string> ComputeISPCExtraObjects(
  std::string const& objectName, std::string const& buildDirectory,
  std::vector<std::string> const& ispcSuffixes);
}
