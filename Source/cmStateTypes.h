/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */

#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <cmext/enum_set>

#include "cmLinkedTree.h"

namespace cmStateDetail {
struct SnapshotDataType;
using PositionType = cmLinkedTree<cmStateDetail::SnapshotDataType>::iterator;
}

namespace cmStateEnums {

enum SnapshotType
{
  BaseType,
  BuildsystemDirectoryType,
  DeferCallType,
  FunctionCallType,
  MacroCallType,
  IncludeFileType,
  InlineListFileType,
  PolicyScopeType,
  VariableScopeType
};

// There are multiple overlapping ranges represented here. Be aware that adding
// a value to this enumeration may cause failures in numerous places which
// assume details about the ordering.
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

// Target search domains for FindTarget() style commands.
enum class TargetDomain : unsigned
{
  // Native, unaliased CMake targets, generated via add_library or
  // add_executable
  NATIVE,
  // Alias index, must include another domain
  ALIAS,
  // Foreign domain targets, generated directly inside CMake
  FOREIGN
};
using TargetDomainSet = cm::enum_set<TargetDomain>;
static TargetDomainSet const AllTargetDomains{ TargetDomain::NATIVE,
                                               TargetDomain::ALIAS,
                                               TargetDomain::FOREIGN };

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

enum ArtifactType
{
  RuntimeBinaryArtifact,
  ImportLibraryArtifact
};
}

namespace cmTraceEnums {

/** \brief Define supported trace formats **/
enum class TraceOutputFormat
{
  Undefined,
  Human,
  JSONv1
};
};
