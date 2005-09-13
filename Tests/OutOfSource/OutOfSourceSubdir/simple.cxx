#include <stdio.h>
#include <string.h>

#include "testlib.h"
#include "testdp.h"

int main ()
{  
  if (strcmp(animal,"SIZZLING"))
    {
    fprintf(stderr,"Get definitions from a subdir did not work\n");
    return -2;
    }
  if(TestLib() != 1.0)
    {
    return -1;
    }
  return 0;
}
