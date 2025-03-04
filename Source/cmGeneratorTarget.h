/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <cstddef>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include <cm/optional>
#include <cm/string_view>

#include "cmAlgorithms.h"
#include "cmLinkItem.h"
#include "cmListFileCache.h"
#include "cmPolicies.h"
#include "cmStandardLevel.h"
#include "cmStateTypes.h"
#include "cmValue.h"

class cmake;
enum class cmBuildStep;
class cmCompiledGeneratorExpression;
class cmComputeLinkInformation;
class cmCustomCommand;
class cmFileSet;
class cmGlobalGenerator;
class cmLocalGenerator;
class cmMakefile;
class cmSourceFile;
struct cmSyntheticTargetCache;
class cmTarget;

struct cmGeneratorExpressionContext;
struct cmGeneratorExpressionDAGChecker;

class cmGeneratorTarget
{
public:
  cmGeneratorTarget(cmTarget*, cmLocalGenerator* lg);
  ~cmGeneratorTarget();

  cmGeneratorTarget(cmGeneratorTarget const&) = delete;
  cmGeneratorTarget& operator=(cmGeneratorTarget const&) = delete;

  cmLocalGenerator* GetLocalGenerator() const;

  cmGlobalGenerator* GetGlobalGenerator() const;

  bool IsInBuildSystem() const;
  bool IsNormal() const;
  bool IsRuntimeBinary() const;
  bool IsSynthetic() const;
  bool IsImported() const;
  bool IsImportedGloballyVisible() const;
  bool IsForeign() const;
  bool CanCompileSources() const;
  bool HasKnownRuntimeArtifactLocation(std::string const& config) const;
  std::string const& GetLocation(std::string const& config) const;

  /** Get the full path to the target's main artifact, if known.  */
  cm::optional<std::string> MaybeGetLocation(std::string const& config) const;

  std::vector<cmCustomCommand> const& GetPreBuildCommands() const;
  std::vector<cmCustomCommand> const& GetPreLinkCommands() const;
  std::vector<cmCustomCommand> const& GetPostBuildCommands() const;

  void AppendCustomCommandSideEffects(
    std::set<cmGeneratorTarget const*>& sideEffects) const;
  void AppendLanguageSideEffects(
    std::map<std::string, std::set<cmGeneratorTarget const*>>& sideEffects)
    const;

#define DECLARE_TARGET_POLICY(POLICY)                                         \
  cmPolicies::PolicyStatus GetPolicyStatus##POLICY() const                    \
  {                                                                           \
    return this->PolicyMap.Get(cmPolicies::POLICY);                           \
  }

  CM_FOR_EACH_TARGET_POLICY(DECLARE_TARGET_POLICY)

#undef DECLARE_TARGET_POLICY

  /** Get the location of the target in the build tree with a placeholder
      referencing the configuration in the native build system.  This
      location is suitable for use as the LOCATION target property.  */
  std::string const& GetLocationForBuild() const;

  cmComputeLinkInformation* GetLinkInformation(
    std::string const& config) const;

  // Perform validation checks on memoized link structures.
  // Call this after generation is complete.
  void CheckLinkLibraries() const;

  class CheckLinkLibrariesSuppressionRAII
  {
  public:
    CheckLinkLibrariesSuppressionRAII();
    ~CheckLinkLibrariesSuppressionRAII();
  };

  cmStateEnums::TargetType GetType() const;
  std::string const& GetName() const;
  std::string GetFamilyName() const;
  std::string GetExportName() const;
  std::string GetFilesystemExportName() const;

  std::vector<std::string> GetPropertyKeys() const;
  //! Might return a nullptr if the property is not set or invalid
  cmValue GetProperty(std::string const& prop) const;
  //! Always returns a valid pointer
  std::string const& GetSafeProperty(std::string const& prop) const;
  bool GetPropertyAsBool(std::string const& prop) const;
  void GetSourceFiles(std::vector<cmSourceFile*>& files,
                      std::string const& config) const;
  std::vector<BT<cmSourceFile*>> GetSourceFiles(
    std::string const& config) const;

  /** Source file kinds (classifications).
      Generators use this to decide how to treat a source file.  */
  enum SourceKind
  {
    SourceKindAppManifest,
    SourceKindCertificate,
    SourceKindCustomCommand,
    SourceKindExternalObject,
    SourceKindCxxModuleSource,
    SourceKindExtra,
    SourceKindHeader,
    SourceKindIDL,
    SourceKindManifest,
    SourceKindModuleDefinition,
    SourceKindObjectSource,
    SourceKindResx,
    SourceKindXaml,
    SourceKindUnityBatched
  };

  /** A source file paired with a kind (classification).  */
  struct SourceAndKind
  {
    BT<cmSourceFile*> Source;
    SourceKind Kind;
  };

  /** All sources needed for a configuration with kinds assigned.  */
  struct KindedSources
  {
    std::vector<SourceAndKind> Sources;
    bool Initialized = false;
  };

  /** Get all sources needed for a configuration with kinds assigned.  */
  KindedSources const& GetKindedSources(std::string const& config) const;

  struct AllConfigSource
  {
    cmSourceFile* Source;
    cmGeneratorTarget::SourceKind Kind;
    std::vector<size_t> Configs;
  };

  /** Get all sources needed for all configurations with kinds and
      per-source configurations assigned.  */
  std::vector<AllConfigSource> const& GetAllConfigSources() const;

  /** Get all sources needed for all configurations with given kind.  */
  std::vector<AllConfigSource> GetAllConfigSources(SourceKind kind) const;

  /** Get all languages used to compile sources in any configuration.
      This excludes the languages of objects from object libraries.  */
  std::set<std::string> GetAllConfigCompileLanguages() const;

  void GetObjectSources(std::vector<cmSourceFile const*>&,
                        std::string const& config) const;
  std::string const& GetObjectName(cmSourceFile const* file);
  char const* GetCustomObjectExtension() const;

  bool HasExplicitObjectName(cmSourceFile const* file) const;
  void AddExplicitObjectName(cmSourceFile const* sf);

  BTs<std::string> const* GetLanguageStandardProperty(
    std::string const& lang, std::string const& config) const;

  cmValue GetLanguageStandard(std::string const& lang,
                              std::string const& config) const;

  cmValue GetLanguageExtensions(std::string const& lang) const;

  bool GetLanguageStandardRequired(std::string const& lang) const;

  void GetModuleDefinitionSources(std::vector<cmSourceFile const*>&,
                                  std::string const& config) const;
  void GetExternalObjects(std::vector<cmSourceFile const*>&,
                          std::string const& config) const;
  void GetHeaderSources(std::vector<cmSourceFile const*>&,
                        std::string const& config) const;
  void GetCxxModuleSources(std::vector<cmSourceFile const*>&,
                           std::string const& config) const;
  void GetExtraSources(std::vector<cmSourceFile const*>&,
                       std::string const& config) const;
  void GetCustomCommands(std::vector<cmSourceFile const*>&,
                         std::string const& config) const;
  void GetManifests(std::vector<cmSourceFile const*>&,
                    std::string const& config) const;

  std::set<cmLinkItem> const& GetUtilityItems() const;

  void ComputeObjectMapping();

  cmValue GetFeature(std::string const& feature,
                     std::string const& config) const;

  std::string GetLinkerTypeProperty(std::string const& lang,
                                    std::string const& config) const;

  char const* GetLinkPIEProperty(std::string const& config) const;

  bool IsIPOEnabled(std::string const& lang, std::string const& config) const;

  bool IsLinkInterfaceDependentBoolProperty(std::string const& p,
                                            std::string const& config) const;
  bool IsLinkInterfaceDependentStringProperty(std::string const& p,
                                              std::string const& config) const;
  bool IsLinkInterfaceDependentNumberMinProperty(
    std::string const& p, std::string const& config) const;
  bool IsLinkInterfaceDependentNumberMaxProperty(
    std::string const& p, std::string const& config) const;

  bool GetLinkInterfaceDependentBoolProperty(std::string const& p,
                                             std::string const& config) const;

  char const* GetLinkInterfaceDependentStringProperty(
    std::string const& p, std::string const& config) const;
  char const* GetLinkInterfaceDependentNumberMinProperty(
    std::string const& p, std::string const& config) const;
  char const* GetLinkInterfaceDependentNumberMaxProperty(
    std::string const& p, std::string const& config) const;

  class DeviceLinkSetter
  {
  public:
    DeviceLinkSetter(cmGeneratorTarget& target)
      : Target(target)
    {
      this->PreviousState = target.SetDeviceLink(true);
    }
    ~DeviceLinkSetter() { this->Target.SetDeviceLink(this->PreviousState); }

  private:
    cmGeneratorTarget& Target;
    bool PreviousState;
  };

  bool SetDeviceLink(bool deviceLink);
  bool IsDeviceLink() const { return this->DeviceLink; }

  cmLinkInterface const* GetLinkInterface(
    std::string const& config, cmGeneratorTarget const* headTarget) const;

  enum class UseTo
  {
    Compile, // Usage requirements for compiling.  Excludes $<LINK_ONLY>.
    Link,    // Usage requirements for linking.  Includes $<LINK_ONLY>.
  };

  cmLinkInterfaceLibraries const* GetLinkInterfaceLibraries(
    std::string const& config, cmGeneratorTarget const* headTarget,
    UseTo usage) const;

  void ComputeLinkInterfaceLibraries(std::string const& config,
                                     cmOptionalLinkInterface& iface,
                                     cmGeneratorTarget const* head,
                                     UseTo usage) const;

  /** Get the library name for an imported interface library.  */
  std::string GetImportedLibName(std::string const& config) const;

  /** Get the full path to the target according to the settings in its
      makefile and the configuration type.  */
  std::string GetFullPath(
    std::string const& config,
    cmStateEnums::ArtifactType artifact = cmStateEnums::RuntimeBinaryArtifact,
    bool realname = false) const;
  std::string NormalGetFullPath(std::string const& config,
                                cmStateEnums::ArtifactType artifact,
                                bool realname) const;
  std::string NormalGetRealName(std::string const& config,
                                cmStateEnums::ArtifactType artifact =
                                  cmStateEnums::RuntimeBinaryArtifact) const;

  /** Get the names of an object library's object files underneath
      its object file directory.  */
  void GetTargetObjectNames(std::string const& config,
                            std::vector<std::string>& objects) const;

  /** What hierarchy level should the reported directory contain */
  enum BundleDirectoryLevel
  {
    BundleDirLevel,
    ContentLevel,
    FullLevel
  };

  /** @return the Mac App directory without the base */
  std::string GetAppBundleDirectory(std::string const& config,
                                    BundleDirectoryLevel level) const;

  /** Return whether this target is marked as deprecated by the
      maintainer  */
  bool IsDeprecated() const;

  /** Returns the deprecation message provided by the maintainer */
  std::string GetDeprecation() const;

  /** Return whether this target is an executable Bundle, a framework
      or CFBundle on Apple.  */
  bool IsBundleOnApple() const;

  /** Return whether this target is a Win32 executable */
  bool IsWin32Executable(std::string const& config) const;

  /** Get the full name of the target according to the settings in its
      makefile.  */
  std::string GetFullName(std::string const& config,
                          cmStateEnums::ArtifactType artifact =
                            cmStateEnums::RuntimeBinaryArtifact) const;

  /** @return the Mac framework directory without the base. */
  std::string GetFrameworkDirectory(std::string const& config,
                                    BundleDirectoryLevel level) const;

  /** Return the framework version string.  Undefined if
      IsFrameworkOnApple returns false.  */
  std::string GetFrameworkVersion() const;

  /** @return the Mac CFBundle directory without the base */
  std::string GetCFBundleDirectory(std::string const& config,
                                   BundleDirectoryLevel level) const;

  /** Return the install name directory for the target in the
   * build tree.  For example: "\@rpath/", "\@loader_path/",
   * or "/full/path/to/library".  */
  std::string GetInstallNameDirForBuildTree(std::string const& config) const;

  /** Return the install name directory for the target in the
   * install tree.  For example: "\@rpath/" or "\@loader_path/". */
  std::string GetInstallNameDirForInstallTree(
    std::string const& config, std::string const& installPrefix) const;

  cmListFileBacktrace GetBacktrace() const;

  std::set<BT<std::pair<std::string, bool>>> const& GetUtilities() const;

  bool LinkLanguagePropagatesToDependents() const
  {
    return this->GetType() == cmStateEnums::STATIC_LIBRARY;
  }

  /** Get the macro to define when building sources in this target.
      If no macro should be defined null is returned.  */
  std::string const* GetExportMacro() const;

  /** Get the soname of the target.  Allowed only for a shared library.  */
  std::string GetSOName(std::string const& config,
                        cmStateEnums::ArtifactType artifact =
                          cmStateEnums::RuntimeBinaryArtifact) const;

  struct NameComponents
  {
    std::string prefix;
    std::string base;
    std::string suffix;
  };
  NameComponents const& GetFullNameComponents(
    std::string const& config,
    cmStateEnums::ArtifactType artifact =
      cmStateEnums::RuntimeBinaryArtifact) const;

  /** Append to @a base the bundle directory hierarchy up to a certain @a level
   * and return it. */
  std::string BuildBundleDirectory(std::string const& base,
                                   std::string const& config,
                                   BundleDirectoryLevel level) const;

  /** @return the mac content directory for this target. */
  std::string GetMacContentDirectory(
    std::string const& config, cmStateEnums::ArtifactType artifact) const;

  /** @return folder prefix for IDEs. */
  std::string GetEffectiveFolderName() const;

  cmTarget* Target;
  cmMakefile* Makefile;
  cmLocalGenerator* LocalGenerator;
  cmGlobalGenerator const* GlobalGenerator;

  std::string targetLabelsString;

  struct ModuleDefinitionInfo
  {
    std::string DefFile;
    bool DefFileGenerated;
    bool WindowsExportAllSymbols;
    std::vector<cmSourceFile const*> Sources;
  };
  ModuleDefinitionInfo const* GetModuleDefinitionInfo(
    std::string const& config) const;

  /** Return whether or not we are targeting AIX. */
  bool IsAIX() const;
  /** Return whether or not we are targeting Apple. */
  bool IsApple() const;

  /** Return whether or not the target is for a DLL platform.  */
  bool IsDLLPlatform() const;

  /** @return whether this target have a well defined output file name. */
  bool HaveWellDefinedOutputFiles() const;

  /** Link information from the transitive closure of the link
      implementation and the interfaces of its dependencies.  */
  struct LinkClosure
  {
    // The preferred linker language.
    std::string LinkerLanguage;

    // Languages whose runtime libraries must be linked.
    std::vector<std::string> Languages;
  };

  LinkClosure const* GetLinkClosure(std::string const& config) const;

  cmLinkImplementation const* GetLinkImplementation(std::string const& config,
                                                    UseTo usage) const;

  void ComputeLinkImplementationLanguages(
    std::string const& config, cmOptionalLinkImplementation& impl) const;

  cmLinkImplementationLibraries const* GetLinkImplementationLibraries(
    std::string const& config, UseTo usage) const;

  void ComputeLinkImplementationLibraries(std::string const& config,
                                          cmOptionalLinkImplementation& impl,
                                          UseTo usage) const;

  struct TargetOrString
  {
    std::string String;
    cmGeneratorTarget* Target = nullptr;
  };
  TargetOrString ResolveTargetReference(std::string const& name) const;
  TargetOrString ResolveTargetReference(std::string const& name,
                                        cmLocalGenerator const* lg) const;

  cmLinkItem ResolveLinkItem(
    BT<std::string> const& name,
    std::string const& linkFeature = cmLinkItem::DEFAULT) const;
  cmLinkItem ResolveLinkItem(
    BT<std::string> const& name, cmLocalGenerator const* lg,
    std::string const& linkFeature = cmLinkItem::DEFAULT) const;

  bool HasPackageReferences() const;
  std::vector<std::string> GetPackageReferences() const;

  // Compute the set of languages compiled by the target.  This is
  // computed every time it is called because the languages can change
  // when source file properties are changed and we do not have enough
  // information to forward these property changes to the targets
  // until we have per-target object file properties.
  void GetLanguages(std::set<std::string>& languages,
                    std::string const& config) const;
  bool IsLanguageUsed(std::string const& language,
                      std::string const& config) const;

  // Get the set of targets directly referenced via `TARGET_OBJECTS` in the
  // source list for a configuration.
  std::set<cmGeneratorTarget const*> GetSourceObjectLibraries(
    std::string const& config) const;

  bool IsCSharpOnly() const;

  bool IsDotNetSdkTarget() const;

  void GetObjectLibrariesInSources(
    std::vector<cmGeneratorTarget*>& objlibs) const;

  std::string GetFullNameImported(std::string const& config,
                                  cmStateEnums::ArtifactType artifact) const;

  /** Get source files common to all configurations and diagnose cases
      with per-config sources.  Excludes sources added by a TARGET_OBJECTS
      generator expression.  Do not use outside the Xcode generator.  */
  bool GetConfigCommonSourceFilesForXcode(
    std::vector<cmSourceFile*>& files) const;

  bool HaveBuildTreeRPATH(std::string const& config) const;

  /** Full path with trailing slash to the top-level directory
      holding object files for this target.  Includes the build
      time config name placeholder if needed for the generator.  */
  std::string ObjectDirectory;

  /** Full path with trailing slash to the top-level directory
      holding object files for the given configuration.  */
  std::string GetObjectDirectory(std::string const& config) const;

  std::vector<std::string> GetAppleArchs(std::string const& config,
                                         cm::optional<std::string> lang) const;

  std::string const& GetTargetLabelsString();

  // The classification of the flag.
  enum class FlagClassification
  {
    // The flag is for the execution of the tool (e.g., the compiler itself,
    // any launchers, etc.).
    ExecutionFlag,
    // The flag is "baseline" and should be apply to TUs which may interact
    // with this compilation (e.g., imported modules).
    BaselineFlag,
    // The flag is "private" and doesn't need to apply to interacting TUs.
    PrivateFlag,
    // Flags for the TU itself (e.g., output paths, dependency scanning, etc.).
    LocationFlag,
  };
  enum class FlagKind
  {
    // Not a flag (executable or other entries).
    NotAFlag,
    // Flags for support of the build system.
    BuildSystem,
    // A compilation flag.
    Compile,
    // An include flag.
    Include,
    // A compile definition.
    Definition,
  };
  struct ClassifiedFlag
  {
    ClassifiedFlag(FlagClassification cls, FlagKind kind, std::string flag)
      : Classification(cls)
      , Kind(kind)
      , Flag(std::move(flag))
    {
    }

    FlagClassification Classification;
    FlagKind Kind;
    std::string Flag;
  };
  using ClassifiedFlags = std::vector<ClassifiedFlag>;
  ClassifiedFlags GetClassifiedFlagsForSource(cmSourceFile const* sf,
                                              std::string const& config);
  struct SourceVariables
  {
    std::string TargetPDB;
    std::string TargetCompilePDB;
    std::string ObjectDir;
    std::string ObjectFileDir;
    std::string DependencyFile;
    std::string DependencyTarget;

    // Dependency flags (if used)
    std::string DependencyFlags;
  };
  SourceVariables GetSourceVariables(cmSourceFile const* sf,
                                     std::string const& config);

  void AddExplicitLanguageFlags(std::string& flags,
                                cmSourceFile const& sf) const;

  void AddCUDAArchitectureFlags(cmBuildStep compileOrLink,
                                std::string const& config,
                                std::string& flags) const;
  void AddCUDAArchitectureFlagsImpl(cmBuildStep compileOrLink,
                                    std::string const& config,
                                    std::string const& lang, std::string arch,
                                    std::string& flags) const;
  void AddCUDAToolkitFlags(std::string& flags) const;

  void AddHIPArchitectureFlags(cmBuildStep compileOrLink,
                               std::string const& config,
                               std::string& flags) const;

  void AddISPCTargetFlags(std::string& flags) const;

  std::string GetFeatureSpecificLinkRuleVariable(
    std::string const& var, std::string const& lang,
    std::string const& config) const;

  /** Return the rule variable used to create this type of target.  */
  std::string GetCreateRuleVariable(std::string const& lang,
                                    std::string const& config) const;

  std::string GetClangTidyExportFixesDirectory(std::string const& lang) const;

  /** Return the swift module name for this target. */
  std::string GetSwiftModuleName() const;

  /** Return the path of the `.swiftmodule` for this target in
      the given configuration.  */
  std::string GetSwiftModulePath(std::string const& config) const;

  /** Return the directory containing Swift module interface
      descriptions for this target (including its `.swiftmodule`,
      `.abi.json`, and `.swiftdoc`) in the given configuration.  */
  std::string GetSwiftModuleDirectory(std::string const& config) const;

private:
  /** Return the given property of this target if it exists; otherwise
      return defaultValue. */
  std::string GetPropertyOrDefault(std::string const& property,
                                   std::string defaultValue) const;

  /** Return the name of the `.swiftmodule` file for this target. */
  std::string GetSwiftModuleFileName() const;

  using ConfigAndLanguage = std::pair<std::string, std::string>;
  using ConfigAndLanguageToBTStrings =
    std::map<ConfigAndLanguage, std::vector<BT<std::string>>>;
  mutable ConfigAndLanguageToBTStrings IncludeDirectoriesCache;
  mutable ConfigAndLanguageToBTStrings CompileOptionsCache;
  mutable ConfigAndLanguageToBTStrings CompileDefinitionsCache;
  mutable ConfigAndLanguageToBTStrings PrecompileHeadersCache;
  mutable ConfigAndLanguageToBTStrings LinkOptionsCache;
  mutable ConfigAndLanguageToBTStrings LinkDirectoriesCache;

public:
  /** Get the include directories for this target.  */
  std::vector<BT<std::string>> GetIncludeDirectories(
    std::string const& config, std::string const& lang) const;

  void GetCompileOptions(std::vector<std::string>& result,
                         std::string const& config,
                         std::string const& language) const;
  std::vector<BT<std::string>> GetCompileOptions(
    std::string const& config, std::string const& language) const;

  void GetCompileFeatures(std::vector<std::string>& features,
                          std::string const& config) const;
  std::vector<BT<std::string>> GetCompileFeatures(
    std::string const& config) const;

  void GetCompileDefinitions(std::vector<std::string>& result,
                             std::string const& config,
                             std::string const& language) const;
  std::vector<BT<std::string>> GetCompileDefinitions(
    std::string const& config, std::string const& language) const;

  void GetLinkOptions(std::vector<std::string>& result,
                      std::string const& config,
                      std::string const& language) const;
  std::vector<BT<std::string>> GetLinkOptions(
    std::string const& config, std::string const& language) const;

  std::vector<BT<std::string>>& ResolveLinkerWrapper(
    std::vector<BT<std::string>>& result, std::string const& language,
    bool joinItems = false) const;

  void GetStaticLibraryLinkOptions(std::vector<std::string>& result,
                                   std::string const& config,
                                   std::string const& language) const;
  std::vector<BT<std::string>> GetStaticLibraryLinkOptions(
    std::string const& config, std::string const& language) const;

  std::vector<BT<std::string>>& ResolveArchiverWrapper(
    std::vector<BT<std::string>>& result, std::string const& language,
    bool joinItems = false) const;

  void GetLinkDirectories(std::vector<std::string>& result,
                          std::string const& config,
                          std::string const& language) const;
  std::vector<BT<std::string>> GetLinkDirectories(
    std::string const& config, std::string const& language) const;

  void GetLinkDepends(std::vector<std::string>& result,
                      std::string const& config,
                      std::string const& language) const;
  std::vector<BT<std::string>> GetLinkDepends(
    std::string const& config, std::string const& language) const;

  std::vector<BT<std::string>> GetPrecompileHeaders(
    std::string const& config, std::string const& language) const;

  std::vector<std::string> GetPchArchs(std::string const& config,
                                       std::string const& lang) const;
  std::string GetPchHeader(std::string const& config,
                           std::string const& language,
                           std::string const& arch = std::string()) const;
  std::string GetPchSource(std::string const& config,
                           std::string const& language,
                           std::string const& arch = std::string()) const;
  std::string GetPchFileObject(std::string const& config,
                               std::string const& language,
                               std::string const& arch = std::string());
  std::string GetPchFile(std::string const& config,
                         std::string const& language,
                         std::string const& arch = std::string());
  std::string GetPchCreateCompileOptions(
    std::string const& config, std::string const& language,
    std::string const& arch = std::string());
  std::string GetPchUseCompileOptions(std::string const& config,
                                      std::string const& language,
                                      std::string const& arch = std::string());

  void AddSourceFileToUnityBatch(std::string const& sourceFilename);
  bool IsSourceFilePartOfUnityBatch(std::string const& sourceFilename) const;

  bool IsSystemIncludeDirectory(std::string const& dir,
                                std::string const& config,
                                std::string const& language) const;

  void AddSystemIncludeCacheKey(std::string const& key,
                                std::string const& config,
                                std::string const& language) const;

  /** Add the target output files to the global generator manifest.  */
  void ComputeTargetManifest(std::string const& config) const;

  bool ComputeCompileFeatures(std::string const& config);

  using LanguagePair = std::pair<std::string, std::string>;
  bool ComputeCompileFeatures(std::string const& config,
                              std::set<LanguagePair> const& languagePairs);

  /**
   * Trace through the source files in this target and add al source files
   * that they depend on, used by all generators
   */
  void TraceDependencies();

  /** Get the directory in which this target will be built.  If the
      configuration name is given then the generator will add its
      subdirectory for that configuration.  Otherwise just the canonical
      output directory is given.  */
  std::string GetDirectory(std::string const& config,
                           cmStateEnums::ArtifactType artifact =
                             cmStateEnums::RuntimeBinaryArtifact) const;

  /** Get the directory in which to place the target compiler .pdb file.
      If the configuration name is given then the generator will add its
      subdirectory for that configuration.  Otherwise just the canonical
      compiler pdb output directory is given.  */
  std::string GetCompilePDBDirectory(std::string const& config) const;

  /** Get sources that must be built before the given source.  */
  std::vector<cmSourceFile*> const* GetSourceDepends(
    cmSourceFile const* sf) const;

  /** Return whether this target uses the default value for its output
      directory.  */
  bool UsesDefaultOutputDir(std::string const& config,
                            cmStateEnums::ArtifactType artifact) const;

  // Cache target output paths for each configuration.
  struct OutputInfo
  {
    std::string OutDir;
    std::string ImpDir;
    std::string PdbDir;
    bool empty() const
    {
      return this->OutDir.empty() && this->ImpDir.empty() &&
        this->PdbDir.empty();
    }
  };

  OutputInfo const* GetOutputInfo(std::string const& config) const;

  // Get the target PDB base name.
  std::string GetPDBOutputName(std::string const& config) const;

  /** Get the name of the pdb file for the target.  */
  std::string GetPDBName(std::string const& config) const;

  /** Whether this library has soname enabled and platform supports it.  */
  bool HasSOName(std::string const& config) const;

  struct CompileInfo
  {
    std::string CompilePdbDir;
  };

  CompileInfo const* GetCompileInfo(std::string const& config) const;

  using CompileInfoMapType = std::map<std::string, CompileInfo>;
  mutable CompileInfoMapType CompileInfoMap;

  bool IsNullImpliedByLinkLibraries(std::string const& p) const;

  /** Get the name of the compiler pdb file for the target.  */
  std::string GetCompilePDBName(std::string const& config) const;

  /** Get the path for the MSVC /Fd option for this target.  */
  std::string GetCompilePDBPath(std::string const& config) const;

  // Get the target base name.
  std::string GetOutputName(std::string const& config,
                            cmStateEnums::ArtifactType artifact) const;

  /** Get target file prefix */
  std::string GetFilePrefix(std::string const& config,
                            cmStateEnums::ArtifactType artifact =
                              cmStateEnums::RuntimeBinaryArtifact) const;
  /** Get target file prefix */
  std::string GetFileSuffix(std::string const& config,
                            cmStateEnums::ArtifactType artifact =
                              cmStateEnums::RuntimeBinaryArtifact) const;

  /** Get target file postfix */
  std::string GetFilePostfix(std::string const& config) const;

  /** Get framework multi-config-specific postfix */
  std::string GetFrameworkMultiConfigPostfix(std::string const& config) const;

  /** Clears cached meta data for local and external source files.
   * The meta data will be recomputed on demand.
   */
  void ClearSourcesCache();

  /**
   * Clears cached evaluations of INTERFACE_LINK_LIBRARIES.
   * They will be recomputed on demand.
   */
  void ClearLinkInterfaceCache();

  void AddSource(std::string const& src, bool before = false);
  void AddTracedSources(std::vector<std::string> const& srcs);

  /**
   * Adds an entry to the INCLUDE_DIRECTORIES list.
   * If before is true the entry is pushed at the front.
   */
  void AddIncludeDirectory(std::string const& src, bool before = false);

  /**
   * Flags for a given source file as used in this target. Typically assigned
   * via SET_TARGET_PROPERTIES when the property is a list of source files.
   */
  enum SourceFileType
  {
    SourceFileTypeNormal,
    SourceFileTypePrivateHeader, // is in "PRIVATE_HEADER" target property
    SourceFileTypePublicHeader,  // is in "PUBLIC_HEADER" target property
    SourceFileTypeResource,      // is in "RESOURCE" target property *or*
                                 // has MACOSX_PACKAGE_LOCATION=="Resources"
    SourceFileTypeDeepResource,  // MACOSX_PACKAGE_LOCATION starts with
                                 // "Resources/"
    SourceFileTypeMacContent     // has MACOSX_PACKAGE_LOCATION!="Resources[/]"
  };
  struct SourceFileFlags
  {
    SourceFileType Type = SourceFileTypeNormal;
    char const* MacFolder = nullptr; // location inside Mac content folders
  };
  void GetAutoUicOptions(std::vector<std::string>& result,
                         std::string const& config) const;

  struct Names
  {
    std::string Base;
    std::string Output;
    std::string Real;
    std::string ImportOutput;
    std::string ImportReal;
    std::string ImportLibrary;
    std::string PDB;
    std::string SharedObject;
  };

  /** Get the names of the executable needed to generate a build rule
      that takes into account executable version numbers.  This should
      be called only on an executable target.  */
  Names GetExecutableNames(std::string const& config) const;

  /** Get the names of the library needed to generate a build rule
      that takes into account shared library version numbers.  This
      should be called only on a library target.  */
  Names GetLibraryNames(std::string const& config) const;

  /**
   * Compute whether this target must be relinked before installing.
   */
  bool NeedRelinkBeforeInstall(std::string const& config) const;

  /** Return true if builtin chrpath will work for this target */
  bool IsChrpathUsed(std::string const& config) const;

  /** Get the directory in which this targets .pdb files will be placed.
      If the configuration name is given then the generator will add its
      subdirectory for that configuration.  Otherwise just the canonical
      pdb output directory is given.  */
  std::string GetPDBDirectory(std::string const& config) const;

  //! Return the preferred linker language for this target
  std::string GetLinkerLanguage(std::string const& config) const;
  //! Return the preferred linker tool for this target
  std::string GetLinkerTool(std::string const& config) const;
  std::string GetLinkerTool(std::string const& lang,
                            std::string const& config) const;

  /** Is the linker known to enforce '--no-allow-shlib-undefined'? */
  bool LinkerEnforcesNoAllowShLibUndefined(std::string const& config) const;

  /** Does this target have a GNU implib to convert to MS format?  */
  bool HasImplibGNUtoMS(std::string const& config) const;

  /** Convert the given GNU import library name (.dll.a) to a name with a new
      extension (.lib or ${CMAKE_IMPORT_LIBRARY_SUFFIX}).  */
  bool GetImplibGNUtoMS(std::string const& config, std::string const& gnuName,
                        std::string& out, char const* newExt = nullptr) const;

  /** Can only ever return true if GetSourceFilePaths() was called before.
      Otherwise, this is indeterminate and false will be assumed/returned!  */
  bool HasContextDependentSources() const;

  bool IsExecutableWithExports() const;

  /* Return whether this target is a shared library with capability to generate
   * a file describing symbols exported (for example, .tbd file on Apple). */
  bool IsSharedLibraryWithExports() const;

  /** Return whether or not the target has a DLL import library.  */
  bool HasImportLibrary(std::string const& config) const;

  /** Get a build-tree directory in which to place target support files.  */
  std::string GetSupportDirectory() const;

  /** Return whether this target may be used to link another target.  */
  bool IsLinkable() const;

  /** Return whether the link step generates a dependency file. */
  bool HasLinkDependencyFile(std::string const& config) const;

  /** Return whether this target is a shared library Framework on
      Apple.  */
  bool IsFrameworkOnApple() const;

  /** Return whether this target is an IMPORTED library target on Apple
      with a .framework folder as its location.  */
  bool IsImportedFrameworkFolderOnApple(std::string const& config) const;

  /** Return whether this target is an executable Bundle on Apple.  */
  bool IsAppBundleOnApple() const;

  /** Return whether this target is a XCTest on Apple.  */
  bool IsXCTestOnApple() const;

  /** Return whether this target is a CFBundle (plugin) on Apple.  */
  bool IsCFBundleOnApple() const;

  /** Return whether this target is a shared library on AIX.  */
  bool IsArchivedAIXSharedLibrary() const;

  /** Assembly types. The order of the values of this enum is relevant
      because of smaller/larger comparison operations! */
  enum ManagedType
  {
    Undefined = 0, // target is no lib or executable
    Native,        // target compiles to unmanaged binary.
    Mixed,         // target compiles to mixed (managed and unmanaged) binary.
    Managed        // target compiles to managed binary.
  };

  /** Return the type of assembly this target compiles to. */
  ManagedType GetManagedType(std::string const& config) const;

  struct SourceFileFlags GetTargetSourceFileFlags(
    cmSourceFile const* sf) const;

  void ReportPropertyOrigin(std::string const& p, std::string const& result,
                            std::string const& report,
                            std::string const& compatibilityType) const;

  class TargetPropertyEntry;

  std::string EvaluateInterfaceProperty(
    std::string const& prop, cmGeneratorExpressionContext* context,
    cmGeneratorExpressionDAGChecker* dagCheckerParent, UseTo usage) const;

  struct TransitiveProperty
  {
#if defined(__SUNPRO_CC) || (defined(__ibmxl__) && defined(__clang__))
    TransitiveProperty(cm::string_view interfaceName, UseTo usage)
      : InterfaceName(interfaceName)
      , Usage(usage)
    {
    }
#endif
    cm::string_view InterfaceName;
    UseTo Usage;
  };

  static std::map<cm::string_view, TransitiveProperty> const
    BuiltinTransitiveProperties;

  cm::optional<TransitiveProperty> IsTransitiveProperty(
    cm::string_view prop, cmLocalGenerator const* lg,
    std::string const& config,
    cmGeneratorExpressionDAGChecker const* dagChecker) const;

  bool HaveInstallTreeRPATH(std::string const& config) const;

  bool GetBuildRPATH(std::string const& config, std::string& rpath) const;
  bool GetInstallRPATH(std::string const& config, std::string& rpath) const;

  /** Whether this library has \@rpath and platform supports it.  */
  bool HasMacOSXRpathInstallNameDir(std::string const& config) const;

  /** Whether this library defaults to \@rpath.  */
  bool MacOSXRpathInstallNameDirDefault() const;

  enum InstallNameType
  {
    INSTALL_NAME_FOR_BUILD,
    INSTALL_NAME_FOR_INSTALL
  };
  /** Whether to use INSTALL_NAME_DIR. */
  bool MacOSXUseInstallNameDir() const;
  /** Whether to generate an install_name. */
  bool CanGenerateInstallNameDir(InstallNameType t) const;

  /** Test for special case of a third-party shared library that has
      no soname at all.  */
  bool IsImportedSharedLibWithoutSOName(std::string const& config) const;

  std::string ImportedGetLocation(std::string const& config) const;

  /** Get the target major and minor version numbers interpreted from
      the VERSION property.  Version 0 is returned if the property is
      not set or cannot be parsed.  */
  void GetTargetVersion(int& major, int& minor) const;

  /** Get the target major, minor, and patch version numbers
      interpreted from the given property.  Version 0
      is returned if the property is not set or cannot be parsed.  */
  void GetTargetVersion(std::string const& property, int& major, int& minor,
                        int& patch) const;

  /** Get the target major, minor, and patch version numbers
      interpreted from the given property and if empty use the
      fallback property.  Version 0 is returned if the property is
      not set or cannot be parsed.  */
  void GetTargetVersionFallback(std::string const& property,
                                std::string const& fallback_property,
                                int& major, int& minor, int& patch) const;

  std::string GetRuntimeLinkLibrary(std::string const& lang,
                                    std::string const& config) const;

  std::string GetFortranModuleDirectory(std::string const& working_dir) const;
  bool IsFortranBuildingIntrinsicModules() const;

  bool IsLinkLookupScope(std::string const& n,
                         cmLocalGenerator const*& lg) const;

  cmValue GetSourcesProperty() const;

  void AddISPCGeneratedHeader(std::string const& header,
                              std::string const& config);
  std::vector<std::string> GetGeneratedISPCHeaders(
    std::string const& config) const;

  void AddISPCGeneratedObject(std::vector<std::string>&& objs,
                              std::string const& config);
  std::vector<std::string> GetGeneratedISPCObjects(
    std::string const& config) const;

  void AddSystemIncludeDirectory(std::string const& inc,
                                 std::string const& lang);
  bool AddHeaderSetVerification();
  std::string GenerateHeaderSetVerificationFile(
    cmSourceFile& source, std::string const& dir,
    cm::optional<std::set<std::string>>& languages) const;

  std::string GetImportedXcFrameworkPath(std::string const& config) const;

  bool ApplyCXXStdTargets();
  bool DiscoverSyntheticTargets(cmSyntheticTargetCache& cache,
                                std::string const& config);

  class CustomTransitiveProperty : public TransitiveProperty
  {
    std::unique_ptr<std::string> InterfaceNameBuf;
    CustomTransitiveProperty(std::unique_ptr<std::string> interfaceNameBuf,
                             UseTo usage);

  public:
    CustomTransitiveProperty(std::string interfaceName, UseTo usage);
  };
  struct CustomTransitiveProperties
    : public std::map<std::string, CustomTransitiveProperty>
  {
    void Add(cmValue props, UseTo usage);
  };

  enum class PropertyFor
  {
    Build,
    Interface,
  };

  CustomTransitiveProperties const& GetCustomTransitiveProperties(
    std::string const& config, PropertyFor propertyFor) const;

private:
  void AddSourceCommon(std::string const& src, bool before = false);

  std::string CreateFortranModuleDirectory(
    std::string const& working_dir) const;
  mutable bool FortranModuleDirectoryCreated = false;
  mutable std::string FortranModuleDirectory;

  friend class cmTargetTraceDependencies;
  struct SourceEntry
  {
    std::vector<cmSourceFile*> Depends;
  };
  using SourceEntriesType = std::map<cmSourceFile const*, SourceEntry>;
  SourceEntriesType SourceDepends;
  mutable std::set<std::string> VisitedConfigsForObjects;
  mutable std::map<cmSourceFile const*, std::string> Objects;
  std::set<cmSourceFile const*> ExplicitObjectName;

  using TargetPtrToBoolMap = std::unordered_map<cmTarget*, bool>;
  mutable std::unordered_map<std::string, TargetPtrToBoolMap>
    MacOSXRpathInstallNameDirCache;
  bool DetermineHasMacOSXRpathInstallNameDir(std::string const& config) const;

  // "config/language" is the key
  mutable std::map<std::string, std::vector<std::string>> SystemIncludesCache;

  mutable std::string ExportMacro;

  void ConstructSourceFileFlags() const;
  mutable bool SourceFileFlagsConstructed = false;
  mutable std::map<cmSourceFile const*, SourceFileFlags> SourceFlagsMap;

  mutable std::map<std::string, bool> DebugCompatiblePropertiesDone;

  bool NeedImportLibraryName(std::string const& config) const;

  cmValue GetFilePrefixInternal(std::string const& config,
                                cmStateEnums::ArtifactType artifact,
                                std::string const& language = "") const;
  cmValue GetFileSuffixInternal(std::string const& config,
                                cmStateEnums::ArtifactType artifact,
                                std::string const& language = "") const;

  std::string GetFullNameInternal(std::string const& config,
                                  cmStateEnums::ArtifactType artifact) const;

  using FullNameCache = std::map<std::string, NameComponents>;

  mutable FullNameCache RuntimeBinaryFullNameCache;
  mutable FullNameCache ImportLibraryFullNameCache;

  NameComponents const& GetFullNameInternalComponents(
    std::string const& config, cmStateEnums::ArtifactType artifact) const;

  mutable std::string LinkerLanguage;
  using LinkClosureMapType = std::map<std::string, LinkClosure>;
  mutable LinkClosureMapType LinkClosureMap;
  bool DeviceLink = false;

  // Returns ARCHIVE, LIBRARY, or RUNTIME based on platform and type.
  char const* GetOutputTargetType(cmStateEnums::ArtifactType artifact) const;

  std::string ComputeVersionedName(std::string const& prefix,
                                   std::string const& base,
                                   std::string const& suffix,
                                   std::string const& name,
                                   cmValue version) const;

  mutable std::map<std::string, CustomTransitiveProperties>
    CustomTransitiveBuildPropertiesMap;
  mutable std::map<std::string, CustomTransitiveProperties>
    CustomTransitiveInterfacePropertiesMap;

  struct CompatibleInterfacesBase
  {
    std::set<std::string> PropsBool;
    std::set<std::string> PropsString;
    std::set<std::string> PropsNumberMax;
    std::set<std::string> PropsNumberMin;
  };
  CompatibleInterfacesBase const& GetCompatibleInterfaces(
    std::string const& config) const;

  struct CompatibleInterfaces : public CompatibleInterfacesBase
  {
    bool Done = false;
  };
  mutable std::map<std::string, CompatibleInterfaces> CompatibleInterfacesMap;

  using cmTargetLinkInformationMap =
    std::map<std::string, std::unique_ptr<cmComputeLinkInformation>>;
  mutable cmTargetLinkInformationMap LinkInformation;

  void CheckPropertyCompatibility(cmComputeLinkInformation& info,
                                  std::string const& config) const;

  void ComputeLinkClosure(std::string const& config, LinkClosure& lc) const;
  bool ComputeLinkClosure(std::string const& config, LinkClosure& lc,
                          bool secondPass) const;

  struct LinkImplClosure : public std::vector<cmGeneratorTarget const*>
  {
    bool Done = false;
  };
  mutable std::map<std::string, LinkImplClosure> LinkImplClosureForLinkMap;
  mutable std::map<std::string, LinkImplClosure> LinkImplClosureForUsageMap;

  using LinkInterfaceMapType = std::map<std::string, cmHeadToLinkInterfaceMap>;
  mutable LinkInterfaceMapType LinkInterfaceMap;
  mutable LinkInterfaceMapType LinkInterfaceUsageRequirementsOnlyMap;

  cmHeadToLinkInterfaceMap& GetHeadToLinkInterfaceMap(
    std::string const& config) const;
  cmHeadToLinkInterfaceMap& GetHeadToLinkInterfaceUsageRequirementsMap(
    std::string const& config) const;

  std::string GetLinkInterfaceDependentStringAsBoolProperty(
    std::string const& p, std::string const& config) const;

  friend class cmTargetCollectLinkLanguages;
  cmLinkInterface const* GetLinkInterface(std::string const& config,
                                          cmGeneratorTarget const* headTarget,
                                          bool secondPass) const;
  void ComputeLinkInterface(std::string const& config,
                            cmOptionalLinkInterface& iface,
                            bool secondPass) const;
  cmLinkImplementation const* GetLinkImplementation(std::string const& config,
                                                    UseTo usage,
                                                    bool secondPass) const;

  enum class LinkItemRole
  {
    Implementation,
    Interface,
  };
  bool VerifyLinkItemIsTarget(LinkItemRole role, cmLinkItem const& item) const;
  bool VerifyLinkItemColons(LinkItemRole role, cmLinkItem const& item) const;

  // Cache import information from properties for each configuration.
  struct ImportInfo
  {
    bool NoSOName = false;
    ManagedType Managed = Native;
    unsigned int Multiplicity = 0;
    std::string Location;
    std::string SOName;
    std::string ImportLibrary;
    std::string LibName;
    std::string Languages;
    std::string LibrariesProp;
    std::vector<BT<std::string>> Libraries;
    std::vector<BT<std::string>> LibrariesHeadInclude;
    std::vector<BT<std::string>> LibrariesHeadExclude;
    std::string SharedDeps;
  };

  using ImportInfoMapType = std::map<std::string, ImportInfo>;
  mutable ImportInfoMapType ImportInfoMap;
  void ComputeImportInfo(std::string const& desired_config,
                         ImportInfo& info) const;
  ImportInfo const* GetImportInfo(std::string const& config) const;

  /** Strip off leading and trailing whitespace from an item named in
      the link dependencies of this target.  */
  std::string CheckCMP0004(std::string const& item) const;

  cmLinkInterface const* GetImportLinkInterface(std::string const& config,
                                                cmGeneratorTarget const* head,
                                                UseTo usage,
                                                bool secondPass = false) const;

  using KindedSourcesMapType = std::map<std::string, KindedSources>;
  mutable KindedSourcesMapType KindedSourcesMap;
  void ComputeKindedSources(KindedSources& files,
                            std::string const& config) const;

  mutable std::vector<AllConfigSource> AllConfigSources;
  void ComputeAllConfigSources() const;

  mutable std::unordered_map<std::string, bool> MaybeInterfacePropertyExists;
  bool MaybeHaveInterfaceProperty(std::string const& prop,
                                  cmGeneratorExpressionContext* context,
                                  UseTo usage) const;

  using TargetPropertyEntryVector =
    std::vector<std::unique_ptr<TargetPropertyEntry>>;

  TargetPropertyEntryVector IncludeDirectoriesEntries;
  TargetPropertyEntryVector CompileOptionsEntries;
  TargetPropertyEntryVector CompileFeaturesEntries;
  TargetPropertyEntryVector CompileDefinitionsEntries;
  TargetPropertyEntryVector LinkOptionsEntries;
  TargetPropertyEntryVector LinkDirectoriesEntries;
  TargetPropertyEntryVector PrecompileHeadersEntries;
  TargetPropertyEntryVector SourceEntries;
  mutable std::set<std::string> LinkImplicitNullProperties;
  mutable std::map<std::string, std::string> PchHeaders;
  mutable std::map<std::string, std::string> PchSources;
  mutable std::map<std::string, std::string> PchObjectFiles;
  mutable std::map<std::string, std::string> PchFiles;
  mutable std::map<std::string, std::string> PchCreateCompileOptions;
  mutable std::map<std::string, std::string> PchUseCompileOptions;

  std::unordered_set<std::string> UnityBatchedSourceFiles;

  std::unordered_map<std::string, std::vector<std::string>>
    ISPCGeneratedHeaders;
  std::unordered_map<std::string, std::vector<std::string>>
    ISPCGeneratedObjects;

  enum class LinkInterfaceField
  {
    Libraries,
    HeadExclude,
    HeadInclude,
  };
  void ExpandLinkItems(std::string const& prop, cmBTStringRange entries,
                       std::string const& config,
                       cmGeneratorTarget const* headTarget, UseTo usage,
                       LinkInterfaceField field, cmLinkInterface& iface) const;

  struct LookupLinkItemScope
  {
    cmLocalGenerator const* LG;
  };
  enum class LookupSelf
  {
    No,
    Yes,
  };
  cm::optional<cmLinkItem> LookupLinkItem(std::string const& n,
                                          cmListFileBacktrace const& bt,
                                          std::string const& linkFeature,
                                          LookupLinkItemScope* scope,
                                          LookupSelf lookupSelf) const;

  std::vector<BT<std::string>>& ResolvePrefixWrapper(
    std::vector<BT<std::string>>& result, cm::string_view prefix,
    std::string const& language, bool joinItems) const;

  std::vector<BT<std::string>> GetSourceFilePaths(
    std::string const& config) const;
  std::vector<BT<cmSourceFile*>> GetSourceFilesWithoutObjectLibraries(
    std::string const& config) const;
  void GetSourceFilesWithoutObjectLibraries(std::vector<cmSourceFile*>& files,
                                            std::string const& config) const;

  using LinkImplMapType = std::map<std::string, cmOptionalLinkImplementation>;
  mutable LinkImplMapType LinkImplMap;
  mutable LinkImplMapType LinkImplUsageRequirementsOnlyMap;

  bool ComputeOutputDir(std::string const& config,
                        cmStateEnums::ArtifactType artifact,
                        std::string& out) const;

  using OutputInfoMapType = std::map<std::string, OutputInfo>;
  mutable OutputInfoMapType OutputInfoMap;

  using ModuleDefinitionInfoMapType =
    std::map<std::string, ModuleDefinitionInfo>;
  mutable ModuleDefinitionInfoMapType ModuleDefinitionInfoMap;
  void ComputeModuleDefinitionInfo(std::string const& config,
                                   ModuleDefinitionInfo& info) const;

  using OutputNameKey = std::pair<std::string, cmStateEnums::ArtifactType>;
  using OutputNameMapType = std::map<OutputNameKey, std::string>;
  mutable OutputNameMapType OutputNameMap;
  mutable std::set<cmLinkItem> UtilityItems;
  cmPolicies::PolicyMap PolicyMap;
  mutable bool PolicyReportedCMP0069 = false;
  mutable bool DebugIncludesDone = false;
  mutable bool DebugCompileOptionsDone = false;
  mutable bool DebugCompileFeaturesDone = false;
  mutable bool DebugCompileDefinitionsDone = false;
  mutable bool DebugLinkOptionsDone = false;
  mutable bool DebugLinkDirectoriesDone = false;
  mutable bool DebugPrecompileHeadersDone = false;
  mutable bool DebugSourcesDone = false;
  mutable bool UtilityItemsDone = false;
  enum class Tribool
  {
    False = 0x0,
    True = 0x1,
    Indeterminate = 0x2
  };
  mutable Tribool SourcesAreContextDependent = Tribool::Indeterminate;

  bool ComputePDBOutputDir(std::string const& kind, std::string const& config,
                           std::string& out) const;

  ManagedType CheckManagedType(std::string const& propval) const;

  bool GetRPATH(std::string const& config, std::string const& prop,
                std::string& rpath) const;

  std::map<std::string, BTs<std::string>> LanguageStandardMap;

  cm::optional<cmStandardLevel> GetExplicitStandardLevel(
    std::string const& lang, std::string const& config) const;
  void UpdateExplicitStandardLevel(std::string const& lang,
                                   std::string const& config,
                                   cmStandardLevel level);
  std::map<std::string, cmStandardLevel> ExplicitStandardLevel;

  cmValue GetPropertyWithPairedLanguageSupport(std::string const& lang,
                                               char const* suffix) const;

  void ComputeLinkImplementationRuntimeLibraries(
    std::string const& config, cmOptionalLinkImplementation& impl) const;

  void ComputeLinkInterfaceRuntimeLibraries(
    std::string const& config, cmOptionalLinkInterface& iface) const;

  // If this method is made public, or call sites are added outside of
  // methods computing cached members, add dedicated caching members.
  std::vector<cmGeneratorTarget const*> GetLinkInterfaceClosure(
    std::string const& config, cmGeneratorTarget const* headTarget,
    UseTo usage) const;

public:
  std::vector<cmGeneratorTarget const*> const& GetLinkImplementationClosure(
    std::string const& config, UseTo usage) const;

  mutable std::map<std::string, std::string> MaxLanguageStandards;
  std::map<std::string, std::string> const& GetMaxLanguageStandards() const
  {
    return this->MaxLanguageStandards;
  }

  struct StrictTargetComparison
  {
    bool operator()(cmGeneratorTarget const* t1,
                    cmGeneratorTarget const* t2) const;
  };

  bool HaveFortranSources() const;
  bool HaveFortranSources(std::string const& config) const;

  // C++20 module support queries.

  /**
   * Query whether the target expects C++20 module support.
   *
   * This will inspect the target itself to see if C++20 module
   * support is expected to work based on its sources.
   *
   * If `errorMessage` is given a non-`nullptr`, any error message will be
   * stored in it, otherwise the error will be reported directly.
   */
  bool HaveCxx20ModuleSources(std::string* errorMessage = nullptr) const;

  enum class Cxx20SupportLevel
  {
    // C++ is not available.
    MissingCxx,
    // The target does not require at least C++20.
    NoCxx20,
    // C++20 module scanning rules are not present.
    MissingRule,
    // C++20 modules are available and working.
    Supported,
  };
  /**
   * Query whether the target has C++20 module support available (regardless of
   * whether it is required or not).
   */
  Cxx20SupportLevel HaveCxxModuleSupport(std::string const& config) const;

  // Check C++ module status for the target.
  void CheckCxxModuleStatus(std::string const& config) const;

  bool NeedCxxModuleSupport(std::string const& lang,
                            std::string const& config) const;
  bool NeedDyndep(std::string const& lang, std::string const& config) const;
  cmFileSet const* GetFileSetForSource(std::string const& config,
                                       cmSourceFile const* sf) const;
  bool NeedDyndepForSource(std::string const& lang, std::string const& config,
                           cmSourceFile const* sf) const;
  enum class CxxModuleSupport
  {
    Unavailable,
    Enabled,
    Disabled,
  };
  CxxModuleSupport NeedCxxDyndep(std::string const& config) const;

  std::string BuildDatabasePath(std::string const& lang,
                                std::string const& config) const;

private:
  void BuildFileSetInfoCache(std::string const& config) const;
  struct InfoByConfig
  {
    bool BuiltFileSetCache = false;
    std::map<std::string, cmFileSet const*> FileSetCache;
    std::map<cmGeneratorTarget const*, std::vector<cmGeneratorTarget const*>>
      SyntheticDeps;
    std::map<cmSourceFile const*, ClassifiedFlags> SourceFlags;
  };
  mutable std::map<std::string, InfoByConfig> Configs;
};

class cmGeneratorTarget::TargetPropertyEntry
{
protected:
  static cmLinkImplItem NoLinkImplItem;

public:
  TargetPropertyEntry(cmLinkImplItem const& item);
  virtual ~TargetPropertyEntry() = default;

  static std::unique_ptr<TargetPropertyEntry> Create(
    cmake& cmakeInstance, const BT<std::string>& propertyValue,
    bool evaluateForBuildsystem = false);
  static std::unique_ptr<TargetPropertyEntry> CreateFileSet(
    std::vector<std::string> dirs, bool contextSensitiveDirs,
    std::unique_ptr<cmCompiledGeneratorExpression> entryCge,
    cmFileSet const* fileSet, cmLinkImplItem const& item = NoLinkImplItem);

  virtual std::string const& Evaluate(
    cmLocalGenerator* lg, std::string const& config,
    cmGeneratorTarget const* headTarget,
    cmGeneratorExpressionDAGChecker* dagChecker,
    std::string const& language) const = 0;

  virtual cmListFileBacktrace GetBacktrace() const = 0;
  virtual std::string const& GetInput() const = 0;
  virtual bool GetHadContextSensitiveCondition() const;

  cmLinkImplItem const& LinkImplItem;
};
