#include <stdio.h>

main(int argc, char *argv[])
{
  FILE *fp = fopen(argv[1],"w");
  
  fprintf(fp,"int generated() { return 3; }\n");
  fclose(fp);
}
