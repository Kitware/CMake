#include "cmUnixMakefile.h"
#include "cmMakeDepend.h"
#include <iostream>

main(int ac, char** av)
{
  if(ac < 2)
    {
    std::cerr << "Usage: " << av[0] << " Makefile.in  -Ipath ..." << std::endl;
    return -1;
    }
  cmUnixMakefile* mf = new cmUnixMakefile;
  cmMakeDepend md;
  if(ac > 2)
    {
    for(int i =2; i < ac; i++)
      {
      std::string arg = av[i];
      if(arg.find("-I",0) != std::string::npos)
	{
	std::string path = arg.substr(2);
	md.AddSearchPath(path.c_str());
	}
      if(arg.find("-S",0) != std::string::npos)
	{
	std::string path = arg.substr(2);
	mf->SetCurrentDirectory(path.c_str());
	}
      }
    }
  if(!mf->ReadMakefile(av[1]))
    {
    std::cerr << "Usage: " << av[0] << " Makefile.in  -Ipath ..." << std::endl;
    return -1;
    }

  md.SetMakefile(mf);
  md.DoDepends();
  mf->OutputMakefile("CMakeTargets.make");
  delete mf;
}
