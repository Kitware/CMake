/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmState_h
#define cmState_h

#include "cmConfigure.h" // IWYU pragma: keep

#include <map>
#include <set>
#include <string>
#include <vector>

#include "cmDefinitions.h"
#include "cmLinkedTree.h"
#include "cmListFileCache.h"
#include "cmPolicies.h"
#include "cmProperty.h"
#include "cmPropertyDefinitionMap.h"
#include "cmPropertyMap.h"
#include "cmStatePrivate.h"
#include "cmStateTypes.h"

class cmCacheManager;
class cmCommand;
class cmGlobVerificationManager;
class cmPropertyDefinition;
class cmStateSnapshot;
class cmMessenger;

class cmState
{
  friend class cmStateSnapshot;

public:
  cmState();
  ~cmState();

  cmState(const cmState&) = delete;
  cmState& operator=(const cmState&) = delete;

  enum Mode
  {
    Unknown,
    Project,
    Script,
    FindPackage,
    CTest,
    CPack,
  };

  static const char* GetTargetTypeName(cmStateEnums::TargetType targetType);

  cmStateSnapshot CreateBaseSnapshot();
  cmStateSnapshot CreateBuildsystemDirectorySnapshot(
    cmStateSnapshot const& originSnapshot);
  cmStateSnapshot CreateFunctionCallSnapshot(
    cmStateSnapshot const& originSnapshot, std::string const& fileName);
  cmStateSnapshot CreateMacroCallSnapshot(
    cmStateSnapshot const& originSnapshot, std::string const& fileName);
  cmStateSnapshot CreateIncludeFileSnapshot(
    cmStateSnapshot const& originSnapshot, std::string const& fileName);
  cmStateSnapshot CreateVariableScopeSnapshot(
    cmStateSnapshot const& originSnapshot);
  cmStateSnapshot CreateInlineListFileSnapshot(
    cmStateSnapshot const& originSnapshot, std::string const& fileName);
  cmStateSnapshot CreatePolicyScopeSnapshot(
    cmStateSnapshot const& originSnapshot);
  cmStateSnapshot Pop(cmStateSnapshot const& originSnapshot);

  static cmStateEnums::CacheEntryType StringToCacheEntryType(const char*);
  static bool StringToCacheEntryType(const char*,
                                     cmStateEnums::CacheEntryType& type);
  static const char* CacheEntryTypeToString(cmStateEnums::CacheEntryType);
  static bool IsCacheEntryType(std::string const& key);

  bool LoadCache(const std::string& path, bool internal,
                 std::set<std::string>& excludes,
                 std::set<std::string>& includes);

  bool SaveCache(const std::string& path, cmMessenger* messenger);

  bool DeleteCache(const std::string& path);

  std::vector<std::string> GetCacheEntryKeys() const;
  const char* GetCacheEntryValue(std::string const& key) const;
  const std::string* GetInitializedCacheValue(std::string const& key) const;
  cmStateEnums::CacheEntryType GetCacheEntryType(std::string const& key) const;
  void SetCacheEntryValue(std::string const& key, std::string const& value);
  void SetCacheValue(std::string const& key, std::string const& value);

  void RemoveCacheEntry(std::string const& key);

  void SetCacheEntryProperty(std::string const& key,
                             std::string const& propertyName,
                             std::string const& value);
  void SetCacheEntryBoolProperty(std::string const& key,
                                 std::string const& propertyName, bool value);
  std::vector<std::string> GetCacheEntryPropertyList(std::string const& key);
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
                              std::string& value,
                              cmStateEnums::CacheEntryType& type);

  cmStateSnapshot Reset();
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

  bool GetIsGeneratorMultiConfig() const;
  void SetIsGeneratorMultiConfig(bool b);

  // Returns a command from its name, case insensitive, or nullptr
  cmCommand* GetCommand(std::string const& name) const;
  // Returns a command from its name, or nullptr
  cmCommand* GetCommandByExactName(std::string const& name) const;

  void AddBuiltinCommand(std::string const& name, cmCommand* command);
  void AddDisallowedCommand(std::string const& name, cmCommand* command,
                            cmPolicies::PolicyID policy, const char* message);
  void AddUnexpectedCommand(std::string const& name, const char* error);
  void AddScriptedCommand(std::string const& name, cmCommand* command);
  void RemoveBuiltinCommand(std::string const& name);
  void RemoveUserDefinedCommands();
  std::vector<std::string> GetCommandNames() const;

  void SetGlobalProperty(const std::string& prop, const char* value);
  void AppendGlobalProperty(const std::string& prop, const char* value,
                            bool asString = false);
  const char* GetGlobalProperty(const std::string& prop);
  bool GetGlobalPropertyAsBool(const std::string& prop);

  std::string const& GetSourceDirectory() const;
  void SetSourceDirectory(std::string const& sourceDirectory);
  std::string const& GetBinaryDirectory() const;
  void SetBinaryDirectory(std::string const& binaryDirectory);

  void SetWindowsShell(bool windowsShell);
  bool UseWindowsShell() const;
  void SetWindowsVSIDE(bool windowsVSIDE);
  bool UseWindowsVSIDE() const;
  void SetGhsMultiIDE(bool ghsMultiIDE);
  bool UseGhsMultiIDE() const;
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

  Mode GetMode() const;
  std::string GetModeString() const;
  void SetMode(Mode mode);

  static std::string ModeToString(Mode mode);

private:
  friend class cmake;
  void AddCacheEntry(const std::string& key, const char* value,
                     const char* helpString,
                     cmStateEnums::CacheEntryType type);

  bool DoWriteGlobVerifyTarget() const;
  std::string const& GetGlobVerifyScript() const;
  std::string const& GetGlobVerifyStamp() const;
  bool SaveVerificationScript(const std::string& path);
  void AddGlobCacheEntry(bool recurse, bool listDirectories,
                         bool followSymlinks, const std::string& relative,
                         const std::string& expression,
                         const std::vector<std::string>& files,
                         const std::string& variable,
                         cmListFileBacktrace const& bt);

  std::map<cmProperty::ScopeType, cmPropertyDefinitionMap> PropertyDefinitions;
  std::vector<std::string> EnabledLanguages;
  std::map<std::string, cmCommand*> BuiltinCommands;
  std::map<std::string, cmCommand*> ScriptedCommands;
  cmPropertyMap GlobalProperties;
  cmCacheManager* CacheManager;
  cmGlobVerificationManager* GlobVerificationManager;

  cmLinkedTree<cmStateDetail::BuildsystemDirectoryStateType>
    BuildsystemDirectory;

  cmLinkedTree<std::string> ExecutionListFiles;

  cmLinkedTree<cmStateDetail::PolicyStackEntry> PolicyStack;
  cmLinkedTree<cmStateDetail::SnapshotDataType> SnapshotData;
  cmLinkedTree<cmDefinitions> VarTree;

  std::string SourceDirectory;
  std::string BinaryDirectory;
  bool IsInTryCompile = false;
  bool IsGeneratorMultiConfig = false;
  bool WindowsShell = false;
  bool WindowsVSIDE = false;
  bool GhsMultiIDE = false;
  bool WatcomWMake = false;
  bool MinGWMake = false;
  bool NMake = false;
  bool MSYSShell = false;
  Mode CurrentMode = Unknown;
};

#endif
