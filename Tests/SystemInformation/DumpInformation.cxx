#include <stdio.h>
#include "DumpInformation.h"

int main(int, char*[])
{
  FILE* file = fopen(CMAKE_DUMP_FILE, "r");
  if(!file)
    {
    printf("Error, could not open file %s\n", CMAKE_DUMP_FILE);
    return -1;
    }
  printf("#CMake System Variables are:");
  while(!feof(file))
    {
    int ch = fgetc(file);
    if(ch != EOF)
      {
      if(ch == '<')
        {
        printf("&lt;");
        }
      else if(ch == '>')
        {
        printf("&gt;");
        }
      else if(ch == '&')
        {
        printf("&amp;");
        }
      else 
        {
        putc(ch, stdout);
        }
      }
    }
  printf("\n");
  fclose(file);
  return 0;
}
