export module importable;
import :internal_partition;

#include "internal-partitions_export.h"

export INTERNAL_PARTITIONS_EXPORT int from_import()
{
  return from_partition();
}
