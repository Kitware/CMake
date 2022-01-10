/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */

#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>
#include <vector>

#include "cmDefinitions.h"
#include "cmLinkedTree.h"
#include "cmListFileCache.h"
#include "cmPolicies.h"
#include "cmPropertyMap.h"
#include "cmStateSnapshot.h"
#include "cmStateTypes.h"

namespace cmStateDetail {
struct BuildsystemDirectoryStateType;
struct PolicyStackEntry;
} // namespace cmStateDetail

static const std::string cmPropertySentinal = std::string();

struct cmStateDetail::SnapshotDataType
{
  cmStateDetail::PositionType ScopeParent;
  cmStateDetail::PositionType DirectoryParent;
  cmLinkedTree<cmStateDetail::PolicyStackEntry>::iterator Policies;
  cmLinkedTree<cmStateDetail::PolicyStackEntry>::iterator PolicyRoot;
  cmLinkedTree<cmStateDetail::PolicyStackEntry>::iterator PolicyScope;
  cmStateEnums::SnapshotType SnapshotType;
  bool Keep;
  cmLinkedTree<std::string>::iterator ExecutionListFile;
  cmLinkedTree<cmStateDetail::BuildsystemDirectoryStateType>::iterator
    BuildSystemDirectory;
  cmLinkedTree<cmDefinitions>::iterator Vars;
  cmLinkedTree<cmDefinitions>::iterator Root;
  cmLinkedTree<cmDefinitions>::iterator Parent;
  std::vector<std::string>::size_type IncludeDirectoryPosition;
  std::vector<std::string>::size_type CompileDefinitionsPosition;
  std::vector<std::string>::size_type CompileOptionsPosition;
  std::vector<std::string>::size_type LinkOptionsPosition;
  std::vector<std::string>::size_type LinkDirectoriesPosition;
};

struct cmStateDetail::PolicyStackEntry : public cmPolicies::PolicyMap
{
  using derived = cmPolicies::PolicyMap;
  PolicyStackEntry(bool w = false)
    : Weak(w)
  {
  }
  PolicyStackEntry(derived const& d, bool w)
    : derived(d)
    , Weak(w)
  {
  }
  bool Weak;
};

struct cmStateDetail::BuildsystemDirectoryStateType
{
  cmStateDetail::PositionType DirectoryEnd;

  std::string Location;
  std::string OutputLocation;

  std::vector<BT<std::string>> IncludeDirectories;

  std::vector<BT<std::string>> CompileDefinitions;

  std::vector<BT<std::string>> CompileOptions;

  std::vector<BT<std::string>> LinkOptions;

  std::vector<BT<std::string>> LinkDirectories;

  std::vector<std::string> NormalTargetNames;
  std::vector<std::string> ImportedTargetNames;

  std::string ProjectName;

  cmPropertyMap Properties;

  std::vector<cmStateSnapshot> Children;
};
