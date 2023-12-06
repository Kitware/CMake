module partition_level;

namespace partition_level {
int non_partition()
{
  return primary() + intf1() + intf2() + intf3() + intf4() + intf5() +
    intf6() + intf7() + impl1() + impl2() + impl3() + impl4() + impl5() +
    impl6() + impl7();
}
}
