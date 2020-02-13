/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmTarget_h
#define cmTarget_h

#include "cmConfigure.h" // IWYU pragma: keep

#include <iosfwd>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "cmAlgorithms.h"
#include "cmListFileCache.h"
#include "cmPolicies.h"
#include "cmStateTypes.h"
#include "cmStringAlgorithms.h"
#include "cmTargetLinkLibraryType.h"

class cmCustomCommand;
class cmGlobalGenerator;
class cmInstallTargetGenerator;
class cmMakefile;
class cmMessenger;
class cmPropertyMap;
class cmSourceFile;
class cmTargetInternals;

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

  cmTarget(std::string const& name, cmStateEnums::TargetType type,
           Visibility vis, cmMakefile* mf, bool perConfig);

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
  void ClearDependencyInformation(cmMakefile& mf);

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
  //! Get the utilities used by this target
  std::set<BT<std::pair<std::string, bool>>> const& GetUtilities() const;

  //! Set/Get a property of this target file
  void SetProperty(const std::string& prop, const char* value);
  void SetProperty(const std::string& prop, const std::string& value)
  {
    SetProperty(prop, value.c_str());
  }
  void AppendProperty(const std::string& prop, const std::string& value,
                      bool asString = false);
  //! Might return a nullptr if the property is not set or invalid
  const char* GetProperty(const std::string& prop) const;
  //! Always returns a valid pointer
  const char* GetSafeProperty(const std::string& prop) const;
  bool GetPropertyAsBool(const std::string& prop) const;
  void CheckProperty(const std::string& prop, cmMakefile* context) const;
  const char* GetComputedProperty(const std::string& prop,
                                  cmMessenger* messenger,
                                  cmListFileBacktrace const& context) const;
  //! Get all properties
  cmPropertyMap const& GetProperties() const;

  //! Return whether or not the target is for a DLL platform.
  bool IsDLLPlatform() const;

  //! Return whether or not we are targeting AIX.
  bool IsAIX() const;

  bool IsImported() const;
  bool IsImportedGloballyVisible() const;
  bool IsPerConfig() const;

  bool GetMappedConfig(std::string const& desired_config, const char** loc,
                       const char** imp, std::string& suffix) const;

  //! Return whether this target is an executable with symbol exports enabled.
  bool IsExecutableWithExports() const;

  //! Return whether this target is a shared library Framework on Apple.
  bool IsFrameworkOnApple() const;

  //! Return whether this target is an executable Bundle on Apple.
  bool IsAppBundleOnApple() const;

  //! Get a backtrace from the creation of the target.
  cmListFileBacktrace const& GetBacktrace() const;

  void InsertInclude(std::string const& entry, cmListFileBacktrace const& bt,
                     bool before = false);
  void InsertCompileOption(std::string const& entry,
                           cmListFileBacktrace const& bt, bool before = false);
  void InsertCompileDefinition(std::string const& entry,
                               cmListFileBacktrace const& bt);
  void InsertLinkOption(std::string const& entry,
                        cmListFileBacktrace const& bt, bool before = false);
  void InsertLinkDirectory(std::string const& entry,
                           cmListFileBacktrace const& bt, bool before = false);
  void InsertPrecompileHeader(std::string const& entry,
                              cmListFileBacktrace const& bt);

  void AppendBuildInterfaceIncludes();

  std::string GetDebugGeneratorExpressions(const std::string& value,
                                           cmTargetLinkLibraryType llt) const;

  void AddSystemIncludeDirectories(std::set<std::string> const& incs);
  std::set<std::string> const& GetSystemIncludeDirectories() const;

  cmStringRange GetIncludeDirectoriesEntries() const;
  cmBacktraceRange GetIncludeDirectoriesBacktraces() const;

  cmStringRange GetCompileOptionsEntries() const;
  cmBacktraceRange GetCompileOptionsBacktraces() const;

  cmStringRange GetCompileFeaturesEntries() const;
  cmBacktraceRange GetCompileFeaturesBacktraces() const;

  cmStringRange GetCompileDefinitionsEntries() const;
  cmBacktraceRange GetCompileDefinitionsBacktraces() const;

  cmStringRange GetPrecompileHeadersEntries() const;
  cmBacktraceRange GetPrecompileHeadersBacktraces() const;

  cmStringRange GetSourceEntries() const;
  cmBacktraceRange GetSourceBacktraces() const;

  cmStringRange GetLinkOptionsEntries() const;
  cmBacktraceRange GetLinkOptionsBacktraces() const;

  cmStringRange GetLinkDirectoriesEntries() const;
  cmBacktraceRange GetLinkDirectoriesBacktraces() const;

  cmStringRange GetLinkImplementationEntries() const;
  cmBacktraceRange GetLinkImplementationBacktraces() const;

  std::string ImportedGetFullPath(const std::string& config,
                                  cmStateEnums::ArtifactType artifact) const;

  struct StrictTargetComparison
  {
    bool operator()(cmTarget const* t1, cmTarget const* t2) const;
  };

private:
  // Internal representation details.
  friend class cmGeneratorTarget;

  const char* GetSuffixVariableInternal(
    cmStateEnums::ArtifactType artifact) const;
  const char* GetPrefixVariableInternal(
    cmStateEnums::ArtifactType artifact) const;

private:
  std::unique_ptr<cmTargetInternals> impl;
};

#endif
