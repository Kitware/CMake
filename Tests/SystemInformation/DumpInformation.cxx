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
    int ch = fgetc(file);
    if(ch != EOF)
      {
      putc(ch, stdout);
      }
    }
  fclose(file);
  return 0;
}
