export module partition_level:intf2;
export import :intf1;
namespace partition_level {
int intf2()
{
  return intf1();
}
}
