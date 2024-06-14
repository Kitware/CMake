/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <chrono>
#include <memory>
#include <string>
#include <vector>

#include "cmsys/Status.hxx"

#include "cmCTestGenericHandler.h"
#include "cmDuration.h"

class cmCTest;
class cmCTestCommand;
class cmGlobalGenerator;
class cmMakefile;
class cmake;

/** \class cmCTestScriptHandler
 * \brief A class that handles ctest -S invocations
 *
 * CTest script is controlled using several variables that script has to
 * specify and some optional ones. Required ones are:
 *   CTEST_SOURCE_DIRECTORY - Source directory of the project
 *   CTEST_BINARY_DIRECTORY - Binary directory of the project
 *   CTEST_COMMAND          - Testing commands
 *
 * Optional variables are:
 *   CTEST_BACKUP_AND_RESTORE
 *   CTEST_CMAKE_COMMAND
 *   CTEST_CMAKE_OUTPUT_FILE_NAME
 *   CTEST_CONTINUOUS_DURATION
 *   CTEST_CONTINUOUS_MINIMUM_INTERVAL
 *   CTEST_CVS_CHECKOUT
 *   CTEST_CVS_COMMAND
 *   CTEST_UPDATE_COMMAND
 *   CTEST_DASHBOARD_ROOT
 *   CTEST_ENVIRONMENT
 *   CTEST_INITIAL_CACHE
 *   CTEST_START_WITH_EMPTY_BINARY_DIRECTORY
 *   CTEST_START_WITH_EMPTY_BINARY_DIRECTORY_ONCE
 *
 * In addition the following variables can be used. The number can be 1-10.
 *   CTEST_EXTRA_UPDATES_1
 *   CTEST_EXTRA_UPDATES_2
 *   ...
 *   CTEST_EXTRA_UPDATES_10
 *
 * CTest script can use the following arguments CTest provides:
 *   CTEST_SCRIPT_ARG
 *   CTEST_SCRIPT_DIRECTORY
 *   CTEST_SCRIPT_NAME
 *
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
  int RunCurrentScript();

  /*
   * Empty Binary Directory
   */
  static bool EmptyBinaryDirectory(const std::string& dir, std::string& err);

  /*
   * Write an initial CMakeCache.txt from the given contents.
   */
  static bool WriteInitialCache(const std::string& directory,
                                const std::string& text);

  /*
   * Some elapsed time handling functions
   */
  static void SleepInSeconds(unsigned int secondsToWait);
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

  void SetRunCurrentScript(bool value);

private:
  // reads in a script
  int ReadInScript(const std::string& total_script_arg);
  int ExecuteScript(const std::string& total_script_arg);

  // extract vars from the script to set ivars
  int ExtractVariables();

  // perform a CVS checkout of the source dir
  int CheckOutSourceDir();

  // perform any extra cvs updates that were requested
  int PerformExtraUpdates();

  // backup and restore dirs
  int BackupDirectories();
  void RestoreBackupDirectories();

  int RunConfigurationScript(const std::string& script, bool pscope);
  int RunConfigurationDashboard();

  // Add ctest command
  void AddCTestCommand(std::string const& name,
                       std::unique_ptr<cmCTestCommand> command);

  // Try to remove the binary directory once
  static cmsys::Status TryToRemoveBinaryDirectoryOnce(
    const std::string& directoryPath);

  std::vector<std::string> ConfigurationScripts;
  std::vector<bool> ScriptProcessScope;

  bool ShouldRunCurrentScript;

  bool Backup = false;
  bool EmptyBinDir = false;
  bool EmptyBinDirOnce = false;

  std::string SourceDir;
  std::string BinaryDir;
  std::string BackupSourceDir;
  std::string BackupBinaryDir;
  std::string CTestRoot;
  std::string CVSCheckOut;
  std::string CTestCmd;
  std::string UpdateCmd;
  std::string CTestEnv;
  std::string InitialCache;
  std::string CMakeCmd;
  std::string CMOutFile;
  std::vector<std::string> ExtraUpdates;

  // the *60 is because the settings are in minutes but GetTime is seconds
  double MinimumInterval = 30 * 60;
  double ContinuousDuration = -1;

  // what time in seconds did this script start running
  std::chrono::steady_clock::time_point ScriptStartTime =
    std::chrono::steady_clock::time_point();

  std::unique_ptr<cmMakefile> Makefile;
  cmMakefile* ParentMakefile = nullptr;
  std::unique_ptr<cmGlobalGenerator> GlobalGenerator;
  std::unique_ptr<cmake> CMake;
};
