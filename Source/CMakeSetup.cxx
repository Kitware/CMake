#include "cmDSWBuilder.h"
#include "cmDSPBuilder.h"
#include <iostream>

void SetArgs(cmPCBuilder& builder, int ac, char** av)
{
  for(int i =2; i < ac; i++)
    {
    std::string arg = av[i];
    if(arg.find("-H",0) != std::string::npos)
      {
      std::string path = arg.substr(2);
      builder.SetHomeDirectory(path.c_str());
      }
    if(arg.find("-D",0) != std::string::npos)
      {
      std::string path = arg.substr(2);
      builder.SetCurrentDirectory(path.c_str());
      }
    }
}

main(int ac, char** av)
{
  if(ac < 3)
    {
    std::cerr << "Usage: " << av[0] << 
      " Makefile.in -[DSP|DSW] -Hinsighthome -Dcurrentdir ..." << std::endl;
    return -1;
    }
  std::string arg = av[2];
  if(arg.find("-DSP", 0) != std::string::npos)
    {
    cmDSPBuilder builder;
    builder.SetInputMakefilePath(av[1]);
    SetArgs(builder, ac, av);
    builder.CreateDSPFile();
    }
  else
    {
    cmDSWBuilder builder;
    builder.SetInputMakefilePath(av[1]);
    SetArgs(builder, ac, av);
    builder.CreateDSWFile();
    }
  return 0;
}

