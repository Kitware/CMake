/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmState.h"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdlib>
#include <utility>

#include <cm/memory>

#include "cmsys/RegularExpression.hxx"

#include "cmCacheManager.h"
#include "cmCommand.h"
#include "cmDefinitions.h"
#include "cmExecutionStatus.h"
#include "cmGlobVerificationManager.h"
#include "cmList.h"
#include "cmListFileCache.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmStatePrivate.h"
#include "cmStateSnapshot.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmake.h"

cmState::cmState(Mode mode, ProjectKind projectKind)
  : StateMode(mode)
  , StateProjectKind(projectKind)
{
  this->CacheManager = cm::make_unique<cmCacheManager>();
  this->GlobVerificationManager = cm::make_unique<cmGlobVerificationManager>();
}

cmState::~cmState() = default;

const std::string& cmState::GetTargetTypeName(
  cmStateEnums::TargetType targetType)
{
#define MAKE_STATIC_PROP(PROP) static const std::string prop##PROP = #PROP
  MAKE_STATIC_PROP(STATIC_LIBRARY);
  MAKE_STATIC_PROP(MODULE_LIBRARY);
  MAKE_STATIC_PROP(SHARED_LIBRARY);
  MAKE_STATIC_PROP(OBJECT_LIBRARY);
  MAKE_STATIC_PROP(EXECUTABLE);
  MAKE_STATIC_PROP(UTILITY);
  MAKE_STATIC_PROP(GLOBAL_TARGET);
  MAKE_STATIC_PROP(INTERFACE_LIBRARY);
  MAKE_STATIC_PROP(UNKNOWN_LIBRARY);
  static const std::string propEmpty;
#undef MAKE_STATIC_PROP

  switch (targetType) {
    case cmStateEnums::STATIC_LIBRARY:
      return propSTATIC_LIBRARY;
    case cmStateEnums::MODULE_LIBRARY:
      return propMODULE_LIBRARY;
    case cmStateEnums::SHARED_LIBRARY:
      return propSHARED_LIBRARY;
    case cmStateEnums::OBJECT_LIBRARY:
      return propOBJECT_LIBRARY;
    case cmStateEnums::EXECUTABLE:
      return propEXECUTABLE;
    case cmStateEnums::UTILITY:
      return propUTILITY;
    case cmStateEnums::GLOBAL_TARGET:
      return propGLOBAL_TARGET;
    case cmStateEnums::INTERFACE_LIBRARY:
      return propINTERFACE_LIBRARY;
    case cmStateEnums::UNKNOWN_LIBRARY:
      return propUNKNOWN_LIBRARY;
  }
  assert(false && "Unexpected target type");
  return propEmpty;
}

static const std::array<std::string, 7> cmCacheEntryTypes = {
  { "BOOL", "PATH", "FILEPATH", "STRING", "INTERNAL", "STATIC",
    "UNINITIALIZED" }
};

const std::string& cmState::CacheEntryTypeToString(
  cmStateEnums::CacheEntryType type)
{
  if (type < cmStateEnums::BOOL || type > cmStateEnums::UNINITIALIZED) {
    type = cmStateEnums::UNINITIALIZED;
  }
  return cmCacheEntryTypes[type];
}

cmStateEnums::CacheEntryType cmState::StringToCacheEntryType(
  const std::string& s)
{
  cmStateEnums::CacheEntryType type = cmStateEnums::STRING;
  StringToCacheEntryType(s, type);
  return type;
}

bool cmState::StringToCacheEntryType(const std::string& s,
                                     cmStateEnums::CacheEntryType& type)
{
  // NOLINTNEXTLINE(readability-qualified-auto)
  auto const entry =
    std::find(cmCacheEntryTypes.begin(), cmCacheEntryTypes.end(), s);
  if (entry != cmCacheEntryTypes.end()) {
    type = static_cast<cmStateEnums::CacheEntryType>(
      entry - cmCacheEntryTypes.begin());
    return true;
  }
  return false;
}

bool cmState::IsCacheEntryType(std::string const& key)
{
  return std::any_of(
    cmCacheEntryTypes.begin(), cmCacheEntryTypes.end(),
    [&key](std::string const& i) -> bool { return key == i; });
}

bool cmState::LoadCache(const std::string& path, bool internal,
                        std::set<std::string>& excludes,
                        std::set<std::string>& includes)
{
  return this->CacheManager->LoadCache(path, internal, excludes, includes);
}

bool cmState::SaveCache(const std::string& path, cmMessenger* messenger)
{
  return this->CacheManager->SaveCache(path, messenger);
}

bool cmState::DeleteCache(const std::string& path)
{
  return this->CacheManager->DeleteCache(path);
}

bool cmState::IsCacheLoaded() const
{
  return this->CacheManager->IsCacheLoaded();
}

std::vector<std::string> cmState::GetCacheEntryKeys() const
{
  return this->CacheManager->GetCacheEntryKeys();
}

cmValue cmState::GetCacheEntryValue(std::string const& key) const
{
  return this->CacheManager->GetCacheEntryValue(key);
}

std::string cmState::GetSafeCacheEntryValue(std::string const& key) const
{
  if (cmValue val = this->GetCacheEntryValue(key)) {
    return *val;
  }
  return std::string();
}

cmValue cmState::GetInitializedCacheValue(std::string const& key) const
{
  return this->CacheManager->GetInitializedCacheValue(key);
}

cmStateEnums::CacheEntryType cmState::GetCacheEntryType(
  std::string const& key) const
{
  return this->CacheManager->GetCacheEntryType(key);
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
  this->CacheManager->SetCacheEntryProperty(key, propertyName, value);
}

void cmState::SetCacheEntryBoolProperty(std::string const& key,
                                        std::string const& propertyName,
                                        bool value)
{
  this->CacheManager->SetCacheEntryBoolProperty(key, propertyName, value);
}

std::vector<std::string> cmState::GetCacheEntryPropertyList(
  const std::string& key)
{
  return this->CacheManager->GetCacheEntryPropertyList(key);
}

cmValue cmState::GetCacheEntryProperty(std::string const& key,
                                       std::string const& propertyName)
{
  return this->CacheManager->GetCacheEntryProperty(key, propertyName);
}

bool cmState::GetCacheEntryPropertyAsBool(std::string const& key,
                                          std::string const& propertyName)
{
  return this->CacheManager->GetCacheEntryPropertyAsBool(key, propertyName);
}

void cmState::AddCacheEntry(const std::string& key, cmValue value,
                            const std::string& helpString,
                            cmStateEnums::CacheEntryType type)
{
  this->CacheManager->AddCacheEntry(key, value, helpString, type);
}

bool cmState::DoWriteGlobVerifyTarget() const
{
  return this->GlobVerificationManager->DoWriteVerifyTarget();
}

std::string const& cmState::GetGlobVerifyScript() const
{
  return this->GlobVerificationManager->GetVerifyScript();
}

std::string const& cmState::GetGlobVerifyStamp() const
{
  return this->GlobVerificationManager->GetVerifyStamp();
}

bool cmState::SaveVerificationScript(const std::string& path,
                                     cmMessenger* messenger)
{
  return this->GlobVerificationManager->SaveVerificationScript(path,
                                                               messenger);
}

void cmState::AddGlobCacheEntry(
  bool recurse, bool listDirectories, bool followSymlinks,
  const std::string& relative, const std::string& expression,
  const std::vector<std::string>& files, const std::string& variable,
  cmListFileBacktrace const& backtrace, cmMessenger* messenger)
{
  this->GlobVerificationManager->AddCacheEntry(
    recurse, listDirectories, followSymlinks, relative, expression, files,
    variable, backtrace, messenger);
}

void cmState::RemoveCacheEntry(std::string const& key)
{
  this->CacheManager->RemoveCacheEntry(key);
}

void cmState::AppendCacheEntryProperty(const std::string& key,
                                       const std::string& property,
                                       const std::string& value, bool asString)
{
  this->CacheManager->AppendCacheEntryProperty(key, property, value, asString);
}

void cmState::RemoveCacheEntryProperty(std::string const& key,
                                       std::string const& propertyName)
{
  this->CacheManager->RemoveCacheEntryProperty(key, propertyName);
}

cmStateSnapshot cmState::Reset()
{
  this->GlobalProperties.Clear();
  this->PropertyDefinitions = {};
  this->GlobVerificationManager->Reset();

  cmStateDetail::PositionType pos = this->SnapshotData.Truncate();
  this->ExecutionListFiles.Truncate();

  {
    cmLinkedTree<cmStateDetail::BuildsystemDirectoryStateType>::iterator it =
      this->BuildsystemDirectory.Truncate();
    it->IncludeDirectories.clear();
    it->CompileDefinitions.clear();
    it->CompileOptions.clear();
    it->LinkOptions.clear();
    it->LinkDirectories.clear();
    it->CurrentScope = pos;
    it->NormalTargetNames.clear();
    it->ImportedTargetNames.clear();
    it->Properties.Clear();
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
      *cmDefinitions::Get("CMAKE_SOURCE_DIR", pos->Vars, pos->Root);
    std::string binDir =
      *cmDefinitions::Get("CMAKE_BINARY_DIR", pos->Vars, pos->Root);
    this->VarTree.Clear();
    pos->Vars = this->VarTree.Push(this->VarTree.Root());
    pos->Parent = this->VarTree.Root();
    pos->Root = this->VarTree.Root();

    pos->Vars->Set("CMAKE_SOURCE_DIR", srcDir);
    pos->Vars->Set("CMAKE_BINARY_DIR", binDir);
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

  return { this, pos };
}

void cmState::DefineProperty(const std::string& name,
                             cmProperty::ScopeType scope,
                             const std::string& ShortDescription,
                             const std::string& FullDescription, bool chained,
                             const std::string& initializeFromVariable)
{
  this->PropertyDefinitions.DefineProperty(name, scope, ShortDescription,
                                           FullDescription, chained,
                                           initializeFromVariable);
}

cmPropertyDefinition const* cmState::GetPropertyDefinition(
  const std::string& name, cmProperty::ScopeType scope) const
{
  return this->PropertyDefinitions.GetPropertyDefinition(name, scope);
}

bool cmState::IsPropertyChained(const std::string& name,
                                cmProperty::ScopeType scope) const
{
  if (const auto* def = this->GetPropertyDefinition(name, scope)) {
    return def->IsChained();
  }
  return false;
}

void cmState::SetLanguageEnabled(std::string const& l)
{
  auto it = std::lower_bound(this->EnabledLanguages.begin(),
                             this->EnabledLanguages.end(), l);
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

bool cmState::GetIsGeneratorMultiConfig() const
{
  return this->IsGeneratorMultiConfig;
}

void cmState::SetIsGeneratorMultiConfig(bool b)
{
  this->IsGeneratorMultiConfig = b;
}

void cmState::AddBuiltinCommand(std::string const& name,
                                std::unique_ptr<cmCommand> command)
{
  this->AddBuiltinCommand(name, cmLegacyCommandWrapper(std::move(command)));
}

void cmState::AddBuiltinCommand(std::string const& name, Command command)
{
  assert(name == cmSystemTools::LowerCase(name));
  assert(this->BuiltinCommands.find(name) == this->BuiltinCommands.end());
  this->BuiltinCommands.emplace(name, std::move(command));
}

static bool InvokeBuiltinCommand(cmState::BuiltinCommand command,
                                 std::vector<cmListFileArgument> const& args,
                                 cmExecutionStatus& status)
{
  cmMakefile& mf = status.GetMakefile();
  std::vector<std::string> expandedArguments;
  if (!mf.ExpandArguments(args, expandedArguments)) {
    // There was an error expanding arguments.  It was already
    // reported, so we can skip this command without error.
    return true;
  }
  return command(expandedArguments, status);
}

void cmState::AddBuiltinCommand(std::string const& name,
                                BuiltinCommand command)
{
  this->AddBuiltinCommand(
    name,
    [command](const std::vector<cmListFileArgument>& args,
              cmExecutionStatus& status) -> bool {
      return InvokeBuiltinCommand(command, args, status);
    });
}

void cmState::AddFlowControlCommand(std::string const& name, Command command)
{
  this->FlowControlCommands.insert(name);
  this->AddBuiltinCommand(name, std::move(command));
}

void cmState::AddFlowControlCommand(std::string const& name,
                                    BuiltinCommand command)
{
  this->FlowControlCommands.insert(name);
  this->AddBuiltinCommand(name, command);
}

void cmState::AddDisallowedCommand(std::string const& name,
                                   BuiltinCommand command,
                                   cmPolicies::PolicyID policy,
                                   const char* message)
{
  this->AddBuiltinCommand(
    name,
    [command, policy, message](const std::vector<cmListFileArgument>& args,
                               cmExecutionStatus& status) -> bool {
      cmMakefile& mf = status.GetMakefile();
      switch (mf.GetPolicyStatus(policy)) {
        case cmPolicies::WARN:
          mf.IssueMessage(MessageType::AUTHOR_WARNING,
                          cmPolicies::GetPolicyWarning(policy));
          CM_FALLTHROUGH;
        case cmPolicies::OLD:
          break;
        case cmPolicies::REQUIRED_IF_USED:
        case cmPolicies::REQUIRED_ALWAYS:
        case cmPolicies::NEW:
          mf.IssueMessage(MessageType::FATAL_ERROR, message);
          return true;
      }
      return InvokeBuiltinCommand(command, args, status);
    });
}

void cmState::AddUnexpectedCommand(std::string const& name, const char* error)
{
  this->AddBuiltinCommand(
    name,
    [name, error](std::vector<cmListFileArgument> const&,
                  cmExecutionStatus& status) -> bool {
      cmValue versionValue =
        status.GetMakefile().GetDefinition("CMAKE_MINIMUM_REQUIRED_VERSION");
      if (name == "endif" &&
          (!versionValue || atof(versionValue->c_str()) <= 1.4)) {
        return true;
      }
      status.SetError(error);
      return false;
    });
}

void cmState::AddUnexpectedFlowControlCommand(std::string const& name,
                                              const char* error)
{
  this->FlowControlCommands.insert(name);
  this->AddUnexpectedCommand(name, error);
}

bool cmState::AddScriptedCommand(std::string const& name, BT<Command> command,
                                 cmMakefile& mf)
{
  std::string sName = cmSystemTools::LowerCase(name);

  if (this->FlowControlCommands.count(sName)) {
    mf.GetCMakeInstance()->IssueMessage(
      MessageType::FATAL_ERROR,
      cmStrCat("Built-in flow control command \"", sName,
               "\" cannot be overridden."),
      command.Backtrace);
    cmSystemTools::SetFatalErrorOccurred();
    return false;
  }

  // if the command already exists, give a new name to the old command.
  if (Command oldCmd = this->GetCommandByExactName(sName)) {
    this->ScriptedCommands["_" + sName] = oldCmd;
  }

  this->ScriptedCommands[sName] = std::move(command.Value);
  return true;
}

cmState::Command cmState::GetCommand(std::string const& name) const
{
  return this->GetCommandByExactName(cmSystemTools::LowerCase(name));
}

cmState::Command cmState::GetCommandByExactName(std::string const& name) const
{
  auto pos = this->ScriptedCommands.find(name);
  if (pos != this->ScriptedCommands.end()) {
    return pos->second;
  }
  pos = this->BuiltinCommands.find(name);
  if (pos != this->BuiltinCommands.end()) {
    return pos->second;
  }
  return nullptr;
}

std::vector<std::string> cmState::GetCommandNames() const
{
  std::vector<std::string> commandNames;
  commandNames.reserve(this->BuiltinCommands.size() +
                       this->ScriptedCommands.size());
  for (auto const& bc : this->BuiltinCommands) {
    commandNames.push_back(bc.first);
  }
  for (auto const& sc : this->ScriptedCommands) {
    commandNames.push_back(sc.first);
  }
  std::sort(commandNames.begin(), commandNames.end());
  commandNames.erase(std::unique(commandNames.begin(), commandNames.end()),
                     commandNames.end());
  return commandNames;
}

void cmState::RemoveBuiltinCommand(std::string const& name)
{
  assert(name == cmSystemTools::LowerCase(name));
  this->BuiltinCommands.erase(name);
}

void cmState::RemoveUserDefinedCommands()
{
  this->ScriptedCommands.clear();
}

void cmState::SetGlobalProperty(const std::string& prop,
                                const std::string& value)
{
  this->GlobalProperties.SetProperty(prop, value);
}
void cmState::SetGlobalProperty(const std::string& prop, cmValue value)
{
  this->GlobalProperties.SetProperty(prop, value);
}

void cmState::AppendGlobalProperty(const std::string& prop,
                                   const std::string& value, bool asString)
{
  this->GlobalProperties.AppendProperty(prop, value, asString);
}

cmValue cmState::GetGlobalProperty(const std::string& prop)
{
  if (prop == "CACHE_VARIABLES") {
    std::vector<std::string> cacheKeys = this->GetCacheEntryKeys();
    this->SetGlobalProperty("CACHE_VARIABLES", cmList::to_string(cacheKeys));
  } else if (prop == "COMMANDS") {
    std::vector<std::string> commands = this->GetCommandNames();
    this->SetGlobalProperty("COMMANDS", cmList::to_string(commands));
  } else if (prop == "IN_TRY_COMPILE") {
    this->SetGlobalProperty(
      "IN_TRY_COMPILE",
      this->StateProjectKind == ProjectKind::TryCompile ? "1" : "0");
  } else if (prop == "GENERATOR_IS_MULTI_CONFIG") {
    this->SetGlobalProperty("GENERATOR_IS_MULTI_CONFIG",
                            this->IsGeneratorMultiConfig ? "1" : "0");
  } else if (prop == "ENABLED_LANGUAGES") {
    auto langs = cmList::to_string(this->EnabledLanguages);
    this->SetGlobalProperty("ENABLED_LANGUAGES", langs);
  } else if (prop == "CMAKE_ROLE") {
    std::string mode = this->GetModeString();
    this->SetGlobalProperty("CMAKE_ROLE", mode);
  }
#define STRING_LIST_ELEMENT(F) ";" #F
  if (prop == "CMAKE_C_KNOWN_FEATURES") {
    static const std::string s_out(
      &FOR_EACH_C_FEATURE(STRING_LIST_ELEMENT)[1]);
    return cmValue(s_out);
  }
  if (prop == "CMAKE_C90_KNOWN_FEATURES") {
    static const std::string s_out(
      &FOR_EACH_C90_FEATURE(STRING_LIST_ELEMENT)[1]);
    return cmValue(s_out);
  }
  if (prop == "CMAKE_C99_KNOWN_FEATURES") {
    static const std::string s_out(
      &FOR_EACH_C99_FEATURE(STRING_LIST_ELEMENT)[1]);
    return cmValue(s_out);
  }
  if (prop == "CMAKE_C11_KNOWN_FEATURES") {
    static const std::string s_out(
      &FOR_EACH_C11_FEATURE(STRING_LIST_ELEMENT)[1]);
    return cmValue(s_out);
  }
  if (prop == "CMAKE_CXX_KNOWN_FEATURES") {
    static const std::string s_out(
      &FOR_EACH_CXX_FEATURE(STRING_LIST_ELEMENT)[1]);
    return cmValue(s_out);
  }
  if (prop == "CMAKE_CXX98_KNOWN_FEATURES") {
    static const std::string s_out(
      &FOR_EACH_CXX98_FEATURE(STRING_LIST_ELEMENT)[1]);
    return cmValue(s_out);
  }
  if (prop == "CMAKE_CXX11_KNOWN_FEATURES") {
    static const std::string s_out(
      &FOR_EACH_CXX11_FEATURE(STRING_LIST_ELEMENT)[1]);
    return cmValue(s_out);
  }
  if (prop == "CMAKE_CXX14_KNOWN_FEATURES") {
    static const std::string s_out(
      &FOR_EACH_CXX14_FEATURE(STRING_LIST_ELEMENT)[1]);
    return cmValue(s_out);
  }
  if (prop == "CMAKE_CUDA_KNOWN_FEATURES") {
    static const std::string s_out(
      &FOR_EACH_CUDA_FEATURE(STRING_LIST_ELEMENT)[1]);
    return cmValue(s_out);
  }

#undef STRING_LIST_ELEMENT
  return this->GlobalProperties.GetPropertyValue(prop);
}

bool cmState::GetGlobalPropertyAsBool(const std::string& prop)
{
  return cmIsOn(this->GetGlobalProperty(prop));
}

void cmState::SetSourceDirectory(std::string const& sourceDirectory)
{
  this->SourceDirectory = sourceDirectory;
  cmSystemTools::ConvertToUnixSlashes(this->SourceDirectory);
}

std::string const& cmState::GetSourceDirectory() const
{
  return this->SourceDirectory;
}

void cmState::SetBinaryDirectory(std::string const& binaryDirectory)
{
  this->BinaryDirectory = binaryDirectory;
  cmSystemTools::ConvertToUnixSlashes(this->BinaryDirectory);
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

void cmState::SetGhsMultiIDE(bool ghsMultiIDE)
{
  this->GhsMultiIDE = ghsMultiIDE;
}

bool cmState::UseGhsMultiIDE() const
{
  return this->GhsMultiIDE;
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

void cmState::SetNinjaMulti(bool ninjaMulti)
{
  this->NinjaMulti = ninjaMulti;
}

bool cmState::UseNinjaMulti() const
{
  return this->NinjaMulti;
}

unsigned int cmState::GetCacheMajorVersion() const
{
  return this->CacheManager->GetCacheMajorVersion();
}

unsigned int cmState::GetCacheMinorVersion() const
{
  return this->CacheManager->GetCacheMinorVersion();
}

cmState::Mode cmState::GetMode() const
{
  return this->StateMode;
}

std::string cmState::GetModeString() const
{
  return ModeToString(this->StateMode);
}

std::string cmState::ModeToString(cmState::Mode mode)
{
  switch (mode) {
    case Project:
      return "PROJECT";
    case Script:
      return "SCRIPT";
    case FindPackage:
      return "FIND_PACKAGE";
    case CTest:
      return "CTEST";
    case CPack:
      return "CPACK";
    case Help:
      return "HELP";
    case Unknown:
      return "UNKNOWN";
  }
  return "UNKNOWN";
}

cmState::ProjectKind cmState::GetProjectKind() const
{
  return this->StateProjectKind;
}

std::string const& cmState::GetBinaryDirectory() const
{
  return this->BinaryDirectory;
}

cmStateSnapshot cmState::CreateBaseSnapshot()
{
  cmStateDetail::PositionType pos =
    this->SnapshotData.Push(this->SnapshotData.Root());
  pos->DirectoryParent = this->SnapshotData.Root();
  pos->ScopeParent = this->SnapshotData.Root();
  pos->SnapshotType = cmStateEnums::BaseType;
  pos->Keep = true;
  pos->BuildSystemDirectory =
    this->BuildsystemDirectory.Push(this->BuildsystemDirectory.Root());
  pos->ExecutionListFile =
    this->ExecutionListFiles.Push(this->ExecutionListFiles.Root());
  pos->IncludeDirectoryPosition = 0;
  pos->CompileDefinitionsPosition = 0;
  pos->CompileOptionsPosition = 0;
  pos->LinkOptionsPosition = 0;
  pos->LinkDirectoriesPosition = 0;
  pos->BuildSystemDirectory->CurrentScope = pos;
  pos->Policies = this->PolicyStack.Root();
  pos->PolicyRoot = this->PolicyStack.Root();
  pos->PolicyScope = this->PolicyStack.Root();
  assert(pos->Policies.IsValid());
  assert(pos->PolicyRoot.IsValid());
  pos->Vars = this->VarTree.Push(this->VarTree.Root());
  assert(pos->Vars.IsValid());
  pos->Parent = this->VarTree.Root();
  pos->Root = this->VarTree.Root();
  return { this, pos };
}

cmStateSnapshot cmState::CreateBuildsystemDirectorySnapshot(
  cmStateSnapshot const& originSnapshot)
{
  assert(originSnapshot.IsValid());
  cmStateDetail::PositionType pos =
    this->SnapshotData.Push(originSnapshot.Position);
  pos->DirectoryParent = originSnapshot.Position;
  pos->ScopeParent = originSnapshot.Position;
  pos->SnapshotType = cmStateEnums::BuildsystemDirectoryType;
  pos->Keep = true;
  pos->BuildSystemDirectory = this->BuildsystemDirectory.Push(
    originSnapshot.Position->BuildSystemDirectory);
  pos->ExecutionListFile =
    this->ExecutionListFiles.Push(originSnapshot.Position->ExecutionListFile);
  pos->BuildSystemDirectory->CurrentScope = pos;
  pos->Policies = originSnapshot.Position->Policies;
  pos->PolicyRoot = originSnapshot.Position->Policies;
  pos->PolicyScope = originSnapshot.Position->Policies;
  assert(pos->Policies.IsValid());
  assert(pos->PolicyRoot.IsValid());

  cmLinkedTree<cmDefinitions>::iterator origin = originSnapshot.Position->Vars;
  pos->Parent = origin;
  pos->Root = origin;
  pos->Vars = this->VarTree.Push(origin);

  cmStateSnapshot snapshot = cmStateSnapshot(this, pos);
  originSnapshot.Position->BuildSystemDirectory->Children.push_back(snapshot);
  snapshot.SetDefaultDefinitions();
  snapshot.InitializeFromParent();
  snapshot.SetDirectoryDefinitions();
  return snapshot;
}

cmStateSnapshot cmState::CreateDeferCallSnapshot(
  cmStateSnapshot const& originSnapshot, std::string const& fileName)
{
  cmStateDetail::PositionType pos =
    this->SnapshotData.Push(originSnapshot.Position, *originSnapshot.Position);
  pos->SnapshotType = cmStateEnums::DeferCallType;
  pos->Keep = false;
  pos->ExecutionListFile = this->ExecutionListFiles.Push(
    originSnapshot.Position->ExecutionListFile, fileName);
  assert(originSnapshot.Position->Vars.IsValid());
  pos->BuildSystemDirectory->CurrentScope = pos;
  pos->PolicyScope = originSnapshot.Position->Policies;
  return { this, pos };
}

cmStateSnapshot cmState::CreateFunctionCallSnapshot(
  cmStateSnapshot const& originSnapshot, std::string const& fileName)
{
  cmStateDetail::PositionType pos =
    this->SnapshotData.Push(originSnapshot.Position, *originSnapshot.Position);
  pos->ScopeParent = originSnapshot.Position;
  pos->SnapshotType = cmStateEnums::FunctionCallType;
  pos->Keep = false;
  pos->ExecutionListFile = this->ExecutionListFiles.Push(
    originSnapshot.Position->ExecutionListFile, fileName);
  pos->BuildSystemDirectory->CurrentScope = pos;
  pos->PolicyScope = originSnapshot.Position->Policies;
  assert(originSnapshot.Position->Vars.IsValid());
  cmLinkedTree<cmDefinitions>::iterator origin = originSnapshot.Position->Vars;
  pos->Parent = origin;
  pos->Vars = this->VarTree.Push(origin);
  return { this, pos };
}

cmStateSnapshot cmState::CreateMacroCallSnapshot(
  cmStateSnapshot const& originSnapshot, std::string const& fileName)
{
  cmStateDetail::PositionType pos =
    this->SnapshotData.Push(originSnapshot.Position, *originSnapshot.Position);
  pos->SnapshotType = cmStateEnums::MacroCallType;
  pos->Keep = false;
  pos->ExecutionListFile = this->ExecutionListFiles.Push(
    originSnapshot.Position->ExecutionListFile, fileName);
  assert(originSnapshot.Position->Vars.IsValid());
  pos->BuildSystemDirectory->CurrentScope = pos;
  pos->PolicyScope = originSnapshot.Position->Policies;
  return { this, pos };
}

cmStateSnapshot cmState::CreateIncludeFileSnapshot(
  cmStateSnapshot const& originSnapshot, std::string const& fileName)
{
  cmStateDetail::PositionType pos =
    this->SnapshotData.Push(originSnapshot.Position, *originSnapshot.Position);
  pos->SnapshotType = cmStateEnums::IncludeFileType;
  pos->Keep = true;
  pos->ExecutionListFile = this->ExecutionListFiles.Push(
    originSnapshot.Position->ExecutionListFile, fileName);
  assert(originSnapshot.Position->Vars.IsValid());
  pos->BuildSystemDirectory->CurrentScope = pos;
  pos->PolicyScope = originSnapshot.Position->Policies;
  return { this, pos };
}

cmStateSnapshot cmState::CreateVariableScopeSnapshot(
  cmStateSnapshot const& originSnapshot)
{
  cmStateDetail::PositionType pos =
    this->SnapshotData.Push(originSnapshot.Position, *originSnapshot.Position);
  pos->ScopeParent = originSnapshot.Position;
  pos->SnapshotType = cmStateEnums::VariableScopeType;
  pos->Keep = false;
  pos->BuildSystemDirectory->CurrentScope = pos;
  pos->PolicyScope = originSnapshot.Position->Policies;
  assert(originSnapshot.Position->Vars.IsValid());

  cmLinkedTree<cmDefinitions>::iterator origin = originSnapshot.Position->Vars;
  pos->Parent = origin;
  pos->Vars = this->VarTree.Push(origin);
  assert(pos->Vars.IsValid());
  return { this, pos };
}

cmStateSnapshot cmState::CreateInlineListFileSnapshot(
  cmStateSnapshot const& originSnapshot, std::string const& fileName)
{
  cmStateDetail::PositionType pos =
    this->SnapshotData.Push(originSnapshot.Position, *originSnapshot.Position);
  pos->SnapshotType = cmStateEnums::InlineListFileType;
  pos->Keep = true;
  pos->ExecutionListFile = this->ExecutionListFiles.Push(
    originSnapshot.Position->ExecutionListFile, fileName);
  pos->BuildSystemDirectory->CurrentScope = pos;
  pos->PolicyScope = originSnapshot.Position->Policies;
  return { this, pos };
}

cmStateSnapshot cmState::CreatePolicyScopeSnapshot(
  cmStateSnapshot const& originSnapshot)
{
  cmStateDetail::PositionType pos =
    this->SnapshotData.Push(originSnapshot.Position, *originSnapshot.Position);
  pos->SnapshotType = cmStateEnums::PolicyScopeType;
  pos->Keep = false;
  pos->BuildSystemDirectory->CurrentScope = pos;
  pos->PolicyScope = originSnapshot.Position->Policies;
  return { this, pos };
}

cmStateSnapshot cmState::Pop(cmStateSnapshot const& originSnapshot)
{
  cmStateDetail::PositionType pos = originSnapshot.Position;
  cmStateDetail::PositionType prevPos = pos;
  ++prevPos;
  prevPos->IncludeDirectoryPosition =
    prevPos->BuildSystemDirectory->IncludeDirectories.size();
  prevPos->CompileDefinitionsPosition =
    prevPos->BuildSystemDirectory->CompileDefinitions.size();
  prevPos->CompileOptionsPosition =
    prevPos->BuildSystemDirectory->CompileOptions.size();
  prevPos->LinkOptionsPosition =
    prevPos->BuildSystemDirectory->LinkOptions.size();
  prevPos->LinkDirectoriesPosition =
    prevPos->BuildSystemDirectory->LinkDirectories.size();
  prevPos->BuildSystemDirectory->CurrentScope = prevPos;

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

  return { this, prevPos };
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
  if (flag && value.size() >= 2 && value.front() == '\'' &&
      value.back() == '\'') {
    value = value.substr(1, value.size() - 2);
  }

  return flag;
}

bool cmState::ParseCacheEntry(const std::string& entry, std::string& var,
                              std::string& value,
                              cmStateEnums::CacheEntryType& type)
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
    type = cmState::StringToCacheEntryType(regQuoted.match(2));
    value = regQuoted.match(3);
    flag = true;
  } else if (reg.find(entry)) {
    var = reg.match(1);
    type = cmState::StringToCacheEntryType(reg.match(2));
    value = reg.match(3);
    flag = true;
  }

  // if value is enclosed in single quotes ('foo') then remove them
  // it is used to enclose trailing space or tab
  if (flag && value.size() >= 2 && value.front() == '\'' &&
      value.back() == '\'') {
    value = value.substr(1, value.size() - 2);
  }

  if (!flag) {
    return ParseEntryWithoutType(entry, var, value);
  }

  return flag;
}

cmState::Command cmState::GetDependencyProviderCommand(
  cmDependencyProvider::Method method) const
{
  return (this->DependencyProvider &&
          this->DependencyProvider->SupportsMethod(method))
    ? this->GetCommand(this->DependencyProvider->GetCommand())
    : Command{};
}
