/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmCTestScriptHandler.h"

#include <cstdio>
#include <cstdlib>
#include <map>
#include <ratio>
#include <sstream>
#include <utility>

#include <cm/memory>

#include "cmsys/Directory.hxx"
#include "cmsys/Process.h"

#include "cmCTest.h"
#include "cmCTestBuildCommand.h"
#include "cmCTestCommand.h"
#include "cmCTestConfigureCommand.h"
#include "cmCTestCoverageCommand.h"
#include "cmCTestEmptyBinaryDirectoryCommand.h"
#include "cmCTestMemCheckCommand.h"
#include "cmCTestReadCustomFilesCommand.h"
#include "cmCTestRunScriptCommand.h"
#include "cmCTestSleepCommand.h"
#include "cmCTestStartCommand.h"
#include "cmCTestSubmitCommand.h"
#include "cmCTestTestCommand.h"
#include "cmCTestUpdateCommand.h"
#include "cmCTestUploadCommand.h"
#include "cmCommand.h"
#include "cmDuration.h"
#include "cmGeneratedFileStream.h"
#include "cmGlobalGenerator.h"
#include "cmList.h"
#include "cmMakefile.h"
#include "cmState.h"
#include "cmStateDirectory.h"
#include "cmStateSnapshot.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmValue.h"
#include "cmake.h"

#ifdef _WIN32
#  include <windows.h>
#else
#  include <unistd.h>
#endif

cmCTestScriptHandler::cmCTestScriptHandler() = default;

void cmCTestScriptHandler::Initialize()
{
  this->Superclass::Initialize();
  this->Backup = false;
  this->EmptyBinDir = false;
  this->EmptyBinDirOnce = false;

  this->SourceDir.clear();
  this->BinaryDir.clear();
  this->BackupSourceDir.clear();
  this->BackupBinaryDir.clear();
  this->CTestRoot.clear();
  this->CVSCheckOut.clear();
  this->CTestCmd.clear();
  this->UpdateCmd.clear();
  this->CTestEnv.clear();
  this->InitialCache.clear();
  this->CMakeCmd.clear();
  this->CMOutFile.clear();
  this->ExtraUpdates.clear();

  this->MinimumInterval = 20 * 60;
  this->ContinuousDuration = -1;

  // what time in seconds did this script start running
  this->ScriptStartTime = std::chrono::steady_clock::time_point();

  this->Makefile.reset();
  this->ParentMakefile = nullptr;

  this->GlobalGenerator.reset();

  this->CMake.reset();
}

cmCTestScriptHandler::~cmCTestScriptHandler() = default;

// just adds an argument to the vector
void cmCTestScriptHandler::AddConfigurationScript(const std::string& script,
                                                  bool pscope)
{
  this->ConfigurationScripts.emplace_back(script);
  this->ScriptProcessScope.push_back(pscope);
}

// the generic entry point for handling scripts, this routine will run all
// the scripts provides a -S arguments
int cmCTestScriptHandler::ProcessHandler()
{
  int res = 0;
  for (size_t i = 0; i < this->ConfigurationScripts.size(); ++i) {
    // for each script run it
    res |= this->RunConfigurationScript(
      cmSystemTools::CollapseFullPath(this->ConfigurationScripts[i]),
      this->ScriptProcessScope[i]);
  }
  if (res) {
    return -1;
  }
  return 0;
}

void cmCTestScriptHandler::UpdateElapsedTime()
{
  if (this->Makefile) {
    // set the current elapsed time
    auto itime = cmDurationTo<unsigned int>(std::chrono::steady_clock::now() -
                                            this->ScriptStartTime);
    auto timeString = std::to_string(itime);
    this->Makefile->AddDefinition("CTEST_ELAPSED_TIME", timeString);
  }
}

void cmCTestScriptHandler::AddCTestCommand(
  std::string const& name, std::unique_ptr<cmCTestCommand> command)
{
  command->CTest = this->CTest;
  command->CTestScriptHandler = this;
  this->CMake->GetState()->AddBuiltinCommand(name, std::move(command));
}

int cmCTestScriptHandler::ExecuteScript(const std::string& total_script_arg)
{
  // execute the script passing in the arguments to the script as well as the
  // arguments from this invocation of cmake
  std::vector<const char*> argv;
  argv.push_back(cmSystemTools::GetCTestCommand().c_str());
  argv.push_back("-SR");
  argv.push_back(total_script_arg.c_str());

  cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT,
             "Executable for CTest is: " << cmSystemTools::GetCTestCommand()
                                         << "\n");

  // now pass through all the other arguments
  std::vector<std::string>& initArgs =
    this->CTest->GetInitialCommandLineArguments();
  //*** need to make sure this does not have the current script ***
  for (size_t i = 1; i < initArgs.size(); ++i) {
    argv.push_back(initArgs[i].c_str());
  }
  argv.push_back(nullptr);

  // Now create process object
  cmsysProcess* cp = cmsysProcess_New();
  cmsysProcess_SetCommand(cp, argv.data());
  // cmsysProcess_SetWorkingDirectory(cp, dir);
  cmsysProcess_SetOption(cp, cmsysProcess_Option_HideWindow, 1);
  // cmsysProcess_SetTimeout(cp, timeout);
  cmsysProcess_Execute(cp);

  std::vector<char> out;
  std::vector<char> err;
  std::string line;
  int pipe =
    cmSystemTools::WaitForLine(cp, line, std::chrono::seconds(100), out, err);
  while (pipe != cmsysProcess_Pipe_None) {
    cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT,
               "Output: " << line << "\n");
    if (pipe == cmsysProcess_Pipe_STDERR) {
      cmCTestLog(this->CTest, ERROR_MESSAGE, line << "\n");
    } else if (pipe == cmsysProcess_Pipe_STDOUT) {
      cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT, line << "\n");
    }
    pipe = cmSystemTools::WaitForLine(cp, line, std::chrono::seconds(100), out,
                                      err);
  }

  // Properly handle output of the build command
  cmsysProcess_WaitForExit(cp, nullptr);
  int result = cmsysProcess_GetState(cp);
  int retVal = 0;
  bool failed = false;
  if (result == cmsysProcess_State_Exited) {
    retVal = cmsysProcess_GetExitValue(cp);
  } else if (result == cmsysProcess_State_Exception) {
    retVal = cmsysProcess_GetExitException(cp);
    cmCTestLog(this->CTest, ERROR_MESSAGE,
               "\tThere was an exception: "
                 << cmsysProcess_GetExceptionString(cp) << " " << retVal
                 << std::endl);
    failed = true;
  } else if (result == cmsysProcess_State_Expired) {
    cmCTestLog(this->CTest, ERROR_MESSAGE,
               "\tThere was a timeout" << std::endl);
    failed = true;
  } else if (result == cmsysProcess_State_Error) {
    cmCTestLog(this->CTest, ERROR_MESSAGE,
               "\tError executing ctest: " << cmsysProcess_GetErrorString(cp)
                                           << std::endl);
    failed = true;
  }
  cmsysProcess_Delete(cp);
  if (failed) {
    std::ostringstream message;
    message << "Error running command: [";
    message << result << "] ";
    for (const char* arg : argv) {
      if (arg) {
        message << arg << " ";
      }
    }
    cmCTestLog(this->CTest, ERROR_MESSAGE,
               message.str() << argv[0] << std::endl);
    return -1;
  }
  return retVal;
}

void cmCTestScriptHandler::CreateCMake()
{
  // create a cmake instance to read the configuration script
  this->CMake = cm::make_unique<cmake>(cmake::RoleScript, cmState::CTest);
  this->CMake->SetHomeDirectory("");
  this->CMake->SetHomeOutputDirectory("");
  this->CMake->GetCurrentSnapshot().SetDefaultDefinitions();
  this->CMake->AddCMakePaths();
  this->GlobalGenerator =
    cm::make_unique<cmGlobalGenerator>(this->CMake.get());

  cmStateSnapshot snapshot = this->CMake->GetCurrentSnapshot();
  std::string cwd = cmSystemTools::GetCurrentWorkingDirectory();
  snapshot.GetDirectory().SetCurrentSource(cwd);
  snapshot.GetDirectory().SetCurrentBinary(cwd);
  this->Makefile =
    cm::make_unique<cmMakefile>(this->GlobalGenerator.get(), snapshot);
  if (this->ParentMakefile) {
    this->Makefile->SetRecursionDepth(
      this->ParentMakefile->GetRecursionDepth());
  }

  this->CMake->SetProgressCallback(
    [this](const std::string& m, float /*unused*/) {
      if (!m.empty()) {
        cmCTestLog(this->CTest, HANDLER_OUTPUT, "-- " << m << std::endl);
      }
    });

  this->AddCTestCommand("ctest_build", cm::make_unique<cmCTestBuildCommand>());
  this->AddCTestCommand("ctest_configure",
                        cm::make_unique<cmCTestConfigureCommand>());
  this->AddCTestCommand("ctest_coverage",
                        cm::make_unique<cmCTestCoverageCommand>());
  this->AddCTestCommand("ctest_empty_binary_directory",
                        cm::make_unique<cmCTestEmptyBinaryDirectoryCommand>());
  this->AddCTestCommand("ctest_memcheck",
                        cm::make_unique<cmCTestMemCheckCommand>());
  this->AddCTestCommand("ctest_read_custom_files",
                        cm::make_unique<cmCTestReadCustomFilesCommand>());
  this->AddCTestCommand("ctest_run_script",
                        cm::make_unique<cmCTestRunScriptCommand>());
  this->AddCTestCommand("ctest_sleep", cm::make_unique<cmCTestSleepCommand>());
  this->AddCTestCommand("ctest_start", cm::make_unique<cmCTestStartCommand>());
  this->AddCTestCommand("ctest_submit",
                        cm::make_unique<cmCTestSubmitCommand>());
  this->AddCTestCommand("ctest_test", cm::make_unique<cmCTestTestCommand>());
  this->AddCTestCommand("ctest_update",
                        cm::make_unique<cmCTestUpdateCommand>());
  this->AddCTestCommand("ctest_upload",
                        cm::make_unique<cmCTestUploadCommand>());
}

// this sets up some variables for the script to use, creates the required
// cmake instance and generators, and then reads in the script
int cmCTestScriptHandler::ReadInScript(const std::string& total_script_arg)
{
  // Reset the error flag so that the script is read in no matter what
  cmSystemTools::ResetErrorOccurredFlag();

  // if the argument has a , in it then it needs to be broken into the fist
  // argument (which is the script) and the second argument which will be
  // passed into the scripts as S_ARG
  std::string script;
  std::string script_arg;
  const std::string::size_type comma_pos = total_script_arg.find(',');
  if (comma_pos != std::string::npos) {
    script = total_script_arg.substr(0, comma_pos);
    script_arg = total_script_arg.substr(comma_pos + 1);
  } else {
    script = total_script_arg;
  }
  // make sure the file exists
  if (!cmSystemTools::FileExists(script)) {
    cmSystemTools::Error("Cannot find file: " + script);
    return 1;
  }

  // read in the list file to fill the cache
  // create a cmake instance to read the configuration script
  this->CreateCMake();

  // set a variable with the path to the current script
  this->Makefile->AddDefinition("CTEST_SCRIPT_DIRECTORY",
                                cmSystemTools::GetFilenamePath(script));
  this->Makefile->AddDefinition("CTEST_SCRIPT_NAME",
                                cmSystemTools::GetFilenameName(script));
  this->Makefile->AddDefinition("CTEST_EXECUTABLE_NAME",
                                cmSystemTools::GetCTestCommand());
  this->Makefile->AddDefinition("CMAKE_EXECUTABLE_NAME",
                                cmSystemTools::GetCMakeCommand());
  this->Makefile->AddDefinitionBool("CTEST_RUN_CURRENT_SCRIPT", true);
  this->SetRunCurrentScript(true);
  this->UpdateElapsedTime();

  // set the CTEST_CONFIGURATION_TYPE variable to the current value of the
  // the -C argument on the command line.
  if (!this->CTest->GetConfigType().empty()) {
    this->Makefile->AddDefinition("CTEST_CONFIGURATION_TYPE",
                                  this->CTest->GetConfigType());
  }

  // add the script arg if defined
  if (!script_arg.empty()) {
    this->Makefile->AddDefinition("CTEST_SCRIPT_ARG", script_arg);
  }

  // set a callback function to update the elapsed time
  this->Makefile->OnExecuteCommand([this] { this->UpdateElapsedTime(); });

  /* Execute CTestScriptMode.cmake, which loads CMakeDetermineSystem and
  CMakeSystemSpecificInformation, so
  that variables like CMAKE_SYSTEM and also the search paths for libraries,
  header and executables are set correctly and can be used. Makes new-style
  ctest scripting easier. */
  std::string systemFile =
    this->Makefile->GetModulesFile("CTestScriptMode.cmake");
  if (!this->Makefile->ReadListFile(systemFile) ||
      cmSystemTools::GetErrorOccurredFlag()) {
    cmCTestLog(this->CTest, ERROR_MESSAGE,
               "Error in read:" << systemFile << "\n");
    return 2;
  }

  // Add definitions of variables passed in on the command line:
  const std::map<std::string, std::string>& defs =
    this->CTest->GetDefinitions();
  for (auto const& d : defs) {
    this->Makefile->AddDefinition(d.first, d.second);
  }

  // finally read in the script
  if (!this->Makefile->ReadListFile(script) ||
      cmSystemTools::GetErrorOccurredFlag()) {
    // Reset the error flag so that it can run more than
    // one script with an error when you use ctest_run_script.
    cmSystemTools::ResetErrorOccurredFlag();
    return 2;
  }

  return 0;
}

// extract variables from the script to set ivars
int cmCTestScriptHandler::ExtractVariables()
{
  // Temporary variables
  cmValue minInterval;
  cmValue contDuration;

  this->SourceDir =
    this->Makefile->GetSafeDefinition("CTEST_SOURCE_DIRECTORY");
  this->BinaryDir =
    this->Makefile->GetSafeDefinition("CTEST_BINARY_DIRECTORY");

  // add in translations for src and bin
  cmSystemTools::AddKeepPath(this->SourceDir);
  cmSystemTools::AddKeepPath(this->BinaryDir);

  this->CTestCmd = this->Makefile->GetSafeDefinition("CTEST_COMMAND");
  this->CVSCheckOut = this->Makefile->GetSafeDefinition("CTEST_CVS_CHECKOUT");
  this->CTestRoot = this->Makefile->GetSafeDefinition("CTEST_DASHBOARD_ROOT");
  this->UpdateCmd = this->Makefile->GetSafeDefinition("CTEST_UPDATE_COMMAND");
  if (this->UpdateCmd.empty()) {
    this->UpdateCmd = this->Makefile->GetSafeDefinition("CTEST_CVS_COMMAND");
  }
  this->CTestEnv = this->Makefile->GetSafeDefinition("CTEST_ENVIRONMENT");
  this->InitialCache =
    this->Makefile->GetSafeDefinition("CTEST_INITIAL_CACHE");
  this->CMakeCmd = this->Makefile->GetSafeDefinition("CTEST_CMAKE_COMMAND");
  this->CMOutFile =
    this->Makefile->GetSafeDefinition("CTEST_CMAKE_OUTPUT_FILE_NAME");

  this->Backup = this->Makefile->IsOn("CTEST_BACKUP_AND_RESTORE");
  this->EmptyBinDir =
    this->Makefile->IsOn("CTEST_START_WITH_EMPTY_BINARY_DIRECTORY");
  this->EmptyBinDirOnce =
    this->Makefile->IsOn("CTEST_START_WITH_EMPTY_BINARY_DIRECTORY_ONCE");

  minInterval =
    this->Makefile->GetDefinition("CTEST_CONTINUOUS_MINIMUM_INTERVAL");
  contDuration = this->Makefile->GetDefinition("CTEST_CONTINUOUS_DURATION");

  char updateVar[40];
  int i;
  for (i = 1; i < 10; ++i) {
    snprintf(updateVar, sizeof(updateVar), "CTEST_EXTRA_UPDATES_%i", i);
    cmValue updateVal = this->Makefile->GetDefinition(updateVar);
    if (updateVal) {
      if (this->UpdateCmd.empty()) {
        cmSystemTools::Error(
          std::string(updateVar) +
          " specified without specifying CTEST_CVS_COMMAND.");
        return 12;
      }
      this->ExtraUpdates.emplace_back(*updateVal);
    }
  }

  // in order to backup and restore we also must have the cvs root
  if (this->Backup && this->CVSCheckOut.empty()) {
    cmSystemTools::Error(
      "Backup was requested without specifying CTEST_CVS_CHECKOUT.");
    return 3;
  }

  // make sure the required info is here
  if (this->SourceDir.empty() || this->BinaryDir.empty() ||
      this->CTestCmd.empty()) {
    std::string msg =
      cmStrCat("CTEST_SOURCE_DIRECTORY = ",
               (!this->SourceDir.empty()) ? this->SourceDir.c_str() : "(Null)",
               "\nCTEST_BINARY_DIRECTORY = ",
               (!this->BinaryDir.empty()) ? this->BinaryDir.c_str() : "(Null)",
               "\nCTEST_COMMAND = ",
               (!this->CTestCmd.empty()) ? this->CTestCmd.c_str() : "(Null)");
    cmSystemTools::Error(
      "Some required settings in the configuration file were missing:\n" +
      msg);
    return 4;
  }

  // if the dashboard root isn't specified then we can compute it from the
  // this->SourceDir
  if (this->CTestRoot.empty()) {
    this->CTestRoot = cmSystemTools::GetFilenamePath(this->SourceDir);
  }

  // the script may override the minimum continuous interval
  if (minInterval) {
    this->MinimumInterval = 60 * atof(minInterval->c_str());
  }
  if (contDuration) {
    this->ContinuousDuration = 60.0 * atof(contDuration->c_str());
  }

  this->UpdateElapsedTime();

  return 0;
}

void cmCTestScriptHandler::SleepInSeconds(unsigned int secondsToWait)
{
#if defined(_WIN32)
  Sleep(1000 * secondsToWait);
#else
  sleep(secondsToWait);
#endif
}

// run a specific script
int cmCTestScriptHandler::RunConfigurationScript(
  const std::string& total_script_arg, bool pscope)
{
#ifndef CMAKE_BOOTSTRAP
  cmSystemTools::SaveRestoreEnvironment sre;
#endif

  int result;

  this->ScriptStartTime = std::chrono::steady_clock::now();

  // read in the script
  if (pscope) {
    cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT,
               "Reading Script: " << total_script_arg << std::endl);
    result = this->ReadInScript(total_script_arg);
  } else {
    cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT,
               "Executing Script: " << total_script_arg << std::endl);
    result = this->ExecuteScript(total_script_arg);
  }
  if (result) {
    return result;
  }

  // only run the current script if we should
  if (this->Makefile && this->Makefile->IsOn("CTEST_RUN_CURRENT_SCRIPT") &&
      this->ShouldRunCurrentScript) {
    return this->RunCurrentScript();
  }
  return result;
}

int cmCTestScriptHandler::RunCurrentScript()
{
  int result;

  // do not run twice
  this->SetRunCurrentScript(false);

  // no popup widows
  cmSystemTools::SetRunCommandHideConsole(true);

  // extract the vars from the cache and store in ivars
  result = this->ExtractVariables();
  if (result) {
    return result;
  }

  // set any environment variables
  if (!this->CTestEnv.empty()) {
    cmList envArgs{ this->CTestEnv };
    cmSystemTools::AppendEnv(envArgs);
  }

  // now that we have done most of the error checking finally run the
  // dashboard, we may be asked to repeatedly run this dashboard, such as
  // for a continuous, do we need to run it more than once?
  if (this->ContinuousDuration >= 0) {
    this->UpdateElapsedTime();
    auto ending_time =
      std::chrono::steady_clock::now() + cmDuration(this->ContinuousDuration);
    if (this->EmptyBinDirOnce) {
      this->EmptyBinDir = true;
    }
    do {
      auto startOfInterval = std::chrono::steady_clock::now();
      result = this->RunConfigurationDashboard();
      auto interval = std::chrono::steady_clock::now() - startOfInterval;
      auto minimumInterval = cmDuration(this->MinimumInterval);
      if (interval < minimumInterval) {
        auto sleepTime =
          cmDurationTo<unsigned int>(minimumInterval - interval);
        this->SleepInSeconds(sleepTime);
      }
      if (this->EmptyBinDirOnce) {
        this->EmptyBinDir = false;
      }
    } while (std::chrono::steady_clock::now() < ending_time);
  }
  // otherwise just run it once
  else {
    result = this->RunConfigurationDashboard();
  }

  return result;
}

int cmCTestScriptHandler::CheckOutSourceDir()
{
  std::string command;
  std::string output;
  int retVal;
  bool res;

  if (!cmSystemTools::FileExists(this->SourceDir) &&
      !this->CVSCheckOut.empty()) {
    // we must now checkout the src dir
    output.clear();
    cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT,
               "Run cvs: " << this->CVSCheckOut << std::endl);
    res = cmSystemTools::RunSingleCommand(
      this->CVSCheckOut, &output, &output, &retVal, this->CTestRoot.c_str(),
      this->HandlerVerbose, cmDuration::zero() /*this->TimeOut*/);
    if (!res || retVal != 0) {
      cmSystemTools::Error("Unable to perform cvs checkout:\n" + output);
      return 6;
    }
  }
  return 0;
}

int cmCTestScriptHandler::BackupDirectories()
{
  int retVal;

  // compute the backup names
  this->BackupSourceDir = cmStrCat(this->SourceDir, "_CMakeBackup");
  this->BackupBinaryDir = cmStrCat(this->BinaryDir, "_CMakeBackup");

  // backup the binary and src directories if requested
  if (this->Backup) {
    // if for some reason those directories exist then first delete them
    if (cmSystemTools::FileExists(this->BackupSourceDir)) {
      cmSystemTools::RemoveADirectory(this->BackupSourceDir);
    }
    if (cmSystemTools::FileExists(this->BackupBinaryDir)) {
      cmSystemTools::RemoveADirectory(this->BackupBinaryDir);
    }

    // first rename the src and binary directories
    rename(this->SourceDir.c_str(), this->BackupSourceDir.c_str());
    rename(this->BinaryDir.c_str(), this->BackupBinaryDir.c_str());

    // we must now checkout the src dir
    retVal = this->CheckOutSourceDir();
    if (retVal) {
      this->RestoreBackupDirectories();
      return retVal;
    }
  }

  return 0;
}

int cmCTestScriptHandler::PerformExtraUpdates()
{
  std::string command;
  std::string output;
  int retVal;
  bool res;

  // do an initial cvs update as required
  command = this->UpdateCmd;
  for (std::string const& eu : this->ExtraUpdates) {
    cmList cvsArgs{ eu };
    if (cvsArgs.size() == 2) {
      std::string fullCommand = cmStrCat(command, " update ", cvsArgs[1]);
      output.clear();
      retVal = 0;
      cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT,
                 "Run Update: " << fullCommand << std::endl);
      res = cmSystemTools::RunSingleCommand(
        fullCommand, &output, &output, &retVal, cvsArgs[0].c_str(),
        this->HandlerVerbose, cmDuration::zero() /*this->TimeOut*/);
      if (!res || retVal != 0) {
        cmSystemTools::Error(cmStrCat("Unable to perform extra updates:\n", eu,
                                      "\nWith output:\n", output));
        return 0;
      }
    }
  }
  return 0;
}

// run a single dashboard entry
int cmCTestScriptHandler::RunConfigurationDashboard()
{
  // local variables
  std::string command;
  std::string output;
  int retVal;
  bool res;

  // make sure the src directory is there, if it isn't then we might be able
  // to check it out from cvs
  retVal = this->CheckOutSourceDir();
  if (retVal) {
    return retVal;
  }

  // backup the dirs if requested
  retVal = this->BackupDirectories();
  if (retVal) {
    return retVal;
  }

  // clear the binary directory?
  if (this->EmptyBinDir) {
    if (!cmCTestScriptHandler::EmptyBinaryDirectory(this->BinaryDir)) {
      cmCTestLog(this->CTest, ERROR_MESSAGE,
                 "Problem removing the binary directory" << std::endl);
    }
  }

  // make sure the binary directory exists if it isn't the srcdir
  if (!cmSystemTools::FileExists(this->BinaryDir) &&
      this->SourceDir != this->BinaryDir) {
    if (!cmSystemTools::MakeDirectory(this->BinaryDir)) {
      cmSystemTools::Error("Unable to create the binary directory:\n" +
                           this->BinaryDir);
      this->RestoreBackupDirectories();
      return 7;
    }
  }

  // if the binary directory and the source directory are the same,
  // and we are starting with an empty binary directory, then that means
  // we must check out the source tree
  if (this->EmptyBinDir && this->SourceDir == this->BinaryDir) {
    // make sure we have the required info
    if (this->CVSCheckOut.empty()) {
      cmSystemTools::Error(
        "You have specified the source and binary "
        "directories to be the same (an in source build). You have also "
        "specified that the binary directory is to be erased. This means "
        "that the source will have to be checked out from CVS. But you have "
        "not specified CTEST_CVS_CHECKOUT");
      return 8;
    }

    // we must now checkout the src dir
    retVal = this->CheckOutSourceDir();
    if (retVal) {
      this->RestoreBackupDirectories();
      return retVal;
    }
  }

  // backup the dirs if requested
  retVal = this->PerformExtraUpdates();
  if (retVal) {
    return retVal;
  }

  // put the initial cache into the bin dir
  if (!this->InitialCache.empty()) {
    if (!cmCTestScriptHandler::WriteInitialCache(this->BinaryDir,
                                                 this->InitialCache)) {
      this->RestoreBackupDirectories();
      return 9;
    }
  }

  // do an initial cmake to setup the DartConfig file
  int cmakeFailed = 0;
  std::string cmakeFailedOuput;
  if (!this->CMakeCmd.empty()) {
    command = cmStrCat(this->CMakeCmd, " \"", this->SourceDir);
    output.clear();
    command += "\"";
    retVal = 0;
    cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT,
               "Run cmake command: " << command << std::endl);
    res = cmSystemTools::RunSingleCommand(
      command, &output, &output, &retVal, this->BinaryDir.c_str(),
      this->HandlerVerbose, cmDuration::zero() /*this->TimeOut*/);

    if (!this->CMOutFile.empty()) {
      std::string cmakeOutputFile = this->CMOutFile;
      if (!cmSystemTools::FileIsFullPath(cmakeOutputFile)) {
        cmakeOutputFile = this->BinaryDir + "/" + cmakeOutputFile;
      }

      cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT,
                 "Write CMake output to file: " << cmakeOutputFile
                                                << std::endl);
      cmGeneratedFileStream fout(cmakeOutputFile);
      if (fout) {
        fout << output.c_str();
      } else {
        cmCTestLog(this->CTest, ERROR_MESSAGE,
                   "Cannot open CMake output file: "
                     << cmakeOutputFile << " for writing" << std::endl);
      }
    }
    if (!res || retVal != 0) {
      // even if this fails continue to the next step
      cmakeFailed = 1;
      cmakeFailedOuput = output;
    }
  }

  // run ctest, it may be more than one command in here
  cmList ctestCommands{ this->CTestCmd };
  // for each variable/argument do a putenv
  for (std::string const& ctestCommand : ctestCommands) {
    command = ctestCommand;
    output.clear();
    retVal = 0;
    cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT,
               "Run ctest command: " << command << std::endl);
    res = cmSystemTools::RunSingleCommand(
      command, &output, &output, &retVal, this->BinaryDir.c_str(),
      this->HandlerVerbose, cmDuration::zero() /*this->TimeOut*/);

    // did something critical fail in ctest
    if (!res || cmakeFailed || retVal & cmCTest::BUILD_ERRORS) {
      this->RestoreBackupDirectories();
      if (cmakeFailed) {
        cmCTestLog(this->CTest, ERROR_MESSAGE,
                   "Unable to run cmake:" << std::endl
                                          << cmakeFailedOuput << std::endl);
        return 10;
      }
      cmCTestLog(this->CTest, ERROR_MESSAGE,
                 "Unable to run ctest:" << std::endl
                                        << "command: " << command << std::endl
                                        << "output: " << output << std::endl);
      if (!res) {
        return 11;
      }
      return retVal * 100;
    }
  }

  // if all was successful, delete the backup dirs to free up disk space
  if (this->Backup) {
    cmSystemTools::RemoveADirectory(this->BackupSourceDir);
    cmSystemTools::RemoveADirectory(this->BackupBinaryDir);
  }

  return 0;
}

bool cmCTestScriptHandler::WriteInitialCache(const std::string& directory,
                                             const std::string& text)
{
  std::string cacheFile = cmStrCat(directory, "/CMakeCache.txt");
  cmGeneratedFileStream fout(cacheFile);
  if (!fout) {
    return false;
  }

  fout.write(text.data(), text.size());

  // Make sure the operating system has finished writing the file
  // before closing it.  This will ensure the file is finished before
  // the check below.
  fout.flush();
  fout.close();
  return true;
}

void cmCTestScriptHandler::RestoreBackupDirectories()
{
  // if we backed up the dirs and the build failed, then restore
  // the backed up dirs
  if (this->Backup) {
    // if for some reason those directories exist then first delete them
    if (cmSystemTools::FileExists(this->SourceDir)) {
      cmSystemTools::RemoveADirectory(this->SourceDir);
    }
    if (cmSystemTools::FileExists(this->BinaryDir)) {
      cmSystemTools::RemoveADirectory(this->BinaryDir);
    }
    // rename the src and binary directories
    rename(this->BackupSourceDir.c_str(), this->SourceDir.c_str());
    rename(this->BackupBinaryDir.c_str(), this->BinaryDir.c_str());
  }
}

bool cmCTestScriptHandler::RunScript(cmCTest* ctest, cmMakefile* mf,
                                     const std::string& sname, bool InProcess,
                                     int* returnValue)
{
  auto sh = cm::make_unique<cmCTestScriptHandler>();
  sh->SetCTestInstance(ctest);
  sh->ParentMakefile = mf;
  sh->AddConfigurationScript(sname, InProcess);
  int res = sh->ProcessHandler();
  if (returnValue) {
    *returnValue = res;
  }
  return true;
}

bool cmCTestScriptHandler::EmptyBinaryDirectory(const std::string& sname)
{
  // try to avoid deleting root
  if (sname.size() < 2) {
    return false;
  }

  // consider non existing target directory a success
  if (!cmSystemTools::FileExists(sname)) {
    return true;
  }

  // try to avoid deleting directories that we shouldn't
  std::string check = cmStrCat(sname, "/CMakeCache.txt");

  if (!cmSystemTools::FileExists(check)) {
    return false;
  }

  for (int i = 0; i < 5; ++i) {
    if (TryToRemoveBinaryDirectoryOnce(sname)) {
      return true;
    }
    cmSystemTools::Delay(100);
  }

  return false;
}

bool cmCTestScriptHandler::TryToRemoveBinaryDirectoryOnce(
  const std::string& directoryPath)
{
  cmsys::Directory directory;
  directory.Load(directoryPath);

  for (unsigned long i = 0; i < directory.GetNumberOfFiles(); ++i) {
    std::string path = directory.GetFile(i);

    if (path == "." || path == ".." || path == "CMakeCache.txt") {
      continue;
    }

    std::string fullPath = cmStrCat(directoryPath, "/", path);

    bool isDirectory = cmSystemTools::FileIsDirectory(fullPath) &&
      !cmSystemTools::FileIsSymlink(fullPath);

    if (isDirectory) {
      if (!cmSystemTools::RemoveADirectory(fullPath)) {
        return false;
      }
    } else {
      if (!cmSystemTools::RemoveFile(fullPath)) {
        return false;
      }
    }
  }

  return static_cast<bool>(cmSystemTools::RemoveADirectory(directoryPath));
}

cmDuration cmCTestScriptHandler::GetRemainingTimeAllowed()
{
  if (!this->Makefile) {
    return cmCTest::MaxDuration();
  }

  cmValue timelimitS = this->Makefile->GetDefinition("CTEST_TIME_LIMIT");

  if (!timelimitS) {
    return cmCTest::MaxDuration();
  }

  auto timelimit = cmDuration(atof(timelimitS->c_str()));

  auto duration = std::chrono::duration_cast<cmDuration>(
    std::chrono::steady_clock::now() - this->ScriptStartTime);
  return (timelimit - duration);
}

void cmCTestScriptHandler::SetRunCurrentScript(bool value)
{
  this->ShouldRunCurrentScript = value;
}
