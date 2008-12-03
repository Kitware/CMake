#include "foo.h"
extern F_test_mod_sub();
extern F_mysub();
int main()
{
  F_mysub();
  F_my_sub();
#ifdef F_test_mod_sub
  F_test_mod_sub();
#endif
}
