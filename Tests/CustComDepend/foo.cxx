#include <stdio.h>

int main(int ac, char** av)
{
  FILE* fout = fopen(av[1], "w");
  printf("create %s\n", av[1]);
  if(!fout)
    {
    return -1;
    }
  fprintf(fout, "int bar(){ return 10;}\n");
  fclose(fout);
  return 0;
}
  
