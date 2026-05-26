export module partition_level;
export import :intf7;
namespace partition_level {
int primary()
{
  return intf7();
}

int non_partition();

export int get()
{
  return non_partition();
}
}
