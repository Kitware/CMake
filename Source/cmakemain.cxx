#include "cmake.h"

int main(int ac, char** av)
{
  cmake cm;
  if(ac < 1)
    {
      cm.Usage(av[0]);
      return -1;
    }
  std::vector<std::string> args;
  for(int i =0; i < ac; ++i)
    {
    args.push_back(av[i]);
    }
  return cm.Generate(args);
}
