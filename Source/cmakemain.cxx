#include "cmake.h"
#include "cmMakefileGenerator.h"

int main(int ac, char** av)
{
  cmake cm;
  std::vector<std::string> args;
  for(int i =0; i < ac; ++i)
    {
    args.push_back(av[i]);
    }
  int ret = cm.Generate(args);
  cmMakefileGenerator::UnRegisterGenerators();
  return ret;
}
