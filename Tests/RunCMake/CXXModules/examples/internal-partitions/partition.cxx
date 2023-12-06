module importable:internal_partition;
import internal;

struct partition_struct
{
  internal_struct i;
};

int from_partition()
{
  return from_internal();
}
