#include <stdio.h>

int main(int argc, char* argv[])
{
  int cc;
  for ( cc = 1; cc < argc; cc ++ )
    {
    printf("%s ", argv[cc]);
    }
  printf("\n");
  return 0;
}
