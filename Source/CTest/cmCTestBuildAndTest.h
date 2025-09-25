/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

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

  cmCTestBuildAndTest(cmCTest* ctest);

private:
  cmCTest* CTest;

  bool RunCMake(cmake* cm);
  bool RunTest(std::vector<std::string> const& args, int* retVal,
               cmDuration timeout);

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
