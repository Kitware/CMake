

#include "testSharedLibDepends.h"

#ifdef TEST_SUBDIR_LIB
#include "subdir.h"
#endif

int main(int,char **)
{
  TestSharedLibDepends dep;
  TestSharedLibRequired req;

#ifdef TEST_SUBDIR_LIB
  SubDirObject sdo;
#endif

  return dep.foo() + req.foo()
#ifdef TEST_SUBDIR_LIB
                   + sdo.foo()
#endif
                              ;
}
