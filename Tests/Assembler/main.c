#include <stdio.h>

#if !defined(__STDC__) || __STDC__ == 0
int main(argc, argv)
  int argc;
  char*argv[];
#else
int main(int argc, char*argv[])
#endif
{
  printf("hello assembler world, %d arguments  given\n", argc);
  return 0;
}
