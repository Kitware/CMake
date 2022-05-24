/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
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

#include "cmAlgorithms.h"
#include "cmLinkItem.h"
#include "cmListFileCache.h"
#include "cmPolicies.h"
#include "cmStateTypes.h"
#include "cmValue.h"

class cmComputeLinkInformation;
class cmCustomCommand;
class cmGlobalGenerator;
class cmLocalGenerator;
class cmMakefile;
class cmSourceFile;
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
  bool IsImported() const;
  bool IsImportedGloballyVisible() const;
  bool CanCompileSources() const;
  const std::string& GetLocation(const std::string& config) const;

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
  const std::string& GetLocationForBuild() const;

  cmComputeLinkInformation* GetLinkInformation(
    const std::string& config) const;

  // Perform validation checks on memoized link structures.
  // Call this after generation is complete.
  void CheckLinkLibraries() const;

  cmStateEnums::TargetType GetType() const;
  const std::string& GetName() const;
  std::string GetExportName() const;

  std::vector<std::string> GetPropertyKeys() const;
  //! Might return a nullptr if the property is not set or invalid
  cmValue GetProperty(const std::string& prop) const;
  //! Always returns a valid pointer
  std::string const& GetSafeProperty(std::string const& prop) const;
  bool GetPropertyAsBool(const std::string& prop) const;
  void GetSourceFiles(std::vector<cmSourceFile*>& files,
                      const std::string& config) const;
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
                        const std::string& config) const;
  const std::string& GetObjectName(cmSourceFile const* file);
  const char* GetCustomObjectExtension() const;

  bool HasExplicitObjectName(cmSourceFile const* file) const;
  void AddExplicitObjectName(cmSourceFile const* sf);

  BTs<std::string> const* GetLanguageStandardProperty(
    std::string const& lang, std::string const& config) const;

  cmValue GetLanguageStandard(std::string const& lang,
                              std::string const& config) const;

  cmValue GetLanguageExtensions(std::string const& lang) const;

  bool GetLanguageStandardRequired(std::string const& lang) const;

  void GetModuleDefinitionSources(std::vector<cmSourceFile const*>&,
                                  const std::string& config) const;
  void GetExternalObjects(std::vector<cmSourceFile const*>&,
                          const std::string& config) const;
  void GetHeaderSources(std::vector<cmSourceFile const*>&,
                        const std::string& config) const;
  void GetExtraSources(std::vector<cmSourceFile const*>&,
                       const std::string& config) const;
  void GetCustomCommands(std::vector<cmSourceFile const*>&,
                         const std::string& config) const;
  void GetManifests(std::vector<cmSourceFile const*>&,
                    const std::string& config) const;

  std::set<cmLinkItem> const& GetUtilityItems() const;

  void ComputeObjectMapping();

  cmValue GetFeature(const std::string& feature,
                     const std::string& config) const;

  const char* GetLinkPIEProperty(const std::string& config) const;

  bool IsIPOEnabled(std::string const& lang, std::string const& config) const;

  bool IsLinkInterfaceDependentBoolProperty(const std::string& p,
                                            const std::string& config) const;
  bool IsLinkInterfaceDependentStringProperty(const std::string& p,
                                              const std::string& config) const;
  bool IsLinkInterfaceDependentNumberMinProperty(
    const std::string& p, const std::string& config) const;
  bool IsLinkInterfaceDependentNumberMaxProperty(
    const std::string& p, const std::string& config) const;

  bool GetLinkInterfaceDependentBoolProperty(const std::string& p,
                                             const std::string& config) const;

  const char* GetLinkInterfaceDependentStringProperty(
    const std::string& p, const std::string& config) const;
  const char* GetLinkInterfaceDependentNumberMinProperty(
    const std::string& p, const std::string& config) const;
  const char* GetLinkInterfaceDependentNumberMaxProperty(
    const std::string& p, const std::string& config) const;

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
    const std::string& config, const cmGeneratorTarget* headTarget) const;
  void ComputeLinkInterface(const std::string& config,
                            cmOptionalLinkInterface& iface,
                            const cmGeneratorTarget* head) const;

  enum class LinkInterfaceFor
  {
    Usage, // Interface for usage requirements excludes $<LINK_ONLY>.
    Link,  // Interface for linking includes $<LINK_ONLY>.
  };

  cmLinkInterfaceLibraries const* GetLinkInterfaceLibraries(
    const std::string& config, const cmGeneratorTarget* headTarget,
    LinkInterfaceFor interfaceFor) const;

  void ComputeLinkInterfaceLibraries(const std::string& config,
                                     cmOptionalLinkInterface& iface,
                                     const cmGeneratorTarget* head,
                                     LinkInterfaceFor interfaceFor) const;

  /** Get the library name for an imported interface library.  */
  std::string GetImportedLibName(std::string const& config) const;

  /** Get the full path to the target according to the settings in its
      makefile and the configuration type.  */
  std::string GetFullPath(
    const std::string& config,
    cmStateEnums::ArtifactType artifact = cmStateEnums::RuntimeBinaryArtifact,
    bool realname = false) const;
  std::string NormalGetFullPath(const std::string& config,
                                cmStateEnums::ArtifactType artifact,
                                bool realname) const;
  std::string NormalGetRealName(const std::string& config) const;

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
  std::string GetAppBundleDirectory(const std::string& config,
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
  bool IsWin32Executable(const std::string& config) const;

  /** Get the full name of the target according to the settings in its
      makefile.  */
  std::string GetFullName(const std::string& config,
                          cmStateEnums::ArtifactType artifact =
                            cmStateEnums::RuntimeBinaryArtifact) const;

  /** @return the Mac framework directory without the base. */
  std::string GetFrameworkDirectory(const std::string& config,
                                    BundleDirectoryLevel level) const;

  /** Return the framework version string.  Undefined if
      IsFrameworkOnApple returns false.  */
  std::string GetFrameworkVersion() const;

  /** @return the Mac CFBundle directory without the base */
  std::string GetCFBundleDirectory(const std::string& config,
                                   BundleDirectoryLevel level) const;

  /** Return the install name directory for the target in the
   * build tree.  For example: "\@rpath/", "\@loader_path/",
   * or "/full/path/to/library".  */
  std::string GetInstallNameDirForBuildTree(const std::string& config) const;

  /** Return the install name directory for the target in the
   * install tree.  For example: "\@rpath/" or "\@loader_path/". */
  std::string GetInstallNameDirForInstallTree(
    const std::string& config, const std::string& installPrefix) const;

  cmListFileBacktrace GetBacktrace() const;

  std::set<BT<std::pair<std::string, bool>>> const& GetUtilities() const;

  bool LinkLanguagePropagatesToDependents() const
  {
    return this->GetType() == cmStateEnums::STATIC_LIBRARY;
  }

  /** Get the macro to define when building sources in this target.
      If no macro should be defined null is returned.  */
  const std::string* GetExportMacro() const;

  /** Get the soname of the target.  Allowed only for a shared library.  */
  std::string GetSOName(const std::string& config) const;

  void GetFullNameComponents(std::string& prefix, std::string& base,
                             std::string& suffix, const std::string& config,
                             cmStateEnums::ArtifactType artifact =
                               cmStateEnums::RuntimeBinaryArtifact) const;

  /** Append to @a base the bundle directory hierarchy up to a certain @a level
   * and return it. */
  std::string BuildBundleDirectory(const std::string& base,
                                   const std::string& config,
                                   BundleDirectoryLevel level) const;

  /** @return the mac content directory for this target. */
  std::string GetMacContentDirectory(
    const std::string& config, cmStateEnums::ArtifactType artifact) const;

  /** @return folder prefix for IDEs. */
  std::string GetEffectiveFolderName() const;

  cmTarget* Target;
  cmMakefile* Makefile;
  cmLocalGenerator* LocalGenerator;
  cmGlobalGenerator const* GlobalGenerator;

  struct ModuleDefinitionInfo
  {
    std::string DefFile;
    bool DefFileGenerated;
    bool WindowsExportAllSymbols;
    std::vector<cmSourceFile const*> Sources;
  };
  ModuleDefinitionInfo const* GetModuleDefinitionInfo(
    std::string const& config) const;

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

  LinkClosure const* GetLinkClosure(const std::string& config) const;

  cmLinkImplementation const* GetLinkImplementation(
    const std::string& config, LinkInterfaceFor implFor) const;

  void ComputeLinkImplementationLanguages(
    const std::string& config, cmOptionalLinkImplementation& impl) const;

  cmLinkImplementationLibraries const* GetLinkImplementationLibraries(
    const std::string& config, LinkInterfaceFor implFor) const;

  void ComputeLinkImplementationLibraries(const std::string& config,
                                          cmOptionalLinkImplementation& impl,
                                          const cmGeneratorTarget* head,
                                          LinkInterfaceFor implFor) const;

  struct TargetOrString
  {
    std::string String;
    cmGeneratorTarget* Target = nullptr;
  };
  TargetOrString ResolveTargetReference(std::string const& name) const;
  TargetOrString ResolveTargetReference(std::string const& name,
                                        cmLocalGenerator const* lg) const;

  cmLinkItem ResolveLinkItem(BT<std::string> const& name) const;
  cmLinkItem ResolveLinkItem(BT<std::string> const& name,
                             cmLocalGenerator const* lg) const;

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

  bool IsCSharpOnly() const;

  bool IsDotNetSdkTarget() const;

  void GetObjectLibrariesCMP0026(
    std::vector<cmGeneratorTarget*>& objlibs) const;

  std::string GetFullNameImported(const std::string& config,
                                  cmStateEnums::ArtifactType artifact) const;

  /** Get source files common to all configurations and diagnose cases
      with per-config sources.  Excludes sources added by a TARGET_OBJECTS
      generator expression.  Do not use outside the Xcode generator.  */
  bool GetConfigCommonSourceFilesForXcode(
    std::vector<cmSourceFile*>& files) const;

  bool HaveBuildTreeRPATH(const std::string& config) const;

  /** Full path with trailing slash to the top-level directory
      holding object files for this target.  Includes the build
      time config name placeholder if needed for the generator.  */
  std::string ObjectDirectory;

  /** Full path with trailing slash to the top-level directory
      holding object files for the given configuration.  */
  std::string GetObjectDirectory(std::string const& config) const;

  void GetAppleArchs(const std::string& config,
                     std::vector<std::string>& archVec) const;

  void AddExplicitLanguageFlags(std::string& flags,
                                cmSourceFile const& sf) const;

  void AddCUDAArchitectureFlags(std::string& flags) const;
  void AddCUDAToolkitFlags(std::string& flags) const;

  void AddHIPArchitectureFlags(std::string& flags) const;

  void AddISPCTargetFlags(std::string& flags) const;

  std::string GetFeatureSpecificLinkRuleVariable(
    std::string const& var, std::string const& lang,
    std::string const& config) const;

  /** Return the rule variable used to create this type of target.  */
  std::string GetCreateRuleVariable(std::string const& lang,
                                    std::string const& config) const;

  /** Get the include directories for this target.  */
  std::vector<BT<std::string>> GetIncludeDirectories(
    const std::string& config, const std::string& lang) const;

  void GetCompileOptions(std::vector<std::string>& result,
                         const std::string& config,
                         const std::string& language) const;
  std::vector<BT<std::string>> GetCompileOptions(
    std::string const& config, std::string const& language) const;

  void GetCompileFeatures(std::vector<std::string>& features,
                          const std::string& config) const;
  std::vector<BT<std::string>> GetCompileFeatures(
    std::string const& config) const;

  void GetCompileDefinitions(std::vector<std::string>& result,
                             const std::string& config,
                             const std::string& language) const;
  std::vector<BT<std::string>> GetCompileDefinitions(
    std::string const& config, std::string const& language) const;

  void GetLinkOptions(std::vector<std::string>& result,
                      const std::string& config,
                      const std::string& language) const;
  std::vector<BT<std::string>> GetLinkOptions(
    std::string const& config, std::string const& language) const;

  std::vector<BT<std::string>>& ResolveLinkerWrapper(
    std::vector<BT<std::string>>& result, const std::string& language,
    bool joinItems = false) const;

  void GetStaticLibraryLinkOptions(std::vector<std::string>& result,
                                   const std::string& config,
                                   const std::string& language) const;
  std::vector<BT<std::string>> GetStaticLibraryLinkOptions(
    std::string const& config, std::string const& language) const;

  void GetLinkDirectories(std::vector<std::string>& result,
                          const std::string& config,
                          const std::string& language) const;
  std::vector<BT<std::string>> GetLinkDirectories(
    std::string const& config, std::string const& language) const;

  void GetLinkDepends(std::vector<std::string>& result,
                      const std::string& config,
                      const std::string& language) const;
  std::vector<BT<std::string>> GetLinkDepends(
    std::string const& config, std::string const& language) const;

  std::vector<BT<std::string>> GetPrecompileHeaders(
    const std::string& config, const std::string& language) const;

  std::string GetPchHeader(const std::string& config,
                           const std::string& language,
                           const std::string& arch = std::string()) const;
  std::string GetPchSource(const std::string& config,
                           const std::string& language,
                           const std::string& arch = std::string()) const;
  std::string GetPchFileObject(const std::string& config,
                               const std::string& language,
                               const std::string& arch = std::string());
  std::string GetPchFile(const std::string& config,
                         const std::string& language,
                         const std::string& arch = std::string());
  std::string GetPchCreateCompileOptions(
    const std::string& config, const std::string& language,
    const std::string& arch = std::string());
  std::string GetPchUseCompileOptions(const std::string& config,
                                      const std::string& language,
                                      const std::string& arch = std::string());

  void AddSourceFileToUnityBatch(const std::string& sourceFilename);
  bool IsSourceFilePartOfUnityBatch(const std::string& sourceFilename) const;

  bool IsSystemIncludeDirectory(const std::string& dir,
                                const std::string& config,
                                const std::string& language) const;

  /** Add the target output files to the global generator manifest.  */
  void ComputeTargetManifest(const std::string& config) const;

  bool ComputeCompileFeatures(std::string const& config) const;

  using LanguagePair = std::pair<std::string, std::string>;
  bool ComputeCompileFeatures(
    std::string const& config,
    std::set<LanguagePair> const& languagePairs) const;

  /**
   * Trace through the source files in this target and add al source files
   * that they depend on, used by all generators
   */
  void TraceDependencies();

  /** Get the directory in which this target will be built.  If the
      configuration name is given then the generator will add its
      subdirectory for that configuration.  Otherwise just the canonical
      output directory is given.  */
  std::string GetDirectory(const std::string& config,
                           cmStateEnums::ArtifactType artifact =
                             cmStateEnums::RuntimeBinaryArtifact) const;

  /** Get the directory in which to place the target compiler .pdb file.
      If the configuration name is given then the generator will add its
      subdirectory for that configuration.  Otherwise just the canonical
      compiler pdb output directory is given.  */
  std::string GetCompilePDBDirectory(const std::string& config) const;

  /** Get sources that must be built before the given source.  */
  std::vector<cmSourceFile*> const* GetSourceDepends(
    cmSourceFile const* sf) const;

  /** Return whether this target uses the default value for its output
      directory.  */
  bool UsesDefaultOutputDir(const std::string& config,
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

  OutputInfo const* GetOutputInfo(const std::string& config) const;

  // Get the target PDB base name.
  std::string GetPDBOutputName(const std::string& config) const;

  /** Get the name of the pdb file for the target.  */
  std::string GetPDBName(const std::string& config) const;

  /** Whether this library has soname enabled and platform supports it.  */
  bool HasSOName(const std::string& config) const;

  struct CompileInfo
  {
    std::string CompilePdbDir;
  };

  CompileInfo const* GetCompileInfo(const std::string& config) const;

  using CompileInfoMapType = std::map<std::string, CompileInfo>;
  mutable CompileInfoMapType CompileInfoMap;

  bool IsNullImpliedByLinkLibraries(const std::string& p) const;

  /** Get the name of the compiler pdb file for the target.  */
  std::string GetCompilePDBName(const std::string& config) const;

  /** Get the path for the MSVC /Fd option for this target.  */
  std::string GetCompilePDBPath(const std::string& config) const;

  // Get the target base name.
  std::string GetOutputName(const std::string& config,
                            cmStateEnums::ArtifactType artifact) const;

  /** Get target file prefix */
  std::string GetFilePrefix(const std::string& config,
                            cmStateEnums::ArtifactType artifact =
                              cmStateEnums::RuntimeBinaryArtifact) const;
  /** Get target file prefix */
  std::string GetFileSuffix(const std::string& config,
                            cmStateEnums::ArtifactType artifact =
                              cmStateEnums::RuntimeBinaryArtifact) const;

  /** Get target file postfix */
  std::string GetFilePostfix(const std::string& config) const;

  /** Get framework multi-config-specific postfix */
  std::string GetFrameworkMultiConfigPostfix(const std::string& config) const;

  /** Clears cached meta data for local and external source files.
   * The meta data will be recomputed on demand.
   */
  void ClearSourcesCache();

  // Do not use.  This is only for a specific call site with a FIXME comment.
  void ClearLinkInterfaceCache();

  void AddSource(const std::string& src, bool before = false);
  void AddTracedSources(std::vector<std::string> const& srcs);

  /**
   * Adds an entry to the INCLUDE_DIRECTORIES list.
   * If before is true the entry is pushed at the front.
   */
  void AddIncludeDirectory(const std::string& src, bool before = false);

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
    const char* MacFolder = nullptr; // location inside Mac content folders
  };
  void GetAutoUicOptions(std::vector<std::string>& result,
                         const std::string& config) const;

  struct Names
  {
    std::string Base;
    std::string Output;
    std::string Real;
    std::string ImportLibrary;
    std::string PDB;
    std::string SharedObject;
  };

  /** Get the names of the executable needed to generate a build rule
      that takes into account executable version numbers.  This should
      be called only on an executable target.  */
  Names GetExecutableNames(const std::string& config) const;

  /** Get the names of the library needed to generate a build rule
      that takes into account shared library version numbers.  This
      should be called only on a library target.  */
  Names GetLibraryNames(const std::string& config) const;

  /**
   * Compute whether this target must be relinked before installing.
   */
  bool NeedRelinkBeforeInstall(const std::string& config) const;

  /** Return true if builtin chrpath will work for this target */
  bool IsChrpathUsed(const std::string& config) const;

  /** Get the directory in which this targets .pdb files will be placed.
      If the configuration name is given then the generator will add its
      subdirectory for that configuration.  Otherwise just the canonical
      pdb output directory is given.  */
  std::string GetPDBDirectory(const std::string& config) const;

  //! Return the preferred linker language for this target
  std::string GetLinkerLanguage(const std::string& config) const;

  /** Does this target have a GNU implib to convert to MS format?  */
  bool HasImplibGNUtoMS(std::string const& config) const;

  /** Convert the given GNU import library name (.dll.a) to a name with a new
      extension (.lib or ${CMAKE_IMPORT_LIBRARY_SUFFIX}).  */
  bool GetImplibGNUtoMS(std::string const& config, std::string const& gnuName,
                        std::string& out, const char* newExt = nullptr) const;

  /** Can only ever return true if GetSourceFilePaths() was called before.
      Otherwise, this is indeterminate and false will be assumed/returned!  */
  bool HasContextDependentSources() const;

  bool IsExecutableWithExports() const;

  /** Return whether or not the target has a DLL import library.  */
  bool HasImportLibrary(std::string const& config) const;

  /** Get a build-tree directory in which to place target support files.  */
  std::string GetSupportDirectory() const;

  /** Return whether this target may be used to link another target.  */
  bool IsLinkable() const;

  /** Return whether this target is a shared library Framework on
      Apple.  */
  bool IsFrameworkOnApple() const;

  /** Return whether this target is an executable Bundle on Apple.  */
  bool IsAppBundleOnApple() const;

  /** Return whether this target is a XCTest on Apple.  */
  bool IsXCTestOnApple() const;

  /** Return whether this target is a CFBundle (plugin) on Apple.  */
  bool IsCFBundleOnApple() const;

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
  ManagedType GetManagedType(const std::string& config) const;

  struct SourceFileFlags GetTargetSourceFileFlags(
    const cmSourceFile* sf) const;

  void ReportPropertyOrigin(const std::string& p, const std::string& result,
                            const std::string& report,
                            const std::string& compatibilityType) const;

  class TargetPropertyEntry;

  std::string EvaluateInterfaceProperty(
    std::string const& prop, cmGeneratorExpressionContext* context,
    cmGeneratorExpressionDAGChecker* dagCheckerParent,
    LinkInterfaceFor interfaceFor = LinkInterfaceFor::Usage) const;

  bool HaveInstallTreeRPATH(const std::string& config) const;

  bool GetBuildRPATH(const std::string& config, std::string& rpath) const;
  bool GetInstallRPATH(const std::string& config, std::string& rpath) const;

  /** Whether this library has \@rpath and platform supports it.  */
  bool HasMacOSXRpathInstallNameDir(const std::string& config) const;

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
  bool IsImportedSharedLibWithoutSOName(const std::string& config) const;

  std::string ImportedGetLocation(const std::string& config) const;

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
  void GetTargetVersionFallback(const std::string& property,
                                const std::string& fallback_property,
                                int& major, int& minor, int& patch) const;

  std::string GetRuntimeLinkLibrary(std::string const& lang,
                                    std::string const& config) const;

  std::string GetFortranModuleDirectory(std::string const& working_dir) const;
  bool IsFortranBuildingInstrinsicModules() const;

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

  bool AddHeaderSetVerification();
  std::string GenerateHeaderSetVerificationFile(
    cmSourceFile& source, const std::string& dir,
    cm::optional<std::set<std::string>>& languages) const;

private:
  void AddSourceCommon(const std::string& src, bool before = false);

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
  bool DetermineHasMacOSXRpathInstallNameDir(const std::string& config) const;

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
                                const std::string& language = "") const;
  cmValue GetFileSuffixInternal(std::string const& config,
                                cmStateEnums::ArtifactType artifact,
                                const std::string& language = "") const;

  std::string GetFullNameInternal(const std::string& config,
                                  cmStateEnums::ArtifactType artifact) const;
  void GetFullNameInternal(const std::string& config,
                           cmStateEnums::ArtifactType artifact,
                           std::string& outPrefix, std::string& outBase,
                           std::string& outSuffix) const;

  mutable std::string LinkerLanguage;
  using LinkClosureMapType = std::map<std::string, LinkClosure>;
  mutable LinkClosureMapType LinkClosureMap;
  bool DeviceLink = false;

  // Returns ARCHIVE, LIBRARY, or RUNTIME based on platform and type.
  const char* GetOutputTargetType(cmStateEnums::ArtifactType artifact) const;

  void ComputeVersionedName(std::string& vName, std::string const& prefix,
                            std::string const& base, std::string const& suffix,
                            std::string const& name, cmValue version) const;

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
                                  const std::string& config) const;

  void ComputeLinkClosure(const std::string& config, LinkClosure& lc) const;
  bool ComputeLinkClosure(const std::string& config, LinkClosure& lc,
                          bool secondPass) const;

  struct LinkImplClosure : public std::vector<cmGeneratorTarget const*>
  {
    bool Done = false;
  };
  mutable std::map<std::string, LinkImplClosure> LinkImplClosureMap;

  using LinkInterfaceMapType = std::map<std::string, cmHeadToLinkInterfaceMap>;
  mutable LinkInterfaceMapType LinkInterfaceMap;
  mutable LinkInterfaceMapType LinkInterfaceUsageRequirementsOnlyMap;

  cmHeadToLinkInterfaceMap& GetHeadToLinkInterfaceMap(
    std::string const& config) const;
  cmHeadToLinkInterfaceMap& GetHeadToLinkInterfaceUsageRequirementsMap(
    std::string const& config) const;

  std::string GetLinkInterfaceDependentStringAsBoolProperty(
    const std::string& p, const std::string& config) const;

  friend class cmTargetCollectLinkLanguages;
  cmLinkInterface const* GetLinkInterface(const std::string& config,
                                          const cmGeneratorTarget* headTarget,
                                          bool secondPass) const;
  void ComputeLinkInterface(const std::string& config,
                            cmOptionalLinkInterface& iface,
                            const cmGeneratorTarget* head,
                            bool secondPass) const;
  cmLinkImplementation const* GetLinkImplementation(const std::string& config,
                                                    LinkInterfaceFor implFor,
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
  ImportInfo const* GetImportInfo(const std::string& config) const;

  /** Strip off leading and trailing whitespace from an item named in
      the link dependencies of this target.  */
  std::string CheckCMP0004(std::string const& item) const;

  cmLinkInterface const* GetImportLinkInterface(const std::string& config,
                                                const cmGeneratorTarget* head,
                                                LinkInterfaceFor interfaceFor,
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
                                  LinkInterfaceFor interfaceFor) const;

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
                       const cmGeneratorTarget* headTarget,
                       LinkInterfaceFor interfaceFor, LinkInterfaceField field,
                       cmLinkInterface& iface) const;

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
                                          LookupLinkItemScope* scope,
                                          LookupSelf lookupSelf) const;

  std::vector<BT<std::string>> GetSourceFilePaths(
    std::string const& config) const;
  std::vector<BT<cmSourceFile*>> GetSourceFilesWithoutObjectLibraries(
    std::string const& config) const;
  void GetSourceFilesWithoutObjectLibraries(std::vector<cmSourceFile*>& files,
                                            const std::string& config) const;

  struct HeadToLinkImplementationMap
    : public std::map<cmGeneratorTarget const*, cmOptionalLinkImplementation>
  {
  };
  using LinkImplMapType = std::map<std::string, HeadToLinkImplementationMap>;
  mutable LinkImplMapType LinkImplMap;
  mutable LinkImplMapType LinkImplUsageRequirementsOnlyMap;

  HeadToLinkImplementationMap& GetHeadToLinkImplementationMap(
    std::string const& config) const;
  HeadToLinkImplementationMap& GetHeadToLinkImplementationUsageRequirementsMap(
    std::string const& config) const;

  cmLinkImplementationLibraries const* GetLinkImplementationLibrariesInternal(
    const std::string& config, const cmGeneratorTarget* head,
    LinkInterfaceFor implFor) const;
  bool ComputeOutputDir(const std::string& config,
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
  mutable bool PolicyWarnedCMP0022 = false;
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

  bool ComputePDBOutputDir(const std::string& kind, const std::string& config,
                           std::string& out) const;

  ManagedType CheckManagedType(std::string const& propval) const;

  bool GetRPATH(const std::string& config, const std::string& prop,
                std::string& rpath) const;

  mutable std::map<std::string, BTs<std::string>> LanguageStandardMap;

  cmValue GetPropertyWithPairedLanguageSupport(std::string const& lang,
                                               const char* suffix) const;

  void ComputeLinkImplementationRuntimeLibraries(
    const std::string& config, cmOptionalLinkImplementation& impl) const;

  void ComputeLinkInterfaceRuntimeLibraries(
    const std::string& config, cmOptionalLinkInterface& iface) const;

public:
  const std::vector<const cmGeneratorTarget*>& GetLinkImplementationClosure(
    const std::string& config) const;

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
};
