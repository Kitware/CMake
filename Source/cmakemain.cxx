#include "cmake.h"

int main(int ac, char** av)
{
  cmake cm;
  std::vector<std::string> args;
  for(int i =0; i < ac; ++i)
    {
    args.push_back(av[i]);
    }
  return cm.Generate(args);
}
