#include "cmDSWMakefile.h"
#include "cmDSPMakefile.h"
#include <iostream>

// this is the command line version of CMakeSetup.
// It is called from Visual Studio when a CMakeLists.txt
// file is changed.


// Set the command line arguments
void SetArgs(cmMakefile& builder, int ac, char** av)
{
  for(int i =3; i < ac; i++)
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
      std::cerr << "set makefile dir " << path.c_str() << std::endl;
      builder.SetCurrentDirectory(path.c_str());
      }
    if(arg.find("-O",0) != std::string::npos)
      {
      std::string path = arg.substr(2);
      builder.SetOutputDirectory(path.c_str());
      }
    if(arg.find("-B",0) != std::string::npos)
      {
      std::string path = arg.substr(2);
      builder.SetOutputHomeDirectory(path.c_str());
      std::cout << "set output home to " << path.c_str() << std::endl;
      }
    }
}


main(int ac, char** av)
{
  if(ac < 3)
    {
    std::cerr << "Usage: " << av[0] << 
      " Makefile.in -[DSP|DSW] -Hinsighthome -Dcurrentdir -Ooutput directory" << std::endl;
    return -1;
    }
  std::string arg = av[2];
  if(arg.find("-DSP", 0) != std::string::npos)
    {
    cmDSPMakefile builder;
    SetArgs(builder, ac, av);
    builder.ReadMakefile(av[1]);
    builder.OutputDSPFile();
    }
  else
    {
    cmDSWMakefile builder;
    SetArgs(builder, ac, av);
    builder.ReadMakefile(av[1]);
    builder.OutputDSWFile();
    }
  return 0;
}

