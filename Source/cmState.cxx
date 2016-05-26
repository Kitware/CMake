/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2015 Stephen Kelly <steveire@gmail.com>

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmState.h"

#include "cmAlgorithms.h"
#include "cmCacheManager.h"
#include "cmCommand.h"
#include "cmDefinitions.h"
#include "cmVersion.h"
#include "cmake.h"

#include <assert.h>

struct cmState::SnapshotDataType
{
  cmState::PositionType ScopeParent;
  cmState::PositionType DirectoryParent;
  cmLinkedTree<cmState::PolicyStackEntry>::iterator Policies;
  cmLinkedTree<cmState::PolicyStackEntry>::iterator PolicyRoot;
  cmLinkedTree<cmState::PolicyStackEntry>::iterator PolicyScope;
  cmState::SnapshotType SnapshotType;
  bool Keep;
  cmLinkedTree<std::string>::iterator ExecutionListFile;
  cmLinkedTree<cmState::BuildsystemDirectoryStateType>::iterator
    BuildSystemDirectory;
  cmLinkedTree<cmDefinitions>::iterator Vars;
  cmLinkedTree<cmDefinitions>::iterator Root;
  cmLinkedTree<cmDefinitions>::iterator Parent;
  std::vector<std::string>::size_type IncludeDirectoryPosition;
  std::vector<std::string>::size_type CompileDefinitionsPosition;
  std::vector<std::string>::size_type CompileOptionsPosition;
};

struct cmState::PolicyStackEntry : public cmPolicies::PolicyMap
{
  typedef cmPolicies::PolicyMap derived;
  PolicyStackEntry(bool w = false)
    : derived()
    , Weak(w)
  {
  }
  PolicyStackEntry(derived const& d, bool w)
    : derived(d)
    , Weak(w)
  {
  }
  PolicyStackEntry(PolicyStackEntry const& r)
    : derived(r)
    , Weak(r.Weak)
  {
  }
  bool Weak;
};

struct cmState::BuildsystemDirectoryStateType
{
  cmState::PositionType DirectoryEnd;

  std::string Location;
  std::string OutputLocation;

  std::vector<std::string> CurrentSourceDirectoryComponents;
  std::vector<std::string> CurrentBinaryDirectoryComponents;
  // The top-most directories for relative path conversion.  Both the
  // source and destination location of a relative path conversion
  // must be underneath one of these directories (both under source or
  // both under binary) in order for the relative path to be evaluated
  // safely by the build tools.
  std::string RelativePathTopSource;
  std::string RelativePathTopBinary;

  std::vector<std::string> IncludeDirectories;
  std::vector<cmListFileBacktrace> IncludeDirectoryBacktraces;

  std::vector<std::string> CompileDefinitions;
  std::vector<cmListFileBacktrace> CompileDefinitionsBacktraces;

  std::vector<std::string> CompileOptions;
  std::vector<cmListFileBacktrace> CompileOptionsBacktraces;

  std::string ProjectName;

  cmPropertyMap Properties;

  std::vector<cmState::Snapshot> Children;
};

cmState::cmState()
  : IsInTryCompile(false)
  , WindowsShell(false)
  , WindowsVSIDE(false)
  , WatcomWMake(false)
  , MinGWMake(false)
  , NMake(false)
  , MSYSShell(false)
{
  this->CacheManager = new cmCacheManager;
}

cmState::~cmState()
{
  delete this->CacheManager;
  cmDeleteAll(this->Commands);
}

const char* cmState::GetTargetTypeName(cmState::TargetType targetType)
{
  switch (targetType) {
    case cmState::STATIC_LIBRARY:
      return "STATIC_LIBRARY";
    case cmState::MODULE_LIBRARY:
      return "MODULE_LIBRARY";
    case cmState::SHARED_LIBRARY:
      return "SHARED_LIBRARY";
    case cmState::OBJECT_LIBRARY:
      return "OBJECT_LIBRARY";
    case cmState::EXECUTABLE:
      return "EXECUTABLE";
    case cmState::UTILITY:
      return "UTILITY";
    case cmState::GLOBAL_TARGET:
      return "GLOBAL_TARGET";
    case cmState::INTERFACE_LIBRARY:
      return "INTERFACE_LIBRARY";
    case cmState::UNKNOWN_LIBRARY:
      return "UNKNOWN_LIBRARY";
  }
  assert(0 && "Unexpected target type");
  return 0;
}

const char* cmCacheEntryTypes[] = { "BOOL",          "PATH",     "FILEPATH",
                                    "STRING",        "INTERNAL", "STATIC",
                                    "UNINITIALIZED", 0 };

const char* cmState::CacheEntryTypeToString(cmState::CacheEntryType type)
{
  if (type > 6) {
    return cmCacheEntryTypes[6];
  }
  return cmCacheEntryTypes[type];
}

cmState::CacheEntryType cmState::StringToCacheEntryType(const char* s)
{
  int i = 0;
  while (cmCacheEntryTypes[i]) {
    if (strcmp(s, cmCacheEntryTypes[i]) == 0) {
      return static_cast<cmState::CacheEntryType>(i);
    }
    ++i;
  }
  return STRING;
}

bool cmState::IsCacheEntryType(std::string const& key)
{
  for (int i = 0; cmCacheEntryTypes[i]; ++i) {
    if (strcmp(key.c_str(), cmCacheEntryTypes[i]) == 0) {
      return true;
    }
  }
  return false;
}

bool cmState::LoadCache(const std::string& path, bool internal,
                        std::set<std::string>& excludes,
                        std::set<std::string>& includes)
{
  return this->CacheManager->LoadCache(path, internal, excludes, includes);
}

bool cmState::SaveCache(const std::string& path)
{
  return this->CacheManager->SaveCache(path);
}

bool cmState::DeleteCache(const std::string& path)
{
  return this->CacheManager->DeleteCache(path);
}

std::vector<std::string> cmState::GetCacheEntryKeys() const
{
  std::vector<std::string> definitions;
  definitions.reserve(this->CacheManager->GetSize());
  cmCacheManager::CacheIterator cit = this->CacheManager->GetCacheIterator();
  for (cit.Begin(); !cit.IsAtEnd(); cit.Next()) {
    definitions.push_back(cit.GetName());
  }
  return definitions;
}

const char* cmState::GetCacheEntryValue(std::string const& key) const
{
  cmCacheManager::CacheEntry* e = this->CacheManager->GetCacheEntry(key);
  if (!e) {
    return 0;
  }
  return e->Value.c_str();
}

const char* cmState::GetInitializedCacheValue(std::string const& key) const
{
  return this->CacheManager->GetInitializedCacheValue(key);
}

cmState::CacheEntryType cmState::GetCacheEntryType(
  std::string const& key) const
{
  cmCacheManager::CacheIterator it =
    this->CacheManager->GetCacheIterator(key.c_str());
  return it.GetType();
}

void cmState::SetCacheEntryValue(std::string const& key,
                                 std::string const& value)
{
  this->CacheManager->SetCacheEntryValue(key, value);
}

void cmState::SetCacheEntryProperty(std::string const& key,
                                    std::string const& propertyName,
                                    std::string const& value)
{
  cmCacheManager::CacheIterator it =
    this->CacheManager->GetCacheIterator(key.c_str());
  it.SetProperty(propertyName, value.c_str());
}

void cmState::SetCacheEntryBoolProperty(std::string const& key,
                                        std::string const& propertyName,
                                        bool value)
{
  cmCacheManager::CacheIterator it =
    this->CacheManager->GetCacheIterator(key.c_str());
  it.SetProperty(propertyName, value);
}

const char* cmState::GetCacheEntryProperty(std::string const& key,
                                           std::string const& propertyName)
{
  cmCacheManager::CacheIterator it =
    this->CacheManager->GetCacheIterator(key.c_str());
  if (!it.PropertyExists(propertyName)) {
    return 0;
  }
  return it.GetProperty(propertyName);
}

bool cmState::GetCacheEntryPropertyAsBool(std::string const& key,
                                          std::string const& propertyName)
{
  return this->CacheManager->GetCacheIterator(key.c_str())
    .GetPropertyAsBool(propertyName);
}

void cmState::AddCacheEntry(const std::string& key, const char* value,
                            const char* helpString,
                            cmState::CacheEntryType type)
{
  this->CacheManager->AddCacheEntry(key, value, helpString, type);
}

void cmState::RemoveCacheEntry(std::string const& key)
{
  this->CacheManager->RemoveCacheEntry(key);
}

void cmState::AppendCacheEntryProperty(const std::string& key,
                                       const std::string& property,
                                       const std::string& value, bool asString)
{
  this->CacheManager->GetCacheIterator(key.c_str())
    .AppendProperty(property, value.c_str(), asString);
}

void cmState::RemoveCacheEntryProperty(std::string const& key,
                                       std::string const& propertyName)
{
  this->CacheManager->GetCacheIterator(key.c_str())
    .SetProperty(propertyName, (void*)0);
}

cmState::Snapshot cmState::Reset()
{
  this->GlobalProperties.clear();
  this->PropertyDefinitions.clear();

  PositionType pos = this->SnapshotData.Truncate();
  this->ExecutionListFiles.Truncate();

  {
    cmLinkedTree<BuildsystemDirectoryStateType>::iterator it =
      this->BuildsystemDirectory.Truncate();
    it->IncludeDirectories.clear();
    it->IncludeDirectoryBacktraces.clear();
    it->CompileDefinitions.clear();
    it->CompileDefinitionsBacktraces.clear();
    it->CompileOptions.clear();
    it->CompileOptionsBacktraces.clear();
    it->DirectoryEnd = pos;
    it->Properties.clear();
    it->Children.clear();
  }

  this->PolicyStack.Clear();
  pos->Policies = this->PolicyStack.Root();
  pos->PolicyRoot = this->PolicyStack.Root();
  pos->PolicyScope = this->PolicyStack.Root();
  assert(pos->Policies.IsValid());
  assert(pos->PolicyRoot.IsValid());

  {
    std::string srcDir =
      cmDefinitions::Get("CMAKE_SOURCE_DIR", pos->Vars, pos->Root);
    std::string binDir =
      cmDefinitions::Get("CMAKE_BINARY_DIR", pos->Vars, pos->Root);
    this->VarTree.Clear();
    pos->Vars = this->VarTree.Push(this->VarTree.Root());
    pos->Parent = this->VarTree.Root();
    pos->Root = this->VarTree.Root();

    pos->Vars->Set("CMAKE_SOURCE_DIR", srcDir.c_str());
    pos->Vars->Set("CMAKE_BINARY_DIR", binDir.c_str());
  }

  this->DefineProperty("RULE_LAUNCH_COMPILE", cmProperty::DIRECTORY, "", "",
                       true);
  this->DefineProperty("RULE_LAUNCH_LINK", cmProperty::DIRECTORY, "", "",
                       true);
  this->DefineProperty("RULE_LAUNCH_CUSTOM", cmProperty::DIRECTORY, "", "",
                       true);

  this->DefineProperty("RULE_LAUNCH_COMPILE", cmProperty::TARGET, "", "",
                       true);
  this->DefineProperty("RULE_LAUNCH_LINK", cmProperty::TARGET, "", "", true);
  this->DefineProperty("RULE_LAUNCH_CUSTOM", cmProperty::TARGET, "", "", true);

  return Snapshot(this, pos);
}

void cmState::DefineProperty(const std::string& name,
                             cmProperty::ScopeType scope,
                             const char* ShortDescription,
                             const char* FullDescription, bool chained)
{
  this->PropertyDefinitions[scope].DefineProperty(
    name, scope, ShortDescription, FullDescription, chained);
}

cmPropertyDefinition const* cmState::GetPropertyDefinition(
  const std::string& name, cmProperty::ScopeType scope) const
{
  if (this->IsPropertyDefined(name, scope)) {
    cmPropertyDefinitionMap const& defs =
      this->PropertyDefinitions.find(scope)->second;
    return &defs.find(name)->second;
  }
  return 0;
}

bool cmState::IsPropertyDefined(const std::string& name,
                                cmProperty::ScopeType scope) const
{
  std::map<cmProperty::ScopeType, cmPropertyDefinitionMap>::const_iterator it =
    this->PropertyDefinitions.find(scope);
  if (it == this->PropertyDefinitions.end()) {
    return false;
  }
  return it->second.IsPropertyDefined(name);
}

bool cmState::IsPropertyChained(const std::string& name,
                                cmProperty::ScopeType scope) const
{
  std::map<cmProperty::ScopeType, cmPropertyDefinitionMap>::const_iterator it =
    this->PropertyDefinitions.find(scope);
  if (it == this->PropertyDefinitions.end()) {
    return false;
  }
  return it->second.IsPropertyChained(name);
}

void cmState::SetLanguageEnabled(std::string const& l)
{
  std::vector<std::string>::iterator it = std::lower_bound(
    this->EnabledLanguages.begin(), this->EnabledLanguages.end(), l);
  if (it == this->EnabledLanguages.end() || *it != l) {
    this->EnabledLanguages.insert(it, l);
  }
}

bool cmState::GetLanguageEnabled(std::string const& l) const
{
  return std::binary_search(this->EnabledLanguages.begin(),
                            this->EnabledLanguages.end(), l);
}

std::vector<std::string> cmState::GetEnabledLanguages() const
{
  return this->EnabledLanguages;
}

void cmState::SetEnabledLanguages(std::vector<std::string> const& langs)
{
  this->EnabledLanguages = langs;
}

void cmState::ClearEnabledLanguages()
{
  this->EnabledLanguages.clear();
}

bool cmState::GetIsInTryCompile() const
{
  return this->IsInTryCompile;
}

void cmState::SetIsInTryCompile(bool b)
{
  this->IsInTryCompile = b;
}

void cmState::RenameCommand(std::string const& oldName,
                            std::string const& newName)
{
  // if the command already exists, free the old one
  std::string sOldName = cmSystemTools::LowerCase(oldName);
  std::string sNewName = cmSystemTools::LowerCase(newName);
  std::map<std::string, cmCommand*>::iterator pos =
    this->Commands.find(sOldName);
  if (pos == this->Commands.end()) {
    return;
  }
  cmCommand* cmd = pos->second;

  pos = this->Commands.find(sNewName);
  if (pos != this->Commands.end()) {
    delete pos->second;
    this->Commands.erase(pos);
  }
  this->Commands.insert(std::make_pair(sNewName, cmd));
  pos = this->Commands.find(sOldName);
  this->Commands.erase(pos);
}

void cmState::AddCommand(cmCommand* command)
{
  std::string name = cmSystemTools::LowerCase(command->GetName());
  // if the command already exists, free the old one
  std::map<std::string, cmCommand*>::iterator pos = this->Commands.find(name);
  if (pos != this->Commands.end()) {
    delete pos->second;
    this->Commands.erase(pos);
  }
  this->Commands.insert(std::make_pair(name, command));
}

void cmState::RemoveUnscriptableCommands()
{
  std::vector<std::string> unscriptableCommands;
  for (std::map<std::string, cmCommand*>::iterator pos =
         this->Commands.begin();
       pos != this->Commands.end();) {
    if (!pos->second->IsScriptable()) {
      delete pos->second;
      this->Commands.erase(pos++);
    } else {
      ++pos;
    }
  }
}

cmCommand* cmState::GetCommand(std::string const& name) const
{
  cmCommand* command = 0;
  std::string sName = cmSystemTools::LowerCase(name);
  std::map<std::string, cmCommand*>::const_iterator pos =
    this->Commands.find(sName);
  if (pos != this->Commands.end()) {
    command = (*pos).second;
  }
  return command;
}

std::vector<std::string> cmState::GetCommandNames() const
{
  std::vector<std::string> commandNames;
  commandNames.reserve(this->Commands.size());
  std::map<std::string, cmCommand*>::const_iterator cmds =
    this->Commands.begin();
  for (; cmds != this->Commands.end(); ++cmds) {
    commandNames.push_back(cmds->first);
  }
  return commandNames;
}

void cmState::RemoveUserDefinedCommands()
{
  std::vector<cmCommand*> renamedCommands;
  for (std::map<std::string, cmCommand*>::iterator j = this->Commands.begin();
       j != this->Commands.end();) {
    if (j->second->IsA("cmMacroHelperCommand") ||
        j->second->IsA("cmFunctionHelperCommand")) {
      delete j->second;
      this->Commands.erase(j++);
    } else if (j->first != j->second->GetName()) {
      renamedCommands.push_back(j->second);
      this->Commands.erase(j++);
    } else {
      ++j;
    }
  }
  for (std::vector<cmCommand*>::const_iterator it = renamedCommands.begin();
       it != renamedCommands.end(); ++it) {
    this->Commands[cmSystemTools::LowerCase((*it)->GetName())] = *it;
  }
}

void cmState::SetGlobalProperty(const std::string& prop, const char* value)
{
  this->GlobalProperties.SetProperty(prop, value);
}

void cmState::AppendGlobalProperty(const std::string& prop, const char* value,
                                   bool asString)
{
  this->GlobalProperties.AppendProperty(prop, value, asString);
}

const char* cmState::GetGlobalProperty(const std::string& prop)
{
  if (prop == "CACHE_VARIABLES") {
    std::vector<std::string> cacheKeys = this->GetCacheEntryKeys();
    this->SetGlobalProperty("CACHE_VARIABLES", cmJoin(cacheKeys, ";").c_str());
  } else if (prop == "COMMANDS") {
    std::vector<std::string> commands = this->GetCommandNames();
    this->SetGlobalProperty("COMMANDS", cmJoin(commands, ";").c_str());
  } else if (prop == "IN_TRY_COMPILE") {
    this->SetGlobalProperty("IN_TRY_COMPILE",
                            this->IsInTryCompile ? "1" : "0");
  } else if (prop == "ENABLED_LANGUAGES") {
    std::string langs;
    langs = cmJoin(this->EnabledLanguages, ";");
    this->SetGlobalProperty("ENABLED_LANGUAGES", langs.c_str());
  }
#define STRING_LIST_ELEMENT(F) ";" #F
  if (prop == "CMAKE_C_KNOWN_FEATURES") {
    return FOR_EACH_C_FEATURE(STRING_LIST_ELEMENT) + 1;
  }
  if (prop == "CMAKE_CXX_KNOWN_FEATURES") {
    return FOR_EACH_CXX_FEATURE(STRING_LIST_ELEMENT) + 1;
  }
#undef STRING_LIST_ELEMENT
  return this->GlobalProperties.GetPropertyValue(prop);
}

bool cmState::GetGlobalPropertyAsBool(const std::string& prop)
{
  return cmSystemTools::IsOn(this->GetGlobalProperty(prop));
}

void cmState::SetSourceDirectory(std::string const& sourceDirectory)
{
  this->SourceDirectory = sourceDirectory;
  cmSystemTools::ConvertToUnixSlashes(this->SourceDirectory);

  cmSystemTools::SplitPath(
    cmSystemTools::CollapseFullPath(this->SourceDirectory),
    this->SourceDirectoryComponents);
}

const char* cmState::GetSourceDirectory() const
{
  return this->SourceDirectory.c_str();
}

std::vector<std::string> const& cmState::GetSourceDirectoryComponents() const
{
  return this->SourceDirectoryComponents;
}

void cmState::SetBinaryDirectory(std::string const& binaryDirectory)
{
  this->BinaryDirectory = binaryDirectory;
  cmSystemTools::ConvertToUnixSlashes(this->BinaryDirectory);

  cmSystemTools::SplitPath(
    cmSystemTools::CollapseFullPath(this->BinaryDirectory),
    this->BinaryDirectoryComponents);
}

void cmState::SetWindowsShell(bool windowsShell)
{
  this->WindowsShell = windowsShell;
}

bool cmState::UseWindowsShell() const
{
  return this->WindowsShell;
}

void cmState::SetWindowsVSIDE(bool windowsVSIDE)
{
  this->WindowsVSIDE = windowsVSIDE;
}

bool cmState::UseWindowsVSIDE() const
{
  return this->WindowsVSIDE;
}

void cmState::SetWatcomWMake(bool watcomWMake)
{
  this->WatcomWMake = watcomWMake;
}

bool cmState::UseWatcomWMake() const
{
  return this->WatcomWMake;
}

void cmState::SetMinGWMake(bool minGWMake)
{
  this->MinGWMake = minGWMake;
}

bool cmState::UseMinGWMake() const
{
  return this->MinGWMake;
}

void cmState::SetNMake(bool nMake)
{
  this->NMake = nMake;
}

bool cmState::UseNMake() const
{
  return this->NMake;
}

void cmState::SetMSYSShell(bool mSYSShell)
{
  this->MSYSShell = mSYSShell;
}

bool cmState::UseMSYSShell() const
{
  return this->MSYSShell;
}

unsigned int cmState::GetCacheMajorVersion() const
{
  return this->CacheManager->GetCacheMajorVersion();
}

unsigned int cmState::GetCacheMinorVersion() const
{
  return this->CacheManager->GetCacheMinorVersion();
}

const char* cmState::GetBinaryDirectory() const
{
  return this->BinaryDirectory.c_str();
}

std::vector<std::string> const& cmState::GetBinaryDirectoryComponents() const
{
  return this->BinaryDirectoryComponents;
}

void cmState::Directory::ComputeRelativePathTopSource()
{
  // Relative path conversion inside the source tree is not used to
  // construct relative paths passed to build tools so it is safe to use
  // even when the source is a network path.

  cmState::Snapshot snapshot = this->Snapshot_;
  std::vector<cmState::Snapshot> snapshots;
  snapshots.push_back(snapshot);
  while (true) {
    snapshot = snapshot.GetBuildsystemDirectoryParent();
    if (snapshot.IsValid()) {
      snapshots.push_back(snapshot);
    } else {
      break;
    }
  }

  std::string result = snapshots.front().GetDirectory().GetCurrentSource();

  for (std::vector<cmState::Snapshot>::const_iterator it =
         snapshots.begin() + 1;
       it != snapshots.end(); ++it) {
    std::string currentSource = it->GetDirectory().GetCurrentSource();
    if (cmSystemTools::IsSubDirectory(result, currentSource)) {
      result = currentSource;
    }
  }
  this->DirectoryState->RelativePathTopSource = result;
}

void cmState::Directory::ComputeRelativePathTopBinary()
{
  cmState::Snapshot snapshot = this->Snapshot_;
  std::vector<cmState::Snapshot> snapshots;
  snapshots.push_back(snapshot);
  while (true) {
    snapshot = snapshot.GetBuildsystemDirectoryParent();
    if (snapshot.IsValid()) {
      snapshots.push_back(snapshot);
    } else {
      break;
    }
  }

  std::string result = snapshots.front().GetDirectory().GetCurrentBinary();

  for (std::vector<cmState::Snapshot>::const_iterator it =
         snapshots.begin() + 1;
       it != snapshots.end(); ++it) {
    std::string currentBinary = it->GetDirectory().GetCurrentBinary();
    if (cmSystemTools::IsSubDirectory(result, currentBinary)) {
      result = currentBinary;
    }
  }

  // The current working directory on Windows cannot be a network
  // path.  Therefore relative paths cannot work when the binary tree
  // is a network path.
  if (result.size() < 2 || result.substr(0, 2) != "//") {
    this->DirectoryState->RelativePathTopBinary = result;
  } else {
    this->DirectoryState->RelativePathTopBinary = "";
  }
}

cmState::Snapshot cmState::CreateBaseSnapshot()
{
  PositionType pos = this->SnapshotData.Push(this->SnapshotData.Root());
  pos->DirectoryParent = this->SnapshotData.Root();
  pos->ScopeParent = this->SnapshotData.Root();
  pos->SnapshotType = BaseType;
  pos->Keep = true;
  pos->BuildSystemDirectory =
    this->BuildsystemDirectory.Push(this->BuildsystemDirectory.Root());
  pos->ExecutionListFile =
    this->ExecutionListFiles.Push(this->ExecutionListFiles.Root());
  pos->IncludeDirectoryPosition = 0;
  pos->CompileDefinitionsPosition = 0;
  pos->CompileOptionsPosition = 0;
  pos->BuildSystemDirectory->DirectoryEnd = pos;
  pos->Policies = this->PolicyStack.Root();
  pos->PolicyRoot = this->PolicyStack.Root();
  pos->PolicyScope = this->PolicyStack.Root();
  assert(pos->Policies.IsValid());
  assert(pos->PolicyRoot.IsValid());
  pos->Vars = this->VarTree.Push(this->VarTree.Root());
  assert(pos->Vars.IsValid());
  pos->Parent = this->VarTree.Root();
  pos->Root = this->VarTree.Root();
  return cmState::Snapshot(this, pos);
}

cmState::Snapshot cmState::CreateBuildsystemDirectorySnapshot(
  Snapshot originSnapshot)
{
  assert(originSnapshot.IsValid());
  PositionType pos = this->SnapshotData.Push(originSnapshot.Position);
  pos->DirectoryParent = originSnapshot.Position;
  pos->ScopeParent = originSnapshot.Position;
  pos->SnapshotType = BuildsystemDirectoryType;
  pos->Keep = true;
  pos->BuildSystemDirectory = this->BuildsystemDirectory.Push(
    originSnapshot.Position->BuildSystemDirectory);
  pos->ExecutionListFile =
    this->ExecutionListFiles.Push(originSnapshot.Position->ExecutionListFile);
  pos->BuildSystemDirectory->DirectoryEnd = pos;
  pos->Policies = originSnapshot.Position->Policies;
  pos->PolicyRoot = originSnapshot.Position->Policies;
  pos->PolicyScope = originSnapshot.Position->Policies;
  assert(pos->Policies.IsValid());
  assert(pos->PolicyRoot.IsValid());

  cmLinkedTree<cmDefinitions>::iterator origin = originSnapshot.Position->Vars;
  pos->Parent = origin;
  pos->Root = origin;
  pos->Vars = this->VarTree.Push(origin);

  cmState::Snapshot snapshot = cmState::Snapshot(this, pos);
  originSnapshot.Position->BuildSystemDirectory->Children.push_back(snapshot);
  snapshot.SetDefaultDefinitions();
  snapshot.InitializeFromParent();
  snapshot.SetDirectoryDefinitions();
  return snapshot;
}

cmState::Snapshot cmState::CreateFunctionCallSnapshot(
  cmState::Snapshot originSnapshot, std::string const& fileName)
{
  PositionType pos =
    this->SnapshotData.Push(originSnapshot.Position, *originSnapshot.Position);
  pos->ScopeParent = originSnapshot.Position;
  pos->SnapshotType = FunctionCallType;
  pos->Keep = false;
  pos->ExecutionListFile = this->ExecutionListFiles.Push(
    originSnapshot.Position->ExecutionListFile, fileName);
  pos->BuildSystemDirectory->DirectoryEnd = pos;
  pos->PolicyScope = originSnapshot.Position->Policies;
  assert(originSnapshot.Position->Vars.IsValid());
  cmLinkedTree<cmDefinitions>::iterator origin = originSnapshot.Position->Vars;
  pos->Parent = origin;
  pos->Vars = this->VarTree.Push(origin);
  return cmState::Snapshot(this, pos);
}

cmState::Snapshot cmState::CreateMacroCallSnapshot(
  cmState::Snapshot originSnapshot, std::string const& fileName)
{
  PositionType pos =
    this->SnapshotData.Push(originSnapshot.Position, *originSnapshot.Position);
  pos->SnapshotType = MacroCallType;
  pos->Keep = false;
  pos->ExecutionListFile = this->ExecutionListFiles.Push(
    originSnapshot.Position->ExecutionListFile, fileName);
  assert(originSnapshot.Position->Vars.IsValid());
  pos->BuildSystemDirectory->DirectoryEnd = pos;
  pos->PolicyScope = originSnapshot.Position->Policies;
  return cmState::Snapshot(this, pos);
}

cmState::Snapshot cmState::CreateIncludeFileSnapshot(
  cmState::Snapshot originSnapshot, const std::string& fileName)
{
  PositionType pos =
    this->SnapshotData.Push(originSnapshot.Position, *originSnapshot.Position);
  pos->SnapshotType = IncludeFileType;
  pos->Keep = true;
  pos->ExecutionListFile = this->ExecutionListFiles.Push(
    originSnapshot.Position->ExecutionListFile, fileName);
  assert(originSnapshot.Position->Vars.IsValid());
  pos->BuildSystemDirectory->DirectoryEnd = pos;
  pos->PolicyScope = originSnapshot.Position->Policies;
  return cmState::Snapshot(this, pos);
}

cmState::Snapshot cmState::CreateVariableScopeSnapshot(
  cmState::Snapshot originSnapshot)
{
  PositionType pos =
    this->SnapshotData.Push(originSnapshot.Position, *originSnapshot.Position);
  pos->ScopeParent = originSnapshot.Position;
  pos->SnapshotType = VariableScopeType;
  pos->Keep = false;
  pos->PolicyScope = originSnapshot.Position->Policies;
  assert(originSnapshot.Position->Vars.IsValid());

  cmLinkedTree<cmDefinitions>::iterator origin = originSnapshot.Position->Vars;
  pos->Parent = origin;
  pos->Vars = this->VarTree.Push(origin);
  assert(pos->Vars.IsValid());
  return cmState::Snapshot(this, pos);
}

cmState::Snapshot cmState::CreateInlineListFileSnapshot(
  cmState::Snapshot originSnapshot, const std::string& fileName)
{
  PositionType pos =
    this->SnapshotData.Push(originSnapshot.Position, *originSnapshot.Position);
  pos->SnapshotType = InlineListFileType;
  pos->Keep = true;
  pos->ExecutionListFile = this->ExecutionListFiles.Push(
    originSnapshot.Position->ExecutionListFile, fileName);
  pos->BuildSystemDirectory->DirectoryEnd = pos;
  pos->PolicyScope = originSnapshot.Position->Policies;
  return cmState::Snapshot(this, pos);
}

cmState::Snapshot cmState::CreatePolicyScopeSnapshot(
  cmState::Snapshot originSnapshot)
{
  PositionType pos =
    this->SnapshotData.Push(originSnapshot.Position, *originSnapshot.Position);
  pos->SnapshotType = PolicyScopeType;
  pos->Keep = false;
  pos->BuildSystemDirectory->DirectoryEnd = pos;
  pos->PolicyScope = originSnapshot.Position->Policies;
  return cmState::Snapshot(this, pos);
}

cmState::Snapshot cmState::Pop(cmState::Snapshot originSnapshot)
{
  PositionType pos = originSnapshot.Position;
  PositionType prevPos = pos;
  ++prevPos;
  prevPos->IncludeDirectoryPosition =
    prevPos->BuildSystemDirectory->IncludeDirectories.size();
  prevPos->CompileDefinitionsPosition =
    prevPos->BuildSystemDirectory->CompileDefinitions.size();
  prevPos->CompileOptionsPosition =
    prevPos->BuildSystemDirectory->CompileOptions.size();
  prevPos->BuildSystemDirectory->DirectoryEnd = prevPos;

  if (!pos->Keep && this->SnapshotData.IsLast(pos)) {
    if (pos->Vars != prevPos->Vars) {
      assert(this->VarTree.IsLast(pos->Vars));
      this->VarTree.Pop(pos->Vars);
    }
    if (pos->ExecutionListFile != prevPos->ExecutionListFile) {
      assert(this->ExecutionListFiles.IsLast(pos->ExecutionListFile));
      this->ExecutionListFiles.Pop(pos->ExecutionListFile);
    }
    this->SnapshotData.Pop(pos);
  }

  return Snapshot(this, prevPos);
}

cmState::Snapshot::Snapshot(cmState* state)
  : State(state)
  , Position()
{
}

std::vector<cmState::Snapshot> cmState::Snapshot::GetChildren()
{
  return this->Position->BuildSystemDirectory->Children;
}

cmState::Snapshot::Snapshot(cmState* state, PositionType position)
  : State(state)
  , Position(position)
{
}

cmState::SnapshotType cmState::Snapshot::GetType() const
{
  return this->Position->SnapshotType;
}

const char* cmState::Directory::GetCurrentSource() const
{
  return this->DirectoryState->Location.c_str();
}

void cmState::Directory::SetCurrentSource(std::string const& dir)
{
  std::string& loc = this->DirectoryState->Location;
  loc = dir;
  cmSystemTools::ConvertToUnixSlashes(loc);
  loc = cmSystemTools::CollapseFullPath(loc);

  cmSystemTools::SplitPath(
    loc, this->DirectoryState->CurrentSourceDirectoryComponents);
  this->ComputeRelativePathTopSource();

  this->Snapshot_.SetDefinition("CMAKE_CURRENT_SOURCE_DIR", loc.c_str());
}

const char* cmState::Directory::GetCurrentBinary() const
{
  return this->DirectoryState->OutputLocation.c_str();
}

void cmState::Directory::SetCurrentBinary(std::string const& dir)
{
  std::string& loc = this->DirectoryState->OutputLocation;
  loc = dir;
  cmSystemTools::ConvertToUnixSlashes(loc);
  loc = cmSystemTools::CollapseFullPath(loc);

  cmSystemTools::SplitPath(
    loc, this->DirectoryState->CurrentBinaryDirectoryComponents);
  this->ComputeRelativePathTopBinary();

  this->Snapshot_.SetDefinition("CMAKE_CURRENT_BINARY_DIR", loc.c_str());
}

void cmState::Snapshot::SetListFile(const std::string& listfile)
{
  *this->Position->ExecutionListFile = listfile;
}

std::vector<std::string> const&
cmState::Directory::GetCurrentSourceComponents() const
{
  return this->DirectoryState->CurrentSourceDirectoryComponents;
}

std::vector<std::string> const&
cmState::Directory::GetCurrentBinaryComponents() const
{
  return this->DirectoryState->CurrentBinaryDirectoryComponents;
}

const char* cmState::Directory::GetRelativePathTopSource() const
{
  return this->DirectoryState->RelativePathTopSource.c_str();
}

const char* cmState::Directory::GetRelativePathTopBinary() const
{
  return this->DirectoryState->RelativePathTopBinary.c_str();
}

void cmState::Directory::SetRelativePathTopSource(const char* dir)
{
  this->DirectoryState->RelativePathTopSource = dir;
}

void cmState::Directory::SetRelativePathTopBinary(const char* dir)
{
  this->DirectoryState->RelativePathTopBinary = dir;
}

std::string cmState::Snapshot::GetExecutionListFile() const
{
  return *this->Position->ExecutionListFile;
}

bool cmState::Snapshot::IsValid() const
{
  return this->State && this->Position.IsValid()
    ? this->Position != this->State->SnapshotData.Root()
    : false;
}

cmState::Snapshot cmState::Snapshot::GetBuildsystemDirectoryParent() const
{
  Snapshot snapshot;
  if (!this->State || this->Position == this->State->SnapshotData.Root()) {
    return snapshot;
  }
  PositionType parentPos = this->Position->DirectoryParent;
  if (parentPos != this->State->SnapshotData.Root()) {
    snapshot =
      Snapshot(this->State, parentPos->BuildSystemDirectory->DirectoryEnd);
  }

  return snapshot;
}

cmState::Snapshot cmState::Snapshot::GetCallStackParent() const
{
  assert(this->State);
  assert(this->Position != this->State->SnapshotData.Root());

  Snapshot snapshot;
  PositionType parentPos = this->Position;
  while (parentPos->SnapshotType == cmState::PolicyScopeType ||
         parentPos->SnapshotType == cmState::VariableScopeType) {
    ++parentPos;
  }
  if (parentPos->SnapshotType == cmState::BuildsystemDirectoryType ||
      parentPos->SnapshotType == cmState::BaseType) {
    return snapshot;
  }

  ++parentPos;
  while (parentPos->SnapshotType == cmState::PolicyScopeType ||
         parentPos->SnapshotType == cmState::VariableScopeType) {
    ++parentPos;
  }

  if (parentPos == this->State->SnapshotData.Root()) {
    return snapshot;
  }

  snapshot = Snapshot(this->State, parentPos);
  return snapshot;
}

cmState::Snapshot cmState::Snapshot::GetCallStackBottom() const
{
  assert(this->State);
  assert(this->Position != this->State->SnapshotData.Root());

  PositionType pos = this->Position;
  while (pos->SnapshotType != cmState::BaseType &&
         pos->SnapshotType != cmState::BuildsystemDirectoryType &&
         pos != this->State->SnapshotData.Root()) {
    ++pos;
  }
  return Snapshot(this->State, pos);
}

void cmState::Snapshot::PushPolicy(cmPolicies::PolicyMap entry, bool weak)
{
  PositionType pos = this->Position;
  pos->Policies = this->State->PolicyStack.Push(pos->Policies,
                                                PolicyStackEntry(entry, weak));
}

bool cmState::Snapshot::PopPolicy()
{
  PositionType pos = this->Position;
  if (pos->Policies == pos->PolicyScope) {
    return false;
  }
  pos->Policies = this->State->PolicyStack.Pop(pos->Policies);
  return true;
}

bool cmState::Snapshot::CanPopPolicyScope()
{
  return this->Position->Policies == this->Position->PolicyScope;
}

void cmState::Snapshot::SetPolicy(cmPolicies::PolicyID id,
                                  cmPolicies::PolicyStatus status)
{
  // Update the policy stack from the top to the top-most strong entry.
  bool previous_was_weak = true;
  for (cmLinkedTree<PolicyStackEntry>::iterator psi = this->Position->Policies;
       previous_was_weak && psi != this->Position->PolicyRoot; ++psi) {
    psi->Set(id, status);
    previous_was_weak = psi->Weak;
  }
}

cmPolicies::PolicyStatus cmState::Snapshot::GetPolicy(
  cmPolicies::PolicyID id) const
{
  cmPolicies::PolicyStatus status = cmPolicies::GetPolicyStatus(id);

  if (status == cmPolicies::REQUIRED_ALWAYS ||
      status == cmPolicies::REQUIRED_IF_USED) {
    return status;
  }

  cmLinkedTree<BuildsystemDirectoryStateType>::iterator dir =
    this->Position->BuildSystemDirectory;

  while (true) {
    assert(dir.IsValid());
    cmLinkedTree<PolicyStackEntry>::iterator leaf =
      dir->DirectoryEnd->Policies;
    cmLinkedTree<PolicyStackEntry>::iterator root =
      dir->DirectoryEnd->PolicyRoot;
    for (; leaf != root; ++leaf) {
      if (leaf->IsDefined(id)) {
        status = leaf->Get(id);
        return status;
      }
    }
    cmState::PositionType e = dir->DirectoryEnd;
    cmState::PositionType p = e->DirectoryParent;
    if (p == this->State->SnapshotData.Root()) {
      break;
    }
    dir = p->BuildSystemDirectory;
  }
  return status;
}

bool cmState::Snapshot::HasDefinedPolicyCMP0011()
{
  return !this->Position->Policies->IsEmpty();
}

const char* cmState::Snapshot::GetDefinition(std::string const& name) const
{
  assert(this->Position->Vars.IsValid());
  return cmDefinitions::Get(name, this->Position->Vars, this->Position->Root);
}

bool cmState::Snapshot::IsInitialized(std::string const& name) const
{
  return cmDefinitions::HasKey(name, this->Position->Vars,
                               this->Position->Root);
}

void cmState::Snapshot::SetDefinition(std::string const& name,
                                      std::string const& value)
{
  this->Position->Vars->Set(name, value.c_str());
}

void cmState::Snapshot::RemoveDefinition(std::string const& name)
{
  this->Position->Vars->Set(name, 0);
}

std::vector<std::string> cmState::Snapshot::UnusedKeys() const
{
  return this->Position->Vars->UnusedKeys();
}

std::vector<std::string> cmState::Snapshot::ClosureKeys() const
{
  return cmDefinitions::ClosureKeys(this->Position->Vars,
                                    this->Position->Root);
}

bool cmState::Snapshot::RaiseScope(std::string const& var, const char* varDef)
{
  if (this->Position->ScopeParent == this->Position->DirectoryParent) {
    Snapshot parentDir = this->GetBuildsystemDirectoryParent();
    if (!parentDir.IsValid()) {
      return false;
    }
    // Update the definition in the parent directory top scope.  This
    // directory's scope was initialized by the closure of the parent
    // scope, so we do not need to localize the definition first.
    if (varDef) {
      parentDir.SetDefinition(var, varDef);
    } else {
      parentDir.RemoveDefinition(var);
    }
    return true;
  }
  // First localize the definition in the current scope.
  cmDefinitions::Raise(var, this->Position->Vars, this->Position->Root);

  // Now update the definition in the parent scope.
  this->Position->Parent->Set(var, varDef);
  return true;
}

static const std::string cmPropertySentinal = std::string();

template <typename T, typename U, typename V>
void InitializeContentFromParent(T& parentContent, T& thisContent,
                                 U& parentBacktraces, U& thisBacktraces,
                                 V& contentEndPosition)
{
  std::vector<std::string>::const_iterator parentBegin = parentContent.begin();
  std::vector<std::string>::const_iterator parentEnd = parentContent.end();

  std::vector<std::string>::const_reverse_iterator parentRbegin =
    cmMakeReverseIterator(parentEnd);
  std::vector<std::string>::const_reverse_iterator parentRend =
    parentContent.rend();
  parentRbegin = std::find(parentRbegin, parentRend, cmPropertySentinal);
  std::vector<std::string>::const_iterator parentIt = parentRbegin.base();

  thisContent = std::vector<std::string>(parentIt, parentEnd);

  std::vector<cmListFileBacktrace>::const_iterator btIt =
    parentBacktraces.begin() + std::distance(parentBegin, parentIt);
  std::vector<cmListFileBacktrace>::const_iterator btEnd =
    parentBacktraces.end();

  thisBacktraces = std::vector<cmListFileBacktrace>(btIt, btEnd);

  contentEndPosition = thisContent.size();
}

void cmState::Snapshot::SetDefaultDefinitions()
{
/* Up to CMake 2.4 here only WIN32, UNIX and APPLE were set.
  With CMake must separate between target and host platform. In most cases
  the tests for WIN32, UNIX and APPLE will be for the target system, so an
  additional set of variables for the host system is required ->
  CMAKE_HOST_WIN32, CMAKE_HOST_UNIX, CMAKE_HOST_APPLE.
  WIN32, UNIX and APPLE are now set in the platform files in
  Modules/Platforms/.
  To keep cmake scripts (-P) and custom language and compiler modules
  working, these variables are still also set here in this place, but they
  will be reset in CMakeSystemSpecificInformation.cmake before the platform
  files are executed. */
#if defined(_WIN32)
  this->SetDefinition("WIN32", "1");
  this->SetDefinition("CMAKE_HOST_WIN32", "1");
#else
  this->SetDefinition("UNIX", "1");
  this->SetDefinition("CMAKE_HOST_UNIX", "1");
#endif
#if defined(__CYGWIN__)
  if (cmSystemTools::IsOn(
        cmSystemTools::GetEnv("CMAKE_LEGACY_CYGWIN_WIN32"))) {
    this->SetDefinition("WIN32", "1");
    this->SetDefinition("CMAKE_HOST_WIN32", "1");
  }
#endif
#if defined(__APPLE__)
  this->SetDefinition("APPLE", "1");
  this->SetDefinition("CMAKE_HOST_APPLE", "1");
#endif
#if defined(__sun__)
  this->SetDefinition("CMAKE_HOST_SOLARIS", "1");
#endif

  char temp[1024];
  sprintf(temp, "%d", cmVersion::GetMinorVersion());
  this->SetDefinition("CMAKE_MINOR_VERSION", temp);
  sprintf(temp, "%d", cmVersion::GetMajorVersion());
  this->SetDefinition("CMAKE_MAJOR_VERSION", temp);
  sprintf(temp, "%d", cmVersion::GetPatchVersion());
  this->SetDefinition("CMAKE_PATCH_VERSION", temp);
  sprintf(temp, "%d", cmVersion::GetTweakVersion());
  this->SetDefinition("CMAKE_TWEAK_VERSION", temp);
  this->SetDefinition("CMAKE_VERSION", cmVersion::GetCMakeVersion());

  this->SetDefinition("CMAKE_FILES_DIRECTORY",
                      cmake::GetCMakeFilesDirectory());

  // Setup the default include file regular expression (match everything).
  this->Position->BuildSystemDirectory->Properties.SetProperty(
    "INCLUDE_REGULAR_EXPRESSION", "^.*$");
}

void cmState::Snapshot::SetDirectoryDefinitions()
{
  this->SetDefinition("CMAKE_SOURCE_DIR", this->State->GetSourceDirectory());
  this->SetDefinition("CMAKE_CURRENT_SOURCE_DIR",
                      this->State->GetSourceDirectory());
  this->SetDefinition("CMAKE_BINARY_DIR", this->State->GetBinaryDirectory());
  this->SetDefinition("CMAKE_CURRENT_BINARY_DIR",
                      this->State->GetBinaryDirectory());
}

void cmState::Snapshot::InitializeFromParent()
{
  PositionType parent = this->Position->DirectoryParent;
  assert(this->Position->Vars.IsValid());
  assert(parent->Vars.IsValid());

  *this->Position->Vars =
    cmDefinitions::MakeClosure(parent->Vars, parent->Root);

  InitializeContentFromParent(
    parent->BuildSystemDirectory->IncludeDirectories,
    this->Position->BuildSystemDirectory->IncludeDirectories,
    parent->BuildSystemDirectory->IncludeDirectoryBacktraces,
    this->Position->BuildSystemDirectory->IncludeDirectoryBacktraces,
    this->Position->IncludeDirectoryPosition);

  InitializeContentFromParent(
    parent->BuildSystemDirectory->CompileDefinitions,
    this->Position->BuildSystemDirectory->CompileDefinitions,
    parent->BuildSystemDirectory->CompileDefinitionsBacktraces,
    this->Position->BuildSystemDirectory->CompileDefinitionsBacktraces,
    this->Position->CompileDefinitionsPosition);

  InitializeContentFromParent(
    parent->BuildSystemDirectory->CompileOptions,
    this->Position->BuildSystemDirectory->CompileOptions,
    parent->BuildSystemDirectory->CompileOptionsBacktraces,
    this->Position->BuildSystemDirectory->CompileOptionsBacktraces,
    this->Position->CompileOptionsPosition);
}

cmState* cmState::Snapshot::GetState() const
{
  return this->State;
}

cmState::Directory cmState::Snapshot::GetDirectory() const
{
  return Directory(this->Position->BuildSystemDirectory, *this);
}

void cmState::Snapshot::SetProjectName(const std::string& name)
{
  this->Position->BuildSystemDirectory->ProjectName = name;
}

std::string cmState::Snapshot::GetProjectName() const
{
  return this->Position->BuildSystemDirectory->ProjectName;
}

void cmState::Snapshot::InitializeFromParent_ForSubdirsCommand()
{
  std::string currentSrcDir = this->GetDefinition("CMAKE_CURRENT_SOURCE_DIR");
  std::string currentBinDir = this->GetDefinition("CMAKE_CURRENT_BINARY_DIR");
  this->InitializeFromParent();
  this->SetDefinition("CMAKE_SOURCE_DIR", this->State->GetSourceDirectory());
  this->SetDefinition("CMAKE_BINARY_DIR", this->State->GetBinaryDirectory());

  this->SetDefinition("CMAKE_CURRENT_SOURCE_DIR", currentSrcDir.c_str());
  this->SetDefinition("CMAKE_CURRENT_BINARY_DIR", currentBinDir.c_str());
}

cmState::Directory::Directory(
  cmLinkedTree<BuildsystemDirectoryStateType>::iterator iter,
  const cmState::Snapshot& snapshot)
  : DirectoryState(iter)
  , Snapshot_(snapshot)
{
}

template <typename T, typename U>
cmStringRange GetPropertyContent(T const& content, U contentEndPosition)
{
  std::vector<std::string>::const_iterator end =
    content.begin() + contentEndPosition;

  std::vector<std::string>::const_reverse_iterator rbegin =
    cmMakeReverseIterator(end);
  rbegin = std::find(rbegin, content.rend(), cmPropertySentinal);

  return cmMakeRange(rbegin.base(), end);
}

template <typename T, typename U, typename V>
cmBacktraceRange GetPropertyBacktraces(T const& content, U const& backtraces,
                                       V contentEndPosition)
{
  std::vector<std::string>::const_iterator entryEnd =
    content.begin() + contentEndPosition;

  std::vector<std::string>::const_reverse_iterator rbegin =
    cmMakeReverseIterator(entryEnd);
  rbegin = std::find(rbegin, content.rend(), cmPropertySentinal);

  std::vector<cmListFileBacktrace>::const_iterator it =
    backtraces.begin() + std::distance(content.begin(), rbegin.base());

  std::vector<cmListFileBacktrace>::const_iterator end = backtraces.end();
  return cmMakeRange(it, end);
}

template <typename T, typename U, typename V>
void AppendEntry(T& content, U& backtraces, V& endContentPosition,
                 const std::string& value, const cmListFileBacktrace& lfbt)
{
  if (value.empty()) {
    return;
  }

  assert(endContentPosition == content.size());

  content.push_back(value);
  backtraces.push_back(lfbt);

  endContentPosition = content.size();
}

template <typename T, typename U, typename V>
void SetContent(T& content, U& backtraces, V& endContentPosition,
                const std::string& vec, const cmListFileBacktrace& lfbt)
{
  assert(endContentPosition == content.size());

  content.resize(content.size() + 2);
  backtraces.resize(backtraces.size() + 2);

  content.back() = vec;
  backtraces.back() = lfbt;

  endContentPosition = content.size();
}

template <typename T, typename U, typename V>
void ClearContent(T& content, U& backtraces, V& endContentPosition)
{
  assert(endContentPosition == content.size());

  content.resize(content.size() + 1);
  backtraces.resize(backtraces.size() + 1);

  endContentPosition = content.size();
}

cmStringRange cmState::Directory::GetIncludeDirectoriesEntries() const
{
  return GetPropertyContent(
    this->DirectoryState->IncludeDirectories,
    this->Snapshot_.Position->IncludeDirectoryPosition);
}

cmBacktraceRange cmState::Directory::GetIncludeDirectoriesEntryBacktraces()
  const
{
  return GetPropertyBacktraces(
    this->DirectoryState->IncludeDirectories,
    this->DirectoryState->IncludeDirectoryBacktraces,
    this->Snapshot_.Position->IncludeDirectoryPosition);
}

void cmState::Directory::AppendIncludeDirectoriesEntry(
  const std::string& vec, const cmListFileBacktrace& lfbt)
{
  AppendEntry(this->DirectoryState->IncludeDirectories,
              this->DirectoryState->IncludeDirectoryBacktraces,
              this->Snapshot_.Position->IncludeDirectoryPosition, vec, lfbt);
}

void cmState::Directory::PrependIncludeDirectoriesEntry(
  const std::string& vec, const cmListFileBacktrace& lfbt)
{
  std::vector<std::string>::iterator entryEnd =
    this->DirectoryState->IncludeDirectories.begin() +
    this->Snapshot_.Position->IncludeDirectoryPosition;

  std::vector<std::string>::reverse_iterator rend =
    this->DirectoryState->IncludeDirectories.rend();
  std::vector<std::string>::reverse_iterator rbegin =
    cmMakeReverseIterator(entryEnd);
  rbegin = std::find(rbegin, rend, cmPropertySentinal);

  std::vector<std::string>::iterator entryIt = rbegin.base();
  std::vector<std::string>::iterator entryBegin =
    this->DirectoryState->IncludeDirectories.begin();

  std::vector<cmListFileBacktrace>::iterator btIt =
    this->DirectoryState->IncludeDirectoryBacktraces.begin() +
    std::distance(entryBegin, entryIt);

  this->DirectoryState->IncludeDirectories.insert(entryIt, vec);
  this->DirectoryState->IncludeDirectoryBacktraces.insert(btIt, lfbt);

  this->Snapshot_.Position->IncludeDirectoryPosition =
    this->DirectoryState->IncludeDirectories.size();
}

void cmState::Directory::SetIncludeDirectories(const std::string& vec,
                                               const cmListFileBacktrace& lfbt)
{
  SetContent(this->DirectoryState->IncludeDirectories,
             this->DirectoryState->IncludeDirectoryBacktraces,
             this->Snapshot_.Position->IncludeDirectoryPosition, vec, lfbt);
}

void cmState::Directory::ClearIncludeDirectories()
{
  ClearContent(this->DirectoryState->IncludeDirectories,
               this->DirectoryState->IncludeDirectoryBacktraces,
               this->Snapshot_.Position->IncludeDirectoryPosition);
}

cmStringRange cmState::Directory::GetCompileDefinitionsEntries() const
{
  return GetPropertyContent(
    this->DirectoryState->CompileDefinitions,
    this->Snapshot_.Position->CompileDefinitionsPosition);
}

cmBacktraceRange cmState::Directory::GetCompileDefinitionsEntryBacktraces()
  const
{
  return GetPropertyBacktraces(
    this->DirectoryState->CompileDefinitions,
    this->DirectoryState->CompileDefinitionsBacktraces,
    this->Snapshot_.Position->CompileDefinitionsPosition);
}

void cmState::Directory::AppendCompileDefinitionsEntry(
  const std::string& vec, const cmListFileBacktrace& lfbt)
{
  AppendEntry(this->DirectoryState->CompileDefinitions,
              this->DirectoryState->CompileDefinitionsBacktraces,
              this->Snapshot_.Position->CompileDefinitionsPosition, vec, lfbt);
}

void cmState::Directory::SetCompileDefinitions(const std::string& vec,
                                               const cmListFileBacktrace& lfbt)
{
  SetContent(this->DirectoryState->CompileDefinitions,
             this->DirectoryState->CompileDefinitionsBacktraces,
             this->Snapshot_.Position->CompileDefinitionsPosition, vec, lfbt);
}

void cmState::Directory::ClearCompileDefinitions()
{
  ClearContent(this->DirectoryState->CompileDefinitions,
               this->DirectoryState->CompileDefinitionsBacktraces,
               this->Snapshot_.Position->CompileDefinitionsPosition);
}

cmStringRange cmState::Directory::GetCompileOptionsEntries() const
{
  return GetPropertyContent(this->DirectoryState->CompileOptions,
                            this->Snapshot_.Position->CompileOptionsPosition);
}

cmBacktraceRange cmState::Directory::GetCompileOptionsEntryBacktraces() const
{
  return GetPropertyBacktraces(
    this->DirectoryState->CompileOptions,
    this->DirectoryState->CompileOptionsBacktraces,
    this->Snapshot_.Position->CompileOptionsPosition);
}

void cmState::Directory::AppendCompileOptionsEntry(
  const std::string& vec, const cmListFileBacktrace& lfbt)
{
  AppendEntry(this->DirectoryState->CompileOptions,
              this->DirectoryState->CompileOptionsBacktraces,
              this->Snapshot_.Position->CompileOptionsPosition, vec, lfbt);
}

void cmState::Directory::SetCompileOptions(const std::string& vec,
                                           const cmListFileBacktrace& lfbt)
{
  SetContent(this->DirectoryState->CompileOptions,
             this->DirectoryState->CompileOptionsBacktraces,
             this->Snapshot_.Position->CompileOptionsPosition, vec, lfbt);
}

void cmState::Directory::ClearCompileOptions()
{
  ClearContent(this->DirectoryState->CompileOptions,
               this->DirectoryState->CompileOptionsBacktraces,
               this->Snapshot_.Position->CompileOptionsPosition);
}

bool cmState::Snapshot::StrictWeakOrder::operator()(
  const cmState::Snapshot& lhs, const cmState::Snapshot& rhs) const
{
  return lhs.Position.StrictWeakOrdered(rhs.Position);
}

void cmState::Directory::SetProperty(const std::string& prop,
                                     const char* value,
                                     cmListFileBacktrace const& lfbt)
{
  if (prop == "INCLUDE_DIRECTORIES") {
    if (!value) {
      this->ClearIncludeDirectories();
      return;
    }
    this->SetIncludeDirectories(value, lfbt);
    return;
  }
  if (prop == "COMPILE_OPTIONS") {
    if (!value) {
      this->ClearCompileOptions();
      return;
    }
    this->SetCompileOptions(value, lfbt);
    return;
  }
  if (prop == "COMPILE_DEFINITIONS") {
    if (!value) {
      this->ClearCompileDefinitions();
      return;
    }
    this->SetCompileDefinitions(value, lfbt);
    return;
  }

  this->DirectoryState->Properties.SetProperty(prop, value);
}

void cmState::Directory::AppendProperty(const std::string& prop,
                                        const char* value, bool asString,
                                        cmListFileBacktrace const& lfbt)
{
  if (prop == "INCLUDE_DIRECTORIES") {
    this->AppendIncludeDirectoriesEntry(value, lfbt);
    return;
  }
  if (prop == "COMPILE_OPTIONS") {
    this->AppendCompileOptionsEntry(value, lfbt);
    return;
  }
  if (prop == "COMPILE_DEFINITIONS") {
    this->AppendCompileDefinitionsEntry(value, lfbt);
    return;
  }

  this->DirectoryState->Properties.AppendProperty(prop, value, asString);
}

const char* cmState::Directory::GetProperty(const std::string& prop) const
{
  const bool chain =
    this->Snapshot_.State->IsPropertyChained(prop, cmProperty::DIRECTORY);
  return this->GetProperty(prop, chain);
}

const char* cmState::Directory::GetProperty(const std::string& prop,
                                            bool chain) const
{
  static std::string output;
  output = "";
  if (prop == "PARENT_DIRECTORY") {
    cmState::Snapshot parent = this->Snapshot_.GetBuildsystemDirectoryParent();
    if (parent.IsValid()) {
      return parent.GetDirectory().GetCurrentSource();
    }
    return "";
  } else if (prop == "LISTFILE_STACK") {
    std::vector<std::string> listFiles;
    cmState::Snapshot snp = this->Snapshot_;
    while (snp.IsValid()) {
      listFiles.push_back(snp.GetExecutionListFile());
      snp = snp.GetCallStackParent();
    }
    std::reverse(listFiles.begin(), listFiles.end());
    output = cmJoin(listFiles, ";");
    return output.c_str();
  } else if (prop == "CACHE_VARIABLES") {
    output = cmJoin(this->Snapshot_.State->GetCacheEntryKeys(), ";");
    return output.c_str();
  } else if (prop == "VARIABLES") {
    std::vector<std::string> res = this->Snapshot_.ClosureKeys();
    std::vector<std::string> cacheKeys =
      this->Snapshot_.State->GetCacheEntryKeys();
    res.insert(res.end(), cacheKeys.begin(), cacheKeys.end());
    std::sort(res.begin(), res.end());
    output = cmJoin(res, ";");
    return output.c_str();
  } else if (prop == "INCLUDE_DIRECTORIES") {
    output = cmJoin(this->GetIncludeDirectoriesEntries(), ";");
    return output.c_str();
  } else if (prop == "COMPILE_OPTIONS") {
    output = cmJoin(this->GetCompileOptionsEntries(), ";");
    return output.c_str();
  } else if (prop == "COMPILE_DEFINITIONS") {
    output = cmJoin(this->GetCompileDefinitionsEntries(), ";");
    return output.c_str();
  }

  const char* retVal = this->DirectoryState->Properties.GetPropertyValue(prop);
  if (!retVal && chain) {
    Snapshot parentSnapshot = this->Snapshot_.GetBuildsystemDirectoryParent();
    if (parentSnapshot.IsValid()) {
      return parentSnapshot.GetDirectory().GetProperty(prop, chain);
    }
    return this->Snapshot_.State->GetGlobalProperty(prop);
  }

  return retVal;
}

bool cmState::Directory::GetPropertyAsBool(const std::string& prop) const
{
  return cmSystemTools::IsOn(this->GetProperty(prop));
}

std::vector<std::string> cmState::Directory::GetPropertyKeys() const
{
  std::vector<std::string> keys;
  keys.reserve(this->DirectoryState->Properties.size());
  for (cmPropertyMap::const_iterator it =
         this->DirectoryState->Properties.begin();
       it != this->DirectoryState->Properties.end(); ++it) {
    keys.push_back(it->first);
  }
  return keys;
}

bool operator==(const cmState::Snapshot& lhs, const cmState::Snapshot& rhs)
{
  return lhs.Position == rhs.Position;
}

bool operator!=(const cmState::Snapshot& lhs, const cmState::Snapshot& rhs)
{
  return lhs.Position != rhs.Position;
}

static bool ParseEntryWithoutType(const std::string& entry, std::string& var,
                                  std::string& value)
{
  // input line is:         key=value
  static cmsys::RegularExpression reg(
    "^([^=]*)=(.*[^\r\t ]|[\r\t ]*)[\r\t ]*$");
  // input line is:         "key"=value
  static cmsys::RegularExpression regQuoted(
    "^\"([^\"]*)\"=(.*[^\r\t ]|[\r\t ]*)[\r\t ]*$");
  bool flag = false;
  if (regQuoted.find(entry)) {
    var = regQuoted.match(1);
    value = regQuoted.match(2);
    flag = true;
  } else if (reg.find(entry)) {
    var = reg.match(1);
    value = reg.match(2);
    flag = true;
  }

  // if value is enclosed in single quotes ('foo') then remove them
  // it is used to enclose trailing space or tab
  if (flag && value.size() >= 2 && value[0] == '\'' &&
      value[value.size() - 1] == '\'') {
    value = value.substr(1, value.size() - 2);
  }

  return flag;
}

bool cmState::ParseCacheEntry(const std::string& entry, std::string& var,
                              std::string& value, CacheEntryType& type)
{
  // input line is:         key:type=value
  static cmsys::RegularExpression reg(
    "^([^=:]*):([^=]*)=(.*[^\r\t ]|[\r\t ]*)[\r\t ]*$");
  // input line is:         "key":type=value
  static cmsys::RegularExpression regQuoted(
    "^\"([^\"]*)\":([^=]*)=(.*[^\r\t ]|[\r\t ]*)[\r\t ]*$");
  bool flag = false;
  if (regQuoted.find(entry)) {
    var = regQuoted.match(1);
    type = cmState::StringToCacheEntryType(regQuoted.match(2).c_str());
    value = regQuoted.match(3);
    flag = true;
  } else if (reg.find(entry)) {
    var = reg.match(1);
    type = cmState::StringToCacheEntryType(reg.match(2).c_str());
    value = reg.match(3);
    flag = true;
  }

  // if value is enclosed in single quotes ('foo') then remove them
  // it is used to enclose trailing space or tab
  if (flag && value.size() >= 2 && value[0] == '\'' &&
      value[value.size() - 1] == '\'') {
    value = value.substr(1, value.size() - 2);
  }

  if (!flag) {
    return ParseEntryWithoutType(entry, var, value);
  }

  return flag;
}
