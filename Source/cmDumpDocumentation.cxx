// Program extracts documentation describing rules from
// the CMake system.
// 
#include "cmMakefile.h"

int main()
{
  cmMakefile makefile;
  makefile.DumpDocumentationToFile("cmake.txt");

  return 0;
}
