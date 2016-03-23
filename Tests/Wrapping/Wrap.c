#include <stdio.h>

#if !defined(__STDC__) || __STDC__ == 0
int main(argc, argv)
  int argc;
  char ** argv;
#else
int main(int argc, const char* argv[])
#endif
{
  FILE* fout = fopen(argv[argc-1], "w");
  printf("Wrap creating \"%s\"\n", argv[argc-1]);
  if(fout)
    {
    fprintf(fout, "int foo() { return 0; }\n");
    fclose(fout);
    }
  else
    {
    return -1;
    }
  return 0;
}
