export module partition_level:intf3;
export import :intf2;
namespace partition_level {
int intf3()
{
  return intf2();
}
}
