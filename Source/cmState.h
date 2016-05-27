/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2015 Stephen Kelly <steveire@gmail.com>

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmState_h
#define cmState_h

#include "cmStandardIncludes.h"

#include "cmAlgorithms.h"
#include "cmLinkedTree.h"
#include "cmPolicies.h"
#include "cmPropertyDefinitionMap.h"
#include "cmPropertyMap.h"

class cmake;
class cmCommand;
class cmDefinitions;
class cmListFileBacktrace;
class cmCacheManager;

class cmState
{
  struct SnapshotDataType;
  struct PolicyStackEntry;
  struct BuildsystemDirectoryStateType;
  typedef cmLinkedTree<SnapshotDataType>::iterator PositionType;
  friend class Snapshot;

public:
  cmState();
  ~cmState();

  enum SnapshotType
  {
    BaseType,
    BuildsystemDirectoryType,
    FunctionCallType,
    MacroCallType,
    IncludeFileType,
    InlineListFileType,
    PolicyScopeType,
    VariableScopeType
  };

  class Directory;

  class Snapshot
  {
  public:
    Snapshot(cmState* state = 0);
    Snapshot(cmState* state, PositionType position);

    const char* GetDefinition(std::string const& name) const;
    bool IsInitialized(std::string const& name) const;
    void SetDefinition(std::string const& name, std::string const& value);
    void RemoveDefinition(std::string const& name);
    std::vector<std::string> UnusedKeys() const;
    std::vector<std::string> ClosureKeys() const;
    bool RaiseScope(std::string const& var, const char* varDef);

    void SetListFile(std::string const& listfile);

    std::string GetExecutionListFile() const;

    std::vector<Snapshot> GetChildren();

    bool IsValid() const;
    Snapshot GetBuildsystemDirectoryParent() const;
    Snapshot GetCallStackParent() const;
    Snapshot GetCallStackBottom() const;
    SnapshotType GetType() const;

    void SetPolicy(cmPolicies::PolicyID id, cmPolicies::PolicyStatus status);
    cmPolicies::PolicyStatus GetPolicy(cmPolicies::PolicyID id) const;
    bool HasDefinedPolicyCMP0011();
    void PushPolicy(cmPolicies::PolicyMap entry, bool weak);
    bool PopPolicy();
    bool CanPopPolicyScope();

    cmState* GetState() const;

    Directory GetDirectory() const;

    void SetProjectName(std::string const& name);
    std::string GetProjectName() const;

    void InitializeFromParent_ForSubdirsCommand();

    struct StrictWeakOrder
    {
      bool operator()(const cmState::Snapshot& lhs,
                      const cmState::Snapshot& rhs) const;
    };

    void SetDirectoryDefinitions();
    void SetDefaultDefinitions();

  private:
    friend bool operator==(const cmState::Snapshot& lhs,
                           const cmState::Snapshot& rhs);
    friend bool operator!=(const cmState::Snapshot& lhs,
                           const cmState::Snapshot& rhs);
    friend class cmState;
    friend class Directory;
    friend struct StrictWeakOrder;

    void InitializeFromParent();

    cmState* State;
    cmState::PositionType Position;
  };

  class Directory
  {
    Directory(cmLinkedTree<BuildsystemDirectoryStateType>::iterator iter,
              Snapshot const& snapshot);

  public:
    const char* GetCurrentSource() const;
    void SetCurrentSource(std::string const& dir);
    const char* GetCurrentBinary() const;
    void SetCurrentBinary(std::string const& dir);

    std::vector<std::string> const& GetCurrentSourceComponents() const;
    std::vector<std::string> const& GetCurrentBinaryComponents() const;

    const char* GetRelativePathTopSource() const;
    const char* GetRelativePathTopBinary() const;
    void SetRelativePathTopSource(const char* dir);
    void SetRelativePathTopBinary(const char* dir);

    cmStringRange GetIncludeDirectoriesEntries() const;
    cmBacktraceRange GetIncludeDirectoriesEntryBacktraces() const;
    void AppendIncludeDirectoriesEntry(std::string const& vec,
                                       cmListFileBacktrace const& lfbt);
    void PrependIncludeDirectoriesEntry(std::string const& vec,
                                        cmListFileBacktrace const& lfbt);
    void SetIncludeDirectories(std::string const& vec,
                               cmListFileBacktrace const& lfbt);
    void ClearIncludeDirectories();

    cmStringRange GetCompileDefinitionsEntries() const;
    cmBacktraceRange GetCompileDefinitionsEntryBacktraces() const;
    void AppendCompileDefinitionsEntry(std::string const& vec,
                                       cmListFileBacktrace const& lfbt);
    void SetCompileDefinitions(std::string const& vec,
                               cmListFileBacktrace const& lfbt);
    void ClearCompileDefinitions();

    cmStringRange GetCompileOptionsEntries() const;
    cmBacktraceRange GetCompileOptionsEntryBacktraces() const;
    void AppendCompileOptionsEntry(std::string const& vec,
                                   cmListFileBacktrace const& lfbt);
    void SetCompileOptions(std::string const& vec,
                           cmListFileBacktrace const& lfbt);
    void ClearCompileOptions();

    void SetProperty(const std::string& prop, const char* value,
                     cmListFileBacktrace const& lfbt);
    void AppendProperty(const std::string& prop, const char* value,
                        bool asString, cmListFileBacktrace const& lfbt);
    const char* GetProperty(const std::string& prop) const;
    const char* GetProperty(const std::string& prop, bool chain) const;
    bool GetPropertyAsBool(const std::string& prop) const;
    std::vector<std::string> GetPropertyKeys() const;

  private:
    void ComputeRelativePathTopSource();
    void ComputeRelativePathTopBinary();

  private:
    cmLinkedTree<BuildsystemDirectoryStateType>::iterator DirectoryState;
    Snapshot Snapshot_;
    friend class Snapshot;
  };

  enum TargetType
  {
    EXECUTABLE,
    STATIC_LIBRARY,
    SHARED_LIBRARY,
    MODULE_LIBRARY,
    OBJECT_LIBRARY,
    UTILITY,
    GLOBAL_TARGET,
    INTERFACE_LIBRARY,
    UNKNOWN_LIBRARY
  };

  static const char* GetTargetTypeName(cmState::TargetType targetType);

  Snapshot CreateBaseSnapshot();
  Snapshot CreateBuildsystemDirectorySnapshot(Snapshot originSnapshot);
  Snapshot CreateFunctionCallSnapshot(Snapshot originSnapshot,
                                      std::string const& fileName);
  Snapshot CreateMacroCallSnapshot(Snapshot originSnapshot,
                                   std::string const& fileName);
  Snapshot CreateIncludeFileSnapshot(Snapshot originSnapshot,
                                     std::string const& fileName);
  Snapshot CreateVariableScopeSnapshot(Snapshot originSnapshot);
  Snapshot CreateInlineListFileSnapshot(Snapshot originSnapshot,
                                        std::string const& fileName);
  Snapshot CreatePolicyScopeSnapshot(Snapshot originSnapshot);
  Snapshot Pop(Snapshot originSnapshot);

  enum CacheEntryType
  {
    BOOL = 0,
    PATH,
    FILEPATH,
    STRING,
    INTERNAL,
    STATIC,
    UNINITIALIZED
  };
  static CacheEntryType StringToCacheEntryType(const char*);
  static const char* CacheEntryTypeToString(CacheEntryType);
  static bool IsCacheEntryType(std::string const& key);

  bool LoadCache(const std::string& path, bool internal,
                 std::set<std::string>& excludes,
                 std::set<std::string>& includes);

  bool SaveCache(const std::string& path);

  bool DeleteCache(const std::string& path);

  std::vector<std::string> GetCacheEntryKeys() const;
  const char* GetCacheEntryValue(std::string const& key) const;
  const char* GetInitializedCacheValue(std::string const& key) const;
  CacheEntryType GetCacheEntryType(std::string const& key) const;
  void SetCacheEntryValue(std::string const& key, std::string const& value);
  void SetCacheValue(std::string const& key, std::string const& value);

  void RemoveCacheEntry(std::string const& key);

  void SetCacheEntryProperty(std::string const& key,
                             std::string const& propertyName,
                             std::string const& value);
  void SetCacheEntryBoolProperty(std::string const& key,
                                 std::string const& propertyName, bool value);
  const char* GetCacheEntryProperty(std::string const& key,
                                    std::string const& propertyName);
  bool GetCacheEntryPropertyAsBool(std::string const& key,
                                   std::string const& propertyName);
  void AppendCacheEntryProperty(std::string const& key,
                                const std::string& property,
                                const std::string& value,
                                bool asString = false);
  void RemoveCacheEntryProperty(std::string const& key,
                                std::string const& propertyName);

  ///! Break up a line like VAR:type="value" into var, type and value
  static bool ParseCacheEntry(const std::string& entry, std::string& var,
                              std::string& value, CacheEntryType& type);

  Snapshot Reset();
  // Define a property
  void DefineProperty(const std::string& name, cmProperty::ScopeType scope,
                      const char* ShortDescription,
                      const char* FullDescription, bool chain = false);

  // get property definition
  cmPropertyDefinition const* GetPropertyDefinition(
    const std::string& name, cmProperty::ScopeType scope) const;

  // Is a property defined?
  bool IsPropertyDefined(const std::string& name,
                         cmProperty::ScopeType scope) const;
  bool IsPropertyChained(const std::string& name,
                         cmProperty::ScopeType scope) const;

  void SetLanguageEnabled(std::string const& l);
  bool GetLanguageEnabled(std::string const& l) const;
  std::vector<std::string> GetEnabledLanguages() const;
  void SetEnabledLanguages(std::vector<std::string> const& langs);
  void ClearEnabledLanguages();

  bool GetIsInTryCompile() const;
  void SetIsInTryCompile(bool b);

  cmCommand* GetCommand(std::string const& name) const;
  void AddCommand(cmCommand* command);
  void RemoveUnscriptableCommands();
  void RenameCommand(std::string const& oldName, std::string const& newName);
  void RemoveUserDefinedCommands();
  std::vector<std::string> GetCommandNames() const;

  void SetGlobalProperty(const std::string& prop, const char* value);
  void AppendGlobalProperty(const std::string& prop, const char* value,
                            bool asString = false);
  const char* GetGlobalProperty(const std::string& prop);
  bool GetGlobalPropertyAsBool(const std::string& prop);

  const char* GetSourceDirectory() const;
  void SetSourceDirectory(std::string const& sourceDirectory);
  const char* GetBinaryDirectory() const;
  void SetBinaryDirectory(std::string const& binaryDirectory);

  std::vector<std::string> const& GetSourceDirectoryComponents() const;
  std::vector<std::string> const& GetBinaryDirectoryComponents() const;

  void SetWindowsShell(bool windowsShell);
  bool UseWindowsShell() const;
  void SetWindowsVSIDE(bool windowsVSIDE);
  bool UseWindowsVSIDE() const;
  void SetWatcomWMake(bool watcomWMake);
  bool UseWatcomWMake() const;
  void SetMinGWMake(bool minGWMake);
  bool UseMinGWMake() const;
  void SetNMake(bool nMake);
  bool UseNMake() const;
  void SetMSYSShell(bool mSYSShell);
  bool UseMSYSShell() const;

  unsigned int GetCacheMajorVersion() const;
  unsigned int GetCacheMinorVersion() const;

private:
  friend class cmake;
  void AddCacheEntry(const std::string& key, const char* value,
                     const char* helpString, CacheEntryType type);

  std::map<cmProperty::ScopeType, cmPropertyDefinitionMap> PropertyDefinitions;
  std::vector<std::string> EnabledLanguages;
  std::map<std::string, cmCommand*> Commands;
  cmPropertyMap GlobalProperties;
  cmCacheManager* CacheManager;

  cmLinkedTree<BuildsystemDirectoryStateType> BuildsystemDirectory;

  cmLinkedTree<std::string> ExecutionListFiles;

  cmLinkedTree<PolicyStackEntry> PolicyStack;
  cmLinkedTree<SnapshotDataType> SnapshotData;
  cmLinkedTree<cmDefinitions> VarTree;

  std::vector<std::string> SourceDirectoryComponents;
  std::vector<std::string> BinaryDirectoryComponents;
  std::string SourceDirectory;
  std::string BinaryDirectory;
  bool IsInTryCompile;
  bool WindowsShell;
  bool WindowsVSIDE;
  bool WatcomWMake;
  bool MinGWMake;
  bool NMake;
  bool MSYSShell;
};

bool operator==(const cmState::Snapshot& lhs, const cmState::Snapshot& rhs);
bool operator!=(const cmState::Snapshot& lhs, const cmState::Snapshot& rhs);

#endif
