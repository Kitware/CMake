#include "cmakewizard.h"
#include "cmake.h"
#include "cmMakefileGenerator.h"

int main(int ac, char** av)
{
  bool wiz = false;
  std::vector<std::string> args;
  for(int i =0; i < ac; ++i)
    {
    if(strcmp(av[i], "-i") == 0)
      {
      wiz = true;
      }
    else
      {
      args.push_back(av[i]);
      }
    }
  if(!wiz)
    {
    cmake cm;
    int ret = cm.Generate(args);
    cmMakefileGenerator::UnRegisterGenerators();
    return ret;
    }
  cmakewizard wizard;
  wizard.RunWizard(args); 
  cmMakefileGenerator::UnRegisterGenerators();
  return 0;
}
