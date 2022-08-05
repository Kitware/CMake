#include "foo.h"
extern void F_test_mod_sub(void);
extern void F_my_sub(void);
extern void F_mysub(void);
int myc(void)
{
  F_mysub();
  F_my_sub();
#ifdef TEST_MOD
  F_test_mod_sub();
#endif
  return 0;
}
