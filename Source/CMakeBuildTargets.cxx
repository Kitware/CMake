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
#include "cmCacheManager.h"

void Usage(const char* program)
{
  std::cerr << "Usage: " << program << " CMakeLists.txt " 
            << "-Ssource_start_directory "
            << "-Ooutput_start_directory "
            << "-Hsource_home_directory "
            << "-Boutput_home_directory\n"
            << "Where start directories are the current place in the tree,"
    "and the home directories are the top.\n";
}

  
// This is the main program used to gentrate makefile fragments 
// from CMakeLists.txt input files.   
int main(int ac, char** av)
{
  if(ac < 2)
    {
    Usage(av[0]);
    return -1;
    }
  // Create a makefile
  cmMakefile mf;
  mf.AddDefinition("UNIX", "1");
  bool makeCache = false;
  // Parse the command line
  if(ac > 2)
    {
    for(int i =2; i < ac; i++)
      {
      std::string arg = av[i];
      // Set the start source directory with a -S dir options
      if(arg.find("-MakeCache",0) == 0)
	{
        makeCache = true;
	}
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
  // Only generate makefiles if not trying to make the cache
  if(!makeCache)
    {
    mf.SetMakefileGenerator(new cmUnixMakefileGenerator);
    }
  

  // Read and parse the input makefile
  mf.MakeStartDirectoriesCurrent();
  cmCacheManager::GetInstance()->LoadCache(&mf);
  if(!mf.ReadListFile(av[1]))
    {
    Usage(av[0]);
    return -1;
    }
  if(makeCache)
    {
    mf.GenerateCacheOnly();
    }
  else
    {
    mf.GenerateMakefile();
    }
  cmCacheManager::GetInstance()->SaveCache(&mf);
  if(makeCache)
    {
    cmCacheManager::GetInstance()->PrintCache(std::cout);
    }
  
  if(cmSystemTools::GetErrorOccuredFlag())
    {
    return -1;
    }
  return 0;
}

