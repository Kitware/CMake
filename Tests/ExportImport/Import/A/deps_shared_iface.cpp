

#include "testSharedLibDepends.h"

#include "testSharedLibImportDepend.h"

#ifdef CHECK_PIC_WORKS
#if defined(__ELF__) && !defined(__PIC__) && !defined(__PIE__)
#error Expected by INTERFACE_POSITION_INDEPENDENT_CODE property of dependency
#endif
#endif

#ifdef TEST_SUBDIR_LIB
#include "subdir.h"
#endif

int main(int,char **)
{
  TestSharedLibDepends dep;
  TestSharedLibRequired req;
  TestSharedLibImportDepend imp;

#ifdef TEST_SUBDIR_LIB
  SubDirObject sdo;
#endif

  return dep.foo() + req.foo() + imp.foo()
#ifdef TEST_SUBDIR_LIB
                   + sdo.foo()
#endif
                              ;
}
