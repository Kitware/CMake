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
#include "cmStandardIncludes.h"
#include "cmMakefile.h"
#include "cmMSProjectGenerator.h"


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
    if(arg.find("-S",0) != std::string::npos)
      {
      std::string path = arg.substr(2);
      builder.SetStartDirectory(path.c_str());
      }
    if(arg.find("-O",0) != std::string::npos)
      {
      std::string path = arg.substr(2);
      builder.SetStartOutputDirectory(path.c_str());
      }
    if(arg.find("-B",0) != std::string::npos)
      {
      std::string path = arg.substr(2);
      builder.SetHomeOutputDirectory(path.c_str());
      std::cout << "set output home to " << path.c_str() << std::endl;
      }
    }
}


int main(int ac, char** av)
{
  if(ac < 3)
    {
    std::cerr << "Usage: " << av[0] << 
      " CMakeLists.txt -[DSP|DSW] -Hinsighthome -Dcurrentdir"
      " -Ooutput directory" << std::endl;
    return -1;
    }
  std::string arg = av[2];
  cmMakefile builder;
  SetArgs(builder, ac, av);
  cmMSProjectGenerator* pg = new cmMSProjectGenerator;
  if(arg.find("-DSP", 0) != std::string::npos)
    {
    pg->BuildDSWOff();
    }
  else
    {
    pg->BuildDSWOn();
    }
  builder.SetMakefileGenerator(pg);
  builder.MakeStartDirectoriesCurrent();
  builder.ReadListFile(av[1]);
  builder.GenerateMakefile();
  return 0;
}

