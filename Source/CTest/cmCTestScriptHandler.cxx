/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmCTestScriptHandler.h"

#include <cstdlib>
#include <map>
#include <ratio>
#include <sstream>
#include <utility>

#include <cm/memory>

#include <cm3p/uv.h>

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
#include "cmGlobalGenerator.h"
#include "cmMakefile.h"
#include "cmState.h"
#include "cmStateDirectory.h"
#include "cmStateSnapshot.h"
#include "cmSystemTools.h"
#include "cmUVHandlePtr.h"
#include "cmUVProcessChain.h"
#include "cmValue.h"
#include "cmake.h"

cmCTestScriptHandler::cmCTestScriptHandler() = default;

void cmCTestScriptHandler::Initialize()
{
  this->Superclass::Initialize();

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
  this->CMake->GetState()->AddBuiltinCommand(name, std::move(command));
}

int cmCTestScriptHandler::ExecuteScript(const std::string& total_script_arg)
{
  // execute the script passing in the arguments to the script as well as the
  // arguments from this invocation of cmake
  std::vector<std::string> argv;
  argv.push_back(cmSystemTools::GetCTestCommand());
  argv.push_back("-SR");
  argv.push_back(total_script_arg);

  cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT,
             "Executable for CTest is: " << cmSystemTools::GetCTestCommand()
                                         << "\n");

  // now pass through all the other arguments
  std::vector<std::string>& initArgs =
    this->CTest->GetInitialCommandLineArguments();
  //*** need to make sure this does not have the current script ***
  for (size_t i = 1; i < initArgs.size(); ++i) {
    argv.push_back(initArgs[i]);
  }

  // Now create process object
  cmUVProcessChainBuilder builder;
  builder.AddCommand(argv)
    .SetBuiltinStream(cmUVProcessChainBuilder::Stream_OUTPUT)
    .SetBuiltinStream(cmUVProcessChainBuilder::Stream_ERROR);
  auto process = builder.Start();
  cm::uv_pipe_ptr outPipe;
  outPipe.init(process.GetLoop(), 0);
  uv_pipe_open(outPipe, process.OutputStream());
  cm::uv_pipe_ptr errPipe;
  errPipe.init(process.GetLoop(), 0);
  uv_pipe_open(errPipe, process.ErrorStream());

  std::vector<char> out;
  std::vector<char> err;
  std::string line;
  auto pipe =
    cmSystemTools::WaitForLine(&process.GetLoop(), outPipe, errPipe, line,
                               std::chrono::seconds(100), out, err);
  while (pipe != cmSystemTools::WaitForLineResult::None) {
    cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT,
               "Output: " << line << "\n");
    if (pipe == cmSystemTools::WaitForLineResult::STDERR) {
      cmCTestLog(this->CTest, ERROR_MESSAGE, line << "\n");
    } else if (pipe == cmSystemTools::WaitForLineResult::STDOUT) {
      cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT, line << "\n");
    }
    pipe =
      cmSystemTools::WaitForLine(&process.GetLoop(), outPipe, errPipe, line,
                                 std::chrono::seconds(100), out, err);
  }

  // Properly handle output of the build command
  process.Wait();
  auto const& status = process.GetStatus(0);
  auto result = status.GetException();
  int retVal = 0;
  bool failed = false;
  switch (result.first) {
    case cmUVProcessChain::ExceptionCode::None:
      retVal = static_cast<int>(status.ExitStatus);
      break;
    case cmUVProcessChain::ExceptionCode::Spawn:
      cmCTestLog(this->CTest, ERROR_MESSAGE,
                 "\tError executing ctest: " << result.second << std::endl);
      failed = true;
      break;
    default:
      retVal = status.TermSignal;
      cmCTestLog(this->CTest, ERROR_MESSAGE,
                 "\tThere was an exception: " << result.second << " " << retVal
                                              << std::endl);
      failed = true;
  }
  if (failed) {
    std::ostringstream message;
    message << "Error running command: [";
    message << static_cast<int>(result.first) << "] ";
    for (std::string const& arg : argv) {
      message << arg << " ";
    }
    cmCTestLog(this->CTest, ERROR_MESSAGE, message.str() << std::endl);
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

  cmState* state = this->CMake->GetState();
  this->AddCTestCommand("ctest_build", cm::make_unique<cmCTestBuildCommand>());
  this->AddCTestCommand("ctest_configure",
                        cm::make_unique<cmCTestConfigureCommand>());
  this->AddCTestCommand("ctest_coverage",
                        cm::make_unique<cmCTestCoverageCommand>());
  state->AddBuiltinCommand("ctest_empty_binary_directory",
                           cmCTestEmptyBinaryDirectoryCommand);
  this->AddCTestCommand("ctest_memcheck",
                        cm::make_unique<cmCTestMemCheckCommand>());
  this->AddCTestCommand("ctest_read_custom_files",
                        cm::make_unique<cmCTestReadCustomFilesCommand>());
  this->AddCTestCommand("ctest_run_script",
                        cm::make_unique<cmCTestRunScriptCommand>());
  state->AddBuiltinCommand("ctest_sleep", cmCTestSleepCommand);
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

  return result;
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
