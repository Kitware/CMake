/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <sstream>
#include <string>
#include <vector>

#include "cmDuration.h"

class cmake;
class cmCTest;

/** \class cmCTestBuildAndTest
 * \brief A class that handles ctest -S invocations
 *
 */
class cmCTestBuildAndTest
{
public:
  /*
   * The main entry point for this class
   */
  int Run();

  /*
   * Get the output variable
   */
  const char* GetOutput();

  cmCTestBuildAndTest(cmCTest* ctest);

private:
  cmCTest* CTest;

  //! Run CMake and build a test and then run it as a single test.
  int RunCMakeAndTest();
  int RunCMake(std::ostringstream& out, std::string& cmakeOutString,
               cmake* cm);

  std::string Output;

  std::string BuildGenerator;
  std::string BuildGeneratorPlatform;
  std::string BuildGeneratorToolset;
  std::vector<std::string> BuildOptions;
  bool BuildTwoConfig = false;
  std::string BuildMakeProgram;
  std::string ConfigSample;
  std::string SourceDir;
  std::string BinaryDir;
  std::string BuildProject;
  std::string TestCommand;
  bool BuildNoClean = false;
  std::string BuildRunDir;
  std::string ExecutableDirectory;
  std::vector<std::string> TestCommandArgs;
  std::vector<std::string> BuildTargets;
  bool BuildNoCMake = false;
  cmDuration Timeout = cmDuration::zero();

  friend class cmCTest;
};
