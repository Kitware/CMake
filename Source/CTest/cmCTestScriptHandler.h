/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <memory>
#include <string>
#include <vector>

class cmCTest;
class cmGlobalGenerator;
class cmMakefile;
class cmake;

/** \class cmCTestScriptHandler
 * \brief A class that handles ctest -S invocations
 */
class cmCTestScriptHandler
{
public:
  /**
   * Add a script to run, and if is should run in the current process
   */
  void AddConfigurationScript(std::string const&, bool pscope);

  /**
   * Run a dashboard using a specified configuration script
   */
  int ProcessHandler();

  /*
   * Run a script
   */
  static bool RunScript(cmCTest* ctest, cmMakefile* mf,
                        std::string const& script, bool InProcess,
                        int* returnValue);

  /*
   * Some elapsed time handling functions
   */
  void UpdateElapsedTime();

  cmCTestScriptHandler(cmCTest* ctest);
  cmCTestScriptHandler(cmCTestScriptHandler const&) = delete;
  cmCTestScriptHandler const& operator=(cmCTestScriptHandler const&) = delete;
  ~cmCTestScriptHandler();

  void CreateCMake();
  cmake* GetCMake() { return this->CMake.get(); }
  cmMakefile* GetMakefile() { return this->Makefile.get(); }

private:
  // reads in a script
  int ReadInScript(std::string const& total_script_arg);
  int ExecuteScript(std::string const& total_script_arg);

  int RunConfigurationScript(std::string const& script, bool pscope);

  cmCTest* CTest = nullptr;
  std::vector<std::string> ConfigurationScripts;
  std::vector<bool> ScriptProcessScope;

  std::unique_ptr<cmMakefile> Makefile;
  cmMakefile* ParentMakefile = nullptr;
  std::unique_ptr<cmGlobalGenerator> GlobalGenerator;
  std::unique_ptr<cmake> CMake;
};
