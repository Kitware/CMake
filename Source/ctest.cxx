/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "cmCTest.h"
#include "cmSystemTools.h"

// this is a test driver program for cmCTest.
int main (int argc, char *argv[])
{
  cmSystemTools::EnableMSVCDebugHook();
  cmCTest inst;
  
  // look at the args
  std::vector<std::string> args;
  for(int i =0; i < argc; ++i)
    {
    args.push_back(argv[i]);
    }

#ifdef _WIN32
  std::string comspec = "cmw9xcom.exe";
  cmSystemTools::SetWindows9xComspecSubstitute(comspec.c_str());
#endif
  
  for(unsigned int i=1; i < args.size(); ++i)
    {
    std::string arg = args[i];
    if(arg.find("-D",0) == 0 && i < args.size() - 1)
      {
      inst.m_ConfigType = args[i+1];
      }

    if( arg.find("-V",0) == 0 || arg.find("--verbose",0) == 0 )
      {
      inst.m_Verbose = true;
      }

    if( arg.find("-N",0) == 0 || arg.find("--show-only",0) == 0 )
      {
      inst.m_ShowOnly = true;
      }

    if( ( arg.find("-T",0) == 0 || arg.find("--dart-mode",0) == 0 ) && 
        (i < args.size() -1) )
      {
      inst.m_DartMode = true;
      inst.SetTest(args[i+1].c_str());
      }
    
    if( ( arg.find("-M",0) == 0 || arg.find("--test-model",0) == 0 ) &&
        (i < args.size() -1) )
      {
      std::string& str = args[i+1];
      if ( str == "NIGHTLY" || str == "nightly" || str == "Nightly" )
        {
        inst.SetTestModel(cmCTest::NIGHTLY);
        }
      else if ( str == "CONTINUOUS" || str == "continuous" || 
                str == "Continuous" )
        {
        inst.SetTestModel(cmCTest::CONTINUOUS);
        std::cout << "Continuous" << std::endl;
        }
      else
        {
        inst.SetTestModel(cmCTest::EXPERIMENTAL);
        }
      }
    
    if(arg.find("-R",0) == 0 && i < args.size() - 1)
      {
      inst.m_UseIncludeRegExp = true;
      inst.m_IncludeRegExp  = args[i+1];
      }

    if(arg.find("-E",0) == 0 && i < args.size() - 1)
      {
      inst.m_UseExcludeRegExp = true;
      inst.m_ExcludeRegExp  = args[i+1];
      inst.m_UseExcludeRegExpFirst = inst.m_UseIncludeRegExp ? false : true;
      }

    if(arg.find("-h") == 0 || 
       arg.find("-help") == 0 || 
       arg.find("-H") == 0 || 
       arg.find("--help") == 0 || 
       arg.find("/H") == 0 || 
       arg.find("/HELP") == 0 || 
       arg.find("/?") == 0)
      {
      std::cerr << "Usage: " << argv[0] << " <options>" << std::endl
                << "\t -D type      Specify config type" << std::endl
                << "\t -E test      Specify regular expression for tests to exclude" 
                << std::endl
                << "\t -R test      Specify regular expression for tests to include" 
                << std::endl
                << "\t -V           Verbose testing" << std::endl
                << "\t -N           Only show what would be done without this option" 
                << std::endl
                << "\t -H           Help page" << std::endl;
      return 1;
      }
    }

  // call process directory
  inst.Initialize();
  int res = inst.ProcessTests();
  inst.Finalize();

  return res;
}

