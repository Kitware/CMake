/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <iosfwd>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include <cm/optional>

#include "cmAlgorithms.h"
#include "cmFileSet.h"
#include "cmPolicies.h"
#include "cmStateTypes.h"
#include "cmStringAlgorithms.h"
#include "cmTargetLinkLibraryType.h"
#include "cmValue.h"

class cmCustomCommand;
class cmGlobalGenerator;
class cmInstallTargetGenerator;
class cmListFileBacktrace;
class cmListFileContext;
class cmMakefile;
class cmPropertyMap;
class cmSourceFile;
class cmTargetExport;
class cmTargetInternals;

template <typename T>
class BT;
template <typename T>
class BTs;

/** \class cmTarget
 * \brief Represent a library or executable target loaded from a makefile.
 *
 * cmTarget represents a target loaded from a makefile.
 */
class cmTarget
{
public:
  enum Visibility
  {
    VisibilityNormal,
    VisibilityImported,
    VisibilityImportedGlobally
  };

  enum class PerConfig
  {
    Yes,
    No
  };

  cmTarget(std::string const& name, cmStateEnums::TargetType type,
           Visibility vis, cmMakefile* mf, PerConfig perConfig);

  cmTarget(cmTarget const&) = delete;
  cmTarget(cmTarget&&) noexcept;
  ~cmTarget();

  cmTarget& operator=(cmTarget const&) = delete;
  cmTarget& operator=(cmTarget&&) noexcept;

  //! Return the type of target.
  cmStateEnums::TargetType GetType() const;

  //! Get the cmMakefile that owns this target.
  cmMakefile* GetMakefile() const;

  //! Return the global generator.
  cmGlobalGenerator* GetGlobalGenerator() const;

  //! Set/Get the name of the target
  const std::string& GetName() const;

  //! Get the policy map
  cmPolicies::PolicyMap const& GetPolicyMap() const;

  //! Get policy status
  cmPolicies::PolicyStatus GetPolicyStatus(cmPolicies::PolicyID policy) const;

#define DECLARE_TARGET_POLICY(POLICY)                                         \
  cmPolicies::PolicyStatus GetPolicyStatus##POLICY() const                    \
  {                                                                           \
    return this->GetPolicyStatus(cmPolicies::POLICY);                         \
  }

  CM_FOR_EACH_TARGET_POLICY(DECLARE_TARGET_POLICY)

#undef DECLARE_TARGET_POLICY

  //! Get the list of the PRE_BUILD custom commands for this target
  std::vector<cmCustomCommand> const& GetPreBuildCommands() const;
  void AddPreBuildCommand(cmCustomCommand const& cmd);
  void AddPreBuildCommand(cmCustomCommand&& cmd);

  //! Get the list of the PRE_LINK custom commands for this target
  std::vector<cmCustomCommand> const& GetPreLinkCommands() const;
  void AddPreLinkCommand(cmCustomCommand const& cmd);
  void AddPreLinkCommand(cmCustomCommand&& cmd);

  //! Get the list of the POST_BUILD custom commands for this target
  std::vector<cmCustomCommand> const& GetPostBuildCommands() const;
  void AddPostBuildCommand(cmCustomCommand const& cmd);
  void AddPostBuildCommand(cmCustomCommand&& cmd);

  //! Add sources to the target.
  void AddSources(std::vector<std::string> const& srcs);
  void AddTracedSources(std::vector<std::string> const& srcs);
  std::string GetSourceCMP0049(const std::string& src);
  cmSourceFile* AddSource(const std::string& src, bool before = false);

  //! how we identify a library, by name and type
  using LibraryID = std::pair<std::string, cmTargetLinkLibraryType>;
  using LinkLibraryVectorType = std::vector<LibraryID>;
  LinkLibraryVectorType const& GetOriginalLinkLibraries() const;

  //! Clear the dependency information recorded for this target, if any.
  void ClearDependencyInformation(cmMakefile& mf) const;

  void AddLinkLibrary(cmMakefile& mf, std::string const& lib,
                      cmTargetLinkLibraryType llt);

  enum TLLSignature
  {
    KeywordTLLSignature,
    PlainTLLSignature
  };
  bool PushTLLCommandTrace(TLLSignature signature,
                           cmListFileContext const& lfc);
  void GetTllSignatureTraces(std::ostream& s, TLLSignature sig) const;

  /**
   * Set the path where this target should be installed. This is relative to
   * INSTALL_PREFIX
   */
  std::string const& GetInstallPath() const;
  void SetInstallPath(std::string const& name);

  /**
   * Set the path where this target (if it has a runtime part) should be
   * installed. This is relative to INSTALL_PREFIX
   */
  std::string const& GetRuntimeInstallPath() const;
  void SetRuntimeInstallPath(std::string const& name);

  /**
   * Get/Set whether there is an install rule for this target.
   */
  bool GetHaveInstallRule() const;
  void SetHaveInstallRule(bool hir);

  void AddInstallGenerator(cmInstallTargetGenerator* g);
  std::vector<cmInstallTargetGenerator*> const& GetInstallGenerators() const;

  /**
   * Get/Set whether this target was auto-created by a generator.
   */
  bool GetIsGeneratorProvided() const;
  void SetIsGeneratorProvided(bool igp);

  /**
   * Add a utility on which this project depends. A utility is an executable
   * name as would be specified to the ADD_EXECUTABLE or UTILITY_SOURCE
   * commands. It is not a full path nor does it have an extension.
   */
  void AddUtility(std::string const& name, bool cross,
                  cmMakefile* mf = nullptr);
  void AddUtility(BT<std::pair<std::string, bool>> util);
  //! Get the utilities used by this target
  std::set<BT<std::pair<std::string, bool>>> const& GetUtilities() const;

  //! Set/Get a property of this target file
  void SetProperty(const std::string& prop, const char* value);
  void SetProperty(const std::string& prop, cmValue value);
  void SetProperty(const std::string& prop, const std::string& value)
  {
    this->SetProperty(prop, cmValue(value));
  }
  void AppendProperty(const std::string& prop, const std::string& value,
                      bool asString = false);
  //! Might return a nullptr if the property is not set or invalid
  cmValue GetProperty(const std::string& prop) const;
  //! Always returns a valid pointer
  std::string const& GetSafeProperty(std::string const& prop) const;
  bool GetPropertyAsBool(const std::string& prop) const;
  void CheckProperty(const std::string& prop, cmMakefile* context) const;
  cmValue GetComputedProperty(const std::string& prop, cmMakefile& mf) const;
  //! Get all properties
  cmPropertyMap const& GetProperties() const;

  //! Return whether or not the target is for a DLL platform.
  bool IsDLLPlatform() const;

  //! Return whether or not we are targeting AIX.
  bool IsAIX() const;

  bool IsImported() const;
  bool IsImportedGloballyVisible() const;
  bool IsPerConfig() const;
  bool CanCompileSources() const;

  bool GetMappedConfig(std::string const& desired_config, cmValue& loc,
                       cmValue& imp, std::string& suffix) const;

  //! Return whether this target is an executable with symbol exports enabled.
  bool IsExecutableWithExports() const;

  //! Return whether this target is a shared library Framework on Apple.
  bool IsFrameworkOnApple() const;

  //! Return whether this target is an executable Bundle on Apple.
  bool IsAppBundleOnApple() const;

  //! Return whether this target is a GUI executable on Android.
  bool IsAndroidGuiExecutable() const;

  bool HasKnownObjectFileLocation(std::string* reason = nullptr) const;

  //! Get a backtrace from the creation of the target.
  cmListFileBacktrace const& GetBacktrace() const;

  void InsertInclude(BT<std::string> const& entry, bool before = false);
  void InsertCompileOption(BT<std::string> const& entry, bool before = false);
  void InsertCompileDefinition(BT<std::string> const& entry);
  void InsertLinkOption(BT<std::string> const& entry, bool before = false);
  void InsertLinkDirectory(BT<std::string> const& entry, bool before = false);
  void InsertPrecompileHeader(BT<std::string> const& entry);

  void AppendBuildInterfaceIncludes();
  void FinalizeTargetCompileInfo(
    const cmBTStringRange& noConfigCompileDefinitions,
    cm::optional<std::map<std::string, cmValue>>& perConfigCompileDefinitions);

  std::string GetDebugGeneratorExpressions(const std::string& value,
                                           cmTargetLinkLibraryType llt) const;

  void AddSystemIncludeDirectories(std::set<std::string> const& incs);
  std::set<std::string> const& GetSystemIncludeDirectories() const;

  void AddInstallIncludeDirectories(cmTargetExport const& te,
                                    cmStringRange const& incs);
  cmStringRange GetInstallIncludeDirectoriesEntries(
    cmTargetExport const& te) const;

  BTs<std::string> const* GetLanguageStandardProperty(
    const std::string& propertyName) const;

  void SetLanguageStandardProperty(std::string const& lang,
                                   std::string const& value,
                                   const std::string& feature);

  cmBTStringRange GetIncludeDirectoriesEntries() const;

  cmBTStringRange GetCompileOptionsEntries() const;

  cmBTStringRange GetCompileFeaturesEntries() const;

  cmBTStringRange GetCompileDefinitionsEntries() const;

  cmBTStringRange GetPrecompileHeadersEntries() const;

  cmBTStringRange GetSourceEntries() const;

  cmBTStringRange GetLinkOptionsEntries() const;

  cmBTStringRange GetLinkDirectoriesEntries() const;

  cmBTStringRange GetLinkImplementationEntries() const;

  cmBTStringRange GetLinkInterfaceEntries() const;
  cmBTStringRange GetLinkInterfaceDirectEntries() const;
  cmBTStringRange GetLinkInterfaceDirectExcludeEntries() const;

  cmBTStringRange GetHeaderSetsEntries() const;

  cmBTStringRange GetInterfaceHeaderSetsEntries() const;

  std::string ImportedGetFullPath(const std::string& config,
                                  cmStateEnums::ArtifactType artifact) const;

  struct StrictTargetComparison
  {
    bool operator()(cmTarget const* t1, cmTarget const* t2) const;
  };

  const cmFileSet* GetFileSet(const std::string& name) const;
  cmFileSet* GetFileSet(const std::string& name);
  std::pair<cmFileSet*, bool> GetOrCreateFileSet(const std::string& name,
                                                 const std::string& type,
                                                 cmFileSetVisibility vis);

  std::vector<std::string> GetAllFileSetNames() const;
  std::vector<std::string> GetAllInterfaceFileSets() const;

  static std::string GetFileSetsPropertyName(const std::string& type);
  static std::string GetInterfaceFileSetsPropertyName(const std::string& type);

private:
  template <typename ValueType>
  void StoreProperty(const std::string& prop, ValueType value);

  // Internal representation details.
  friend class cmGeneratorTarget;

  const char* GetSuffixVariableInternal(
    cmStateEnums::ArtifactType artifact) const;
  const char* GetPrefixVariableInternal(
    cmStateEnums::ArtifactType artifact) const;

  std::unique_ptr<cmTargetInternals> impl;
};
