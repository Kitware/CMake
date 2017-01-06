/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */

#ifndef cmStateTypes_h
#define cmStateTypes_h

#include <cmConfigure.h> // IWYU pragma: keep

#include "cmLinkedTree.h"

namespace cmStateDetail {
struct SnapshotDataType;
typedef cmLinkedTree<cmStateDetail::SnapshotDataType>::iterator PositionType;
}

namespace cmStateEnums {

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
}

#endif
