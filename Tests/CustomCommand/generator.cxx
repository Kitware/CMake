#include <stdio.h>

int main(int argc, char *argv[])
{
  if ( argc < 2 )
    {
    fprintf(stderr, "Usage: %s <file>\n", argv[0]);
    return 1;
    }
  FILE *fp = fopen(argv[1],"w");
  
  fprintf(fp,"int generated() { return 3; }\n");
  fclose(fp);
  return 0;
}
