/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Kitware, Inc. All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef cmCTestBuildAndTestHandler_h
#define cmCTestBuildAndTestHandler_h


#include "cmCTestGenericHandler.h"
#include "cmListFileCache.h"

class cmake;

/** \class cmCTestBuildAndTestHandler
 * \brief A class that handles ctest -S invocations
 *
 */
class cmCTestBuildAndTestHandler : public cmCTestGenericHandler
{
public:
  cmTypeMacro(cmCTestBuildAndTestHandler, cmCTestGenericHandler);

  /*
   * The main entry point for this class
   */
  int ProcessHandler();

  //! Set all the build and test arguments
  virtual int ProcessCommandLineArguments(
    const std::string& currentArg, size_t& idx,
    const std::vector<std::string>& allArgs);

  /*
   * Get the output variable
   */
  const char* GetOutput();
  
  cmCTestBuildAndTestHandler();

  virtual void Initialize();

protected:
  ///! Run CMake and build a test and then run it as a single test.
  int RunCMakeAndTest(std::string* output);
  int RunCMake(std::string* outstring, cmOStringStream &out, 
               std::string &cmakeOutString,
               std::string &cwd, cmake *cm);
  
  cmStdString  Output;

  std::string              BuildGenerator;
  std::vector<std::string> BuildOptions;
  bool                     BuildTwoConfig;
  std::string              BuildMakeProgram;
  std::string              SourceDir;
  std::string              BinaryDir;
  std::string              BuildProject;
  std::string              TestCommand;
  bool                     BuildNoClean;
  std::string              BuildRunDir;
  std::string              ExecutableDirectory;
  std::vector<std::string> TestCommandArgs;
  std::vector<std::string> BuildTargets;
  bool                     BuildNoCMake;
};

#endif

