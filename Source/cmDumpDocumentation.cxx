// Program extracts documentation describing commands from
// the CMake system.
// 
#include "cmMakefile.h"

int main()
{
  cmMakefile makefile;
  makefile.DumpDocumentationToFile("cmake.txt");

  return 0;
}
