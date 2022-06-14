export module importable;
export import importable : partition;

#include "partitions_export.h"

export PARTITIONS_EXPORT int from_import()
{
  return from_partition();
}
