/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


  Copyright (c) 2000 National Library of Medicine
  All rights reserved.

  See COPYRIGHT.txt for copyright details.

=========================================================================*/
#include "cmMakefile.h"
#include "cmStandardIncludes.h"
#include "cmMakeDepend.h"
#include "cmUnixMakefileGenerator.h"

// This is the main program used to gentrate makefile fragments 
// from CMakeLists.txt input files.   
int main(int ac, char** av)
{
  if(ac < 2)
    {
    std::cerr << "Usage: " << av[0] << " Makefile.in  -Ipath ..." << std::endl;
    return -1;
    }
  // Create a makefile
  cmMakefile mf;
  mf.AddDefinition("UNIX", "1");
  // Parse the command line
  if(ac > 2)
    {
    for(int i =2; i < ac; i++)
      {
      std::string arg = av[i];
      // Set the start source directory with a -S dir options
      if(arg.find("-S",0) == 0)
	{
	std::string path = arg.substr(2);
	mf.SetStartDirectory(path.c_str());
	}
      // Set the start output directory with a -O dir options
      if(arg.find("-O",0) == 0)
	{
	std::string path = arg.substr(2);
	mf.SetStartOutputDirectory(path.c_str());
	}
      // Set the source home directory with a -H dir option
      if(arg.find("-H",0) == 0)
	{
	std::string path = arg.substr(2);
	mf.SetHomeDirectory(path.c_str());
	}
      // Set the output or binary directory with a -B dir option
      if(arg.find("-B",0) == 0)
	{
	std::string path = arg.substr(2);
	mf.SetHomeOutputDirectory(path.c_str());
	}
      }
    }
  mf.SetMakefileGenerator(new cmUnixMakefileGenerator);

  // Read and parse the input makefile
  mf.MakeStartDirectoriesCurrent();
  if(!mf.ReadListFile(av[1]))
    {
    std::cerr << "Usage: " << av[0] << " Makefile.in  -Ipath ..." << std::endl;
    return -1;
    }
  mf.GenerateMakefile();
}
