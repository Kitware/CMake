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

// Need these for documentation support.
#include "cmake.h"
#include "cmDocumentation.h"

//----------------------------------------------------------------------------
static const cmDocumentationEntry cmDocumentationName[] =
{
  {0,
   "  ctest - Testing driver provided by CMake.", 0},
  {0,0,0}
};

//----------------------------------------------------------------------------
static const cmDocumentationEntry cmDocumentationUsage[] =
{
  {0,
   "  ctest [options]", 0},
  {0,0,0}
};

//----------------------------------------------------------------------------
static const cmDocumentationEntry cmDocumentationDescription[] =
{
  {0,
   "The \"ctest\" executable is the CMake test driver program.  "
   "CMake-generated build trees created for projects that use "
   "the ENABLE_TESTING and ADD_TEST commands have testing support.  "
   "This program will run the tests and report results.", 0},
  CMAKE_STANDARD_INTRODUCTION,
  {0,0,0}
};

//----------------------------------------------------------------------------
static const cmDocumentationEntry cmDocumentationOptions[] =
{
  {"-C <config>", "Choose configuration to test.",
   "Some CMake-generated build trees can have multiple build configurations "
   "in the same tree.  This option can be used to specify which one should "
   "be tested.  Example configurations are \"Debug\" and \"Release\"."},
  {"-V,--verbose", "Enable verbose output from tests.",
   "Test output is normally suppressed and only summary information is "
   "displayed.  This option will show all test output."},
  {"-N,--show-only", "Disable actual execution of tests.",
   "This option tells ctest to list the tests that would be run but not "
   "actually run them.  Useful in conjunction with the -R and -E options."},
  {"-R <regex>", "Run tests matching regular expression.",
   "This option tells ctest to run only the tests whose names match the "
   "given regular expression."},
  {"-E <regex>", "Exclude tests matching regular expression.",
   "This option tells ctest to NOT run the tests whose names match the "
   "given regular expression."},
  {"-D <DashboardTest>", "Execute dashboard test",
   "This option tells ctest to perform act as a Dart client and perform "
   "a dashboard test. All tests are ModeTest, where Mode can be Experimental, "
   "Nightly, and Continuous, and Test can be Start, Update, Configure, "
   "Build, Test, Coverage, and Submit."},
  {"-S <ConfigScript>", "Execute a dashboard for a configuration",
   "This option tells ctest to load in a configuration script which sets "
   "a number of parameters such as the binary and source directories. Then "
   "ctest will do what is required to create and run a dashboard. This "
   "option basically sets up a dashboard and then runs ctest -D with the "
   "appropriate options."},
  {"-A <Notes file>", "Add a notes file with submission",
   "This option tells ctest to include a notes file when submitting dashboard. "},
  {"-I [Start,End,Stride,test#,test#|Test file]", "Run a specific number of tests by number.",
   "This option causes ctest to run tests starting at number Start, ending at number End, "
   "and incrementing by Stride. Any additional numbers after Stride are considered individual "
   "test numbers.  Start, End,or stride can be empty.  Optionally a file can be given that contains "
   "the same syntax as the command line."},
  {0,0,0}
};

//----------------------------------------------------------------------------
static const cmDocumentationEntry cmDocumentationSeeAlso[] =
{
  {0, "cmake", 0},
  {0, "ccmake", 0},
  {0, 0, 0}
};

// this is a test driver program for cmCTest.
int main (int argc, char *argv[])
{
  cmSystemTools::EnableMSVCDebugHook();
  
  // If there is a testing input file, check for documentation options
  // only if there are actually arguments.  We want running without
  // arguments to run tests.
  if(argc > 1 || !cmSystemTools::FileExists("DartTestfile.txt"))
    {
    if(argc == 1)
      {
      std::cout << "*********************************" << std::endl;
      std::cout << "No test configuration file found!" << std::endl;
      std::cout << "*********************************" << std::endl;
      }
    cmDocumentation doc;
    if(doc.CheckOptions(argc, argv))
      {
      // Construct and print requested documentation.
      doc.SetName("ctest");
      doc.SetNameSection(cmDocumentationName);
      doc.SetUsageSection(cmDocumentationUsage);
      doc.SetDescriptionSection(cmDocumentationDescription);
      doc.SetOptionsSection(cmDocumentationOptions);
      doc.SetSeeAlsoList(cmDocumentationSeeAlso);
      return doc.PrintRequestedDocumentation(std::cout)? 0:1;
      }
    }
  
  
#ifdef _WIN32
  std::string comspec = "cmw9xcom.exe";
  cmSystemTools::SetWindows9xComspecSubstitute(comspec.c_str());
#endif
  cmCTest inst;
  // copy the args to a vector
  std::vector<std::string> args;
  for(int i =0; i < argc; ++i)
    {
    args.push_back(argv[i]);
    }
  // run ctest
  return inst.Run(args);
}

