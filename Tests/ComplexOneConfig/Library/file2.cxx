#include <Library/cmTestLibraryConfigure.h>
#include <string.h>

int file2()
{
  return 1;
}

int PropertyTest()
{
  int ret = 1;
#ifndef ISABS
  ret = 0;
#endif
#ifndef WRAPEX
  ret = 0;
#endif
  if(strcmp(FLAGS,"-foo -bar") != 0)
    {
    ret  =0;
    }
  return ret;
}
