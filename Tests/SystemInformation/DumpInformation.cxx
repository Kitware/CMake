#include <stdio.h>
#include "DumpInformation.h"

int DumpFile(char* filename, char* comment)
{
  FILE* file = fopen(filename, "r");
  if(!file)
    {
    printf("Error, could not open file %s\n", filename);
    return 1;
    }
  printf("%s", comment);
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


int main(int, char*[])
{
  int res = 0;
  res += DumpFile(CMAKE_DUMP_FILE, "#CMake System Variables are:");
  res += DumpFile(CMAKE_CACHE_FILE, "#CMake Cache is:");
  res += DumpFile(CMAKE_ALL_VARIABLES, "#CMake Variables are:");
  return res;
}
