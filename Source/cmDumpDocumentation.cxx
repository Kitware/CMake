// Program extracts documentation describing commands from
// the CMake system.
// 
#include "cmMakefile.h"

int main(int ac, char** av)
{
  cmMakefile makefile;
  const char* outname = "cmake.html";
  if(ac > 1)
    {
    outname = av[1];
    }
  std::ofstream fout(outname);
  if(!fout)
    {
    std::cerr << "failed to open output file: " << outname << "\n";
    return -1;
    }
  makefile.DumpDocumentationToFile(fout);
  return 0;
}
