#include <stdio.h>
#include "DumpInformation.h"

int main(int, char*[])
{
  FILE* file = fopen(CMAKE_DUMP_FILE, "r");
  if(!file)
    {
    printf("Error, could not open file %s", CMAKE_DUMP_FILE);
    return -1;
    }
  while(!feof(file))
    {
    putc(fgetc(file), stdout);
    }
  fclose(file);
  return 0;
}
