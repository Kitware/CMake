#include "cmUnixMakefile.h"
#include "cmMakeDepend.h"
#include <iostream>


// This is the main program used to gentrate makefile fragments 
// from CMakeLists.txt input files.   
main(int ac, char** av)
{
  if(ac < 2)
    {
    std::cerr << "Usage: " << av[0] << " Makefile.in  -Ipath ..." << std::endl;
    return -1;
    }
  // Create a unix makefile
  cmUnixMakefile mf;
  // Create a depends object
  cmMakeDepend md;
  // Parse the command line
  if(ac > 2)
    {
    for(int i =2; i < ac; i++)
      {
      std::string arg = av[i];
      // Set the current source directory with a -S dir options
      if(arg.find("-S",0) != std::string::npos)
	{
	std::string path = arg.substr(2);
	mf.SetCurrentDirectory(path.c_str());
	}
      // Set the output or binary directory with a -B dir option
      if(arg.find("-B",0) != std::string::npos)
	{
	std::string path = arg.substr(2);
	mf.SetOutputHomeDirectory(path.c_str());
	}
      // Set the source home directory with a -H dir option
      if(arg.find("-H",0) != std::string::npos)
	{
	std::string path = arg.substr(2);
	mf.SetHomeDirectory(path.c_str());
	}
      }
    }
  // Read and parse the input makefile
  if(!mf.ReadMakefile(av[1]))
    {
    std::cerr << "Usage: " << av[0] << " Makefile.in  -Ipath ..." << std::endl;
    return -1;
    }
  // Set the makefile object on the depend object
  md.SetMakefile(&mf);
  // compute the depend information
  md.DoDepends();
  // Ouput the result
  mf.OutputMakefile("CMakeTargets.make");
}
