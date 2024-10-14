/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <chrono>
#include <memory>
#include <string>
#include <vector>

#include "cmCTestGenericHandler.h"
#include "cmDuration.h"

class cmCTest;
class cmCTestCommand;
class cmGlobalGenerator;
class cmMakefile;
class cmake;

/** \class cmCTestScriptHandler
 * \brief A class that handles ctest -S invocations
 */
class cmCTestScriptHandler : public cmCTestGenericHandler
{
public:
  using Superclass = cmCTestGenericHandler;

  /**
   * Add a script to run, and if is should run in the current process
   */
  void AddConfigurationScript(const std::string&, bool pscope);

  /**
   * Run a dashboard using a specified confiuration script
   */
  int ProcessHandler() override;

  /*
   * Run a script
   */
  static bool RunScript(cmCTest* ctest, cmMakefile* mf,
                        const std::string& script, bool InProcess,
                        int* returnValue);

  /*
   * Some elapsed time handling functions
   */
  void UpdateElapsedTime();

  /**
   * Return the time remaianing that the script is allowed to run in
   * seconds if the user has set the variable CTEST_TIME_LIMIT. If that has
   * not been set it returns a very large value.
   */
  cmDuration GetRemainingTimeAllowed();

  cmCTestScriptHandler();
  cmCTestScriptHandler(const cmCTestScriptHandler&) = delete;
  const cmCTestScriptHandler& operator=(const cmCTestScriptHandler&) = delete;
  ~cmCTestScriptHandler() override;

  void Initialize() override;

  void CreateCMake();
  cmake* GetCMake() { return this->CMake.get(); }
  cmMakefile* GetMakefile() { return this->Makefile.get(); }

private:
  // reads in a script
  int ReadInScript(const std::string& total_script_arg);
  int ExecuteScript(const std::string& total_script_arg);

  int RunConfigurationScript(const std::string& script, bool pscope);

  // Add ctest command
  void AddCTestCommand(std::string const& name,
                       std::unique_ptr<cmCTestCommand> command);

  std::vector<std::string> ConfigurationScripts;
  std::vector<bool> ScriptProcessScope;

  // what time in seconds did this script start running
  std::chrono::steady_clock::time_point ScriptStartTime =
    std::chrono::steady_clock::time_point();

  std::unique_ptr<cmMakefile> Makefile;
  cmMakefile* ParentMakefile = nullptr;
  std::unique_ptr<cmGlobalGenerator> GlobalGenerator;
  std::unique_ptr<cmake> CMake;
};
