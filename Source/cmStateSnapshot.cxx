/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */

#include "cmStateSnapshot.h"

#include <algorithm>
#include <cassert>
#include <string>

#include <cm/iterator>

#include "cmDefinitions.h"
#include "cmListFileCache.h"
#include "cmProperty.h"
#include "cmPropertyMap.h"
#include "cmState.h"
#include "cmStateDirectory.h"
#include "cmStatePrivate.h"
#include "cmVersion.h"

#if !defined(_WIN32)
#  include <sys/utsname.h>
#endif

#if defined(__CYGWIN__)
#  include "cmSystemTools.h"
#endif

cmStateSnapshot::cmStateSnapshot(cmState* state)
  : State(state)
{
}

std::vector<cmStateSnapshot> cmStateSnapshot::GetChildren()
{
  return this->Position->BuildSystemDirectory->Children;
}

cmStateSnapshot::cmStateSnapshot(cmState* state,
                                 cmStateDetail::PositionType position)
  : State(state)
  , Position(position)
{
}

cmStateEnums::SnapshotType cmStateSnapshot::GetType() const
{
  return this->Position->SnapshotType;
}

void cmStateSnapshot::SetListFile(const std::string& listfile)
{
  *this->Position->ExecutionListFile = listfile;
}

std::string const& cmStateSnapshot::GetExecutionListFile() const
{
  return *this->Position->ExecutionListFile;
}

bool cmStateSnapshot::IsValid() const
{
  return this->State && this->Position.IsValid()
    ? this->Position != this->State->SnapshotData.Root()
    : false;
}

cmStateSnapshot cmStateSnapshot::GetBuildsystemDirectory() const
{
  return { this->State, this->Position->BuildSystemDirectory->DirectoryEnd };
}

cmStateSnapshot cmStateSnapshot::GetBuildsystemDirectoryParent() const
{
  cmStateSnapshot snapshot;
  if (!this->State || this->Position == this->State->SnapshotData.Root()) {
    return snapshot;
  }
  cmStateDetail::PositionType parentPos = this->Position->DirectoryParent;
  if (parentPos != this->State->SnapshotData.Root()) {
    snapshot = cmStateSnapshot(this->State,
                               parentPos->BuildSystemDirectory->DirectoryEnd);
  }

  return snapshot;
}

cmStateSnapshot cmStateSnapshot::GetCallStackParent() const
{
  assert(this->State);
  assert(this->Position != this->State->SnapshotData.Root());

  cmStateSnapshot snapshot;
  cmStateDetail::PositionType parentPos = this->Position;
  while (parentPos->SnapshotType == cmStateEnums::PolicyScopeType ||
         parentPos->SnapshotType == cmStateEnums::VariableScopeType) {
    ++parentPos;
  }
  if (parentPos->SnapshotType == cmStateEnums::BuildsystemDirectoryType ||
      parentPos->SnapshotType == cmStateEnums::BaseType) {
    return snapshot;
  }

  ++parentPos;
  while (parentPos->SnapshotType == cmStateEnums::PolicyScopeType ||
         parentPos->SnapshotType == cmStateEnums::VariableScopeType) {
    ++parentPos;
  }

  if (parentPos == this->State->SnapshotData.Root()) {
    return snapshot;
  }

  snapshot = cmStateSnapshot(this->State, parentPos);
  return snapshot;
}

cmStateSnapshot cmStateSnapshot::GetCallStackBottom() const
{
  assert(this->State);
  assert(this->Position != this->State->SnapshotData.Root());

  cmStateDetail::PositionType pos = this->Position;
  while (pos->SnapshotType != cmStateEnums::BaseType &&
         pos->SnapshotType != cmStateEnums::BuildsystemDirectoryType &&
         pos != this->State->SnapshotData.Root()) {
    ++pos;
  }
  return { this->State, pos };
}

void cmStateSnapshot::PushPolicy(cmPolicies::PolicyMap const& entry, bool weak)
{
  cmStateDetail::PositionType pos = this->Position;
  pos->Policies = this->State->PolicyStack.Push(
    pos->Policies, cmStateDetail::PolicyStackEntry(entry, weak));
}

bool cmStateSnapshot::PopPolicy()
{
  cmStateDetail::PositionType pos = this->Position;
  if (pos->Policies == pos->PolicyScope) {
    return false;
  }
  pos->Policies = this->State->PolicyStack.Pop(pos->Policies);
  return true;
}

bool cmStateSnapshot::CanPopPolicyScope()
{
  return this->Position->Policies != this->Position->PolicyScope;
}

void cmStateSnapshot::SetPolicy(cmPolicies::PolicyID id,
                                cmPolicies::PolicyStatus status)
{
  // Update the policy stack from the top to the top-most strong entry.
  bool previous_was_weak = true;
  for (cmLinkedTree<cmStateDetail::PolicyStackEntry>::iterator psi =
         this->Position->Policies;
       previous_was_weak && psi != this->Position->PolicyRoot; ++psi) {
    psi->Set(id, status);
    previous_was_weak = psi->Weak;
  }
}

cmPolicies::PolicyStatus cmStateSnapshot::GetPolicy(cmPolicies::PolicyID id,
                                                    bool parent_scope) const
{
  cmPolicies::PolicyStatus status = cmPolicies::GetPolicyStatus(id);

  if (status == cmPolicies::REQUIRED_ALWAYS ||
      status == cmPolicies::REQUIRED_IF_USED) {
    return status;
  }

  cmLinkedTree<cmStateDetail::BuildsystemDirectoryStateType>::iterator dir =
    this->Position->BuildSystemDirectory;

  while (true) {
    assert(dir.IsValid());
    cmLinkedTree<cmStateDetail::PolicyStackEntry>::iterator leaf =
      dir->DirectoryEnd->Policies;
    cmLinkedTree<cmStateDetail::PolicyStackEntry>::iterator root =
      dir->DirectoryEnd->PolicyRoot;
    for (; leaf != root; ++leaf) {
      if (parent_scope) {
        parent_scope = false;
        continue;
      }
      if (leaf->IsDefined(id)) {
        status = leaf->Get(id);
        return status;
      }
    }
    cmStateDetail::PositionType e = dir->DirectoryEnd;
    cmStateDetail::PositionType p = e->DirectoryParent;
    if (p == this->State->SnapshotData.Root()) {
      break;
    }
    dir = p->BuildSystemDirectory;
  }
  return status;
}

bool cmStateSnapshot::HasDefinedPolicyCMP0011()
{
  return !this->Position->Policies->IsEmpty();
}

std::string const* cmStateSnapshot::GetDefinition(
  std::string const& name) const
{
  assert(this->Position->Vars.IsValid());
  return cmDefinitions::Get(name, this->Position->Vars, this->Position->Root);
}

bool cmStateSnapshot::IsInitialized(std::string const& name) const
{
  return cmDefinitions::HasKey(name, this->Position->Vars,
                               this->Position->Root);
}

void cmStateSnapshot::SetDefinition(std::string const& name,
                                    cm::string_view value)
{
  this->Position->Vars->Set(name, value);
}

void cmStateSnapshot::RemoveDefinition(std::string const& name)
{
  this->Position->Vars->Unset(name);
}

std::vector<std::string> cmStateSnapshot::ClosureKeys() const
{
  return cmDefinitions::ClosureKeys(this->Position->Vars,
                                    this->Position->Root);
}

bool cmStateSnapshot::RaiseScope(std::string const& var, const char* varDef)
{
  if (this->Position->ScopeParent == this->Position->DirectoryParent) {
    cmStateSnapshot parentDir = this->GetBuildsystemDirectoryParent();
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
  if (varDef) {
    this->Position->Parent->Set(var, varDef);
  } else {
    this->Position->Parent->Unset(var);
  }
  return true;
}

template <typename T, typename U, typename V>
void InitializeContentFromParent(T& parentContent, T& thisContent,
                                 U& parentBacktraces, U& thisBacktraces,
                                 V& contentEndPosition)
{
  auto parentBegin = parentContent.begin();
  auto parentEnd = parentContent.end();

  auto parentRbegin = cm::make_reverse_iterator(parentEnd);
  auto parentRend = parentContent.rend();
  parentRbegin = std::find(parentRbegin, parentRend, cmPropertySentinal);
  auto parentIt = parentRbegin.base();

  thisContent = std::vector<std::string>(parentIt, parentEnd);

  auto btIt = parentBacktraces.begin() + std::distance(parentBegin, parentIt);
  auto btEnd = parentBacktraces.end();

  thisBacktraces = std::vector<cmListFileBacktrace>(btIt, btEnd);

  contentEndPosition = thisContent.size();
}

void cmStateSnapshot::SetDefaultDefinitions()
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
  this->SetDefinition("CMAKE_HOST_SYSTEM_NAME", "Windows");
#else
  this->SetDefinition("UNIX", "1");
  this->SetDefinition("CMAKE_HOST_UNIX", "1");

#  if defined(__ANDROID__)
  this->SetDefinition("CMAKE_HOST_SYSTEM_NAME", "Android");
#  else
  struct utsname uts_name;
  if (uname(&uts_name) >= 0) {
    this->SetDefinition("CMAKE_HOST_SYSTEM_NAME", uts_name.sysname);
  }
#  endif
#endif
#if defined(__CYGWIN__)
  std::string legacy;
  if (cmSystemTools::GetEnv("CMAKE_LEGACY_CYGWIN_WIN32", legacy) &&
      cmIsOn(legacy)) {
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

  this->SetDefinition("CMAKE_MAJOR_VERSION",
                      std::to_string(cmVersion::GetMajorVersion()));
  this->SetDefinition("CMAKE_MINOR_VERSION",
                      std::to_string(cmVersion::GetMinorVersion()));
  this->SetDefinition("CMAKE_PATCH_VERSION",
                      std::to_string(cmVersion::GetPatchVersion()));
  this->SetDefinition("CMAKE_TWEAK_VERSION",
                      std::to_string(cmVersion::GetTweakVersion()));
  this->SetDefinition("CMAKE_VERSION", cmVersion::GetCMakeVersion());

  this->SetDefinition("CMAKE_FILES_DIRECTORY", "/CMakeFiles");

  // Setup the default include file regular expression (match everything).
  this->Position->BuildSystemDirectory->Properties.SetProperty(
    "INCLUDE_REGULAR_EXPRESSION", "^.*$");
}

void cmStateSnapshot::SetDirectoryDefinitions()
{
  this->SetDefinition("CMAKE_SOURCE_DIR", this->State->GetSourceDirectory());
  this->SetDefinition("CMAKE_CURRENT_SOURCE_DIR",
                      this->State->GetSourceDirectory());
  this->SetDefinition("CMAKE_BINARY_DIR", this->State->GetBinaryDirectory());
  this->SetDefinition("CMAKE_CURRENT_BINARY_DIR",
                      this->State->GetBinaryDirectory());
}

void cmStateSnapshot::InitializeFromParent()
{
  cmStateDetail::PositionType parent = this->Position->DirectoryParent;
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

  InitializeContentFromParent(
    parent->BuildSystemDirectory->LinkOptions,
    this->Position->BuildSystemDirectory->LinkOptions,
    parent->BuildSystemDirectory->LinkOptionsBacktraces,
    this->Position->BuildSystemDirectory->LinkOptionsBacktraces,
    this->Position->LinkOptionsPosition);

  InitializeContentFromParent(
    parent->BuildSystemDirectory->LinkDirectories,
    this->Position->BuildSystemDirectory->LinkDirectories,
    parent->BuildSystemDirectory->LinkDirectoriesBacktraces,
    this->Position->BuildSystemDirectory->LinkDirectoriesBacktraces,
    this->Position->LinkDirectoriesPosition);

  cmProp include_regex =
    parent->BuildSystemDirectory->Properties.GetPropertyValue(
      "INCLUDE_REGULAR_EXPRESSION");
  this->Position->BuildSystemDirectory->Properties.SetProperty(
    "INCLUDE_REGULAR_EXPRESSION", cmToCStr(include_regex));
}

cmState* cmStateSnapshot::GetState() const
{
  return this->State;
}

cmStateDirectory cmStateSnapshot::GetDirectory() const
{
  return { this->Position->BuildSystemDirectory, *this };
}

void cmStateSnapshot::SetProjectName(const std::string& name)
{
  this->Position->BuildSystemDirectory->ProjectName = name;
}

std::string cmStateSnapshot::GetProjectName() const
{
  return this->Position->BuildSystemDirectory->ProjectName;
}

void cmStateSnapshot::InitializeFromParent_ForSubdirsCommand()
{
  std::string currentSrcDir = *this->GetDefinition("CMAKE_CURRENT_SOURCE_DIR");
  std::string currentBinDir = *this->GetDefinition("CMAKE_CURRENT_BINARY_DIR");
  this->InitializeFromParent();
  this->SetDefinition("CMAKE_SOURCE_DIR", this->State->GetSourceDirectory());
  this->SetDefinition("CMAKE_BINARY_DIR", this->State->GetBinaryDirectory());

  this->SetDefinition("CMAKE_CURRENT_SOURCE_DIR", currentSrcDir);
  this->SetDefinition("CMAKE_CURRENT_BINARY_DIR", currentBinDir);
}

bool cmStateSnapshot::StrictWeakOrder::operator()(
  const cmStateSnapshot& lhs, const cmStateSnapshot& rhs) const
{
  return lhs.Position.StrictWeakOrdered(rhs.Position);
}

bool operator==(const cmStateSnapshot& lhs, const cmStateSnapshot& rhs)
{
  return lhs.Position == rhs.Position;
}

bool operator!=(const cmStateSnapshot& lhs, const cmStateSnapshot& rhs)
{
  return lhs.Position != rhs.Position;
}
