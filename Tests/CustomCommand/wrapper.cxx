#include <stdio.h>

int main(int argc, char *argv[])
{
  FILE *fp = fopen(argv[1],"w");
  
  fprintf(fp,"int wrapped() { return 5; }\n");
  fclose(fp);
}
