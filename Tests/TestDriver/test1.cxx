#include <stdio.h>
int test1(int ac, char** av)
{
  printf("test1\n");
  for(int i =0; i < ac; i++)
    printf("arg %d is %s\n", ac, av[i]);
  return 0;
}
