/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmCTestBuildAndTest.h"

#include <chrono>
#include <cstdint>
#include <iostream>
#include <ratio>
#include <utility>

#include <cm3p/uv.h>

#include "cmBuildOptions.h"
#include "cmCTest.h"
#include "cmCTestTestHandler.h"
#include "cmGlobalGenerator.h"
#include "cmMakefile.h"
#include "cmProcessOutput.h"
#include "cmState.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmUVHandlePtr.h"
#include "cmUVProcessChain.h"
#include "cmUVStream.h"
#include "cmWorkingDirectory.h"
#include "cmake.h"

struct cmMessageMetadata;

cmCTestBuildAndTest::cmCTestBuildAndTest(cmCTest* ctest)
  : CTest(ctest)
{
}

bool cmCTestBuildAndTest::RunCMake(cmake* cm)
{
  std::vector<std::string> args;
  args.push_back(cmSystemTools::GetCMakeCommand());
  args.push_back(this->SourceDir);
  if (!this->BuildGenerator.empty()) {
    args.push_back("-G" + this->BuildGenerator);
  }
  if (!this->BuildGeneratorPlatform.empty()) {
    args.push_back("-A" + this->BuildGeneratorPlatform);
  }
  if (!this->BuildGeneratorToolset.empty()) {
    args.push_back("-T" + this->BuildGeneratorToolset);
  }

  char const* config = nullptr;
  if (!this->CTest->GetConfigType().empty()) {
    config = this->CTest->GetConfigType().c_str();
  }

  if (config) {
    args.push_back("-DCMAKE_BUILD_TYPE:STRING=" + std::string(config));
  }
  if (!this->BuildMakeProgram.empty() &&
      (this->BuildGenerator.find("Make") != std::string::npos ||
       this->BuildGenerator.find("Ninja") != std::string::npos)) {
    args.push_back("-DCMAKE_MAKE_PROGRAM:FILEPATH=" + this->BuildMakeProgram);
  }

  for (std::string const& opt : this->BuildOptions) {
    args.push_back(opt);
  }
  std::cout << "======== CMake output     ======\n";
  if (cm->Run(args) != 0) {
    std::cout << "======== End CMake output ======\n"
                 "Error: cmake execution failed\n";
    return false;
  }
  // do another config?
  if (this->BuildTwoConfig) {
    if (cm->Run(args) != 0) {
      std::cout << "======== End CMake output ======\n"
                   "Error: cmake execution failed\n";
      return false;
    }
  }
  std::cout << "======== End CMake output ======\n";
  return true;
}

bool cmCTestBuildAndTest::RunTest(std::vector<std::string> const& argv,
                                  int* retVal, cmDuration timeout)
{
  cmUVProcessChainBuilder builder;
  builder.AddCommand(argv).SetMergedBuiltinStreams();
  auto chain = builder.Start();

  cmProcessOutput processOutput(cmProcessOutput::Auto);
  cm::uv_pipe_ptr outputStream;
  outputStream.init(chain.GetLoop(), 0);
  uv_pipe_open(outputStream, chain.OutputStream());
  auto outputHandle = cmUVStreamRead(
    outputStream,
    [&processOutput](std::vector<char> data) {
      std::string decoded;
      processOutput.DecodeText(data.data(), data.size(), decoded);
      std::cout << decoded << std::flush;
    },
    []() {});

  bool complete = chain.Wait(static_cast<uint64_t>(timeout.count() * 1000.0));

  bool result = false;

  if (complete) {
    auto const& status = chain.GetStatus(0);
    auto exception = status.GetException();
    switch (exception.first) {
      case cmUVProcessChain::ExceptionCode::None:
        *retVal = static_cast<int>(status.ExitStatus);
        result = true;
        break;
      case cmUVProcessChain::ExceptionCode::Spawn: {
        std::cout << "\n*** ERROR executing: " << exception.second;
      } break;
      default: {
        *retVal = status.TermSignal;
        std::cout << "\n*** Exception executing: " << exception.second;
      } break;
    }
  }

  return result;
}

class cmCTestBuildAndTestCaptureRAII
{
  cmake& CM;

public:
  cmCTestBuildAndTestCaptureRAII(cmake& cm)
    : CM(cm)
  {
    cmSystemTools::SetMessageCallback(
      [](std::string const& msg, cmMessageMetadata const& /* unused */) {
        std::cout << msg << std::endl;
      });

    cmSystemTools::SetStdoutCallback(
      [](std::string const& m) { std::cout << m << std::flush; });
    cmSystemTools::SetStderrCallback(
      [](std::string const& m) { std::cout << m << std::flush; });

    this->CM.SetProgressCallback([](std::string const& msg, float prog) {
      if (prog < 0) {
        std::cout << msg << std::endl;
      }
    });
  }

  ~cmCTestBuildAndTestCaptureRAII()
  {
    this->CM.SetProgressCallback(nullptr);
    cmSystemTools::SetStderrCallback(nullptr);
    cmSystemTools::SetStdoutCallback(nullptr);
    cmSystemTools::SetMessageCallback(nullptr);
  }

  cmCTestBuildAndTestCaptureRAII(cmCTestBuildAndTestCaptureRAII const&) =
    delete;
  cmCTestBuildAndTestCaptureRAII& operator=(
    cmCTestBuildAndTestCaptureRAII const&) = delete;
};

int cmCTestBuildAndTest::Run()
{
  // if the generator and make program are not specified then it is an error
  if (this->BuildGenerator.empty()) {
    std::cout << "--build-and-test requires that the generator "
                 "be provided using the --build-generator "
                 "command line option.\n";
    return 1;
  }

  cmake cm(cmake::RoleProject, cmState::Project);
  cm.SetHomeDirectory("");
  cm.SetHomeOutputDirectory("");
  cmCTestBuildAndTestCaptureRAII captureRAII(cm);
  static_cast<void>(captureRAII);

  if (this->CTest->GetConfigType().empty() && !this->ConfigSample.empty()) {
    // use the config sample to set the ConfigType
    std::string fullPath;
    std::string resultingConfig;
    std::vector<std::string> extraPaths;
    std::vector<std::string> failed;
    fullPath = cmCTestTestHandler::FindExecutable(
      this->CTest, this->ConfigSample, resultingConfig, extraPaths, failed);
    if (!fullPath.empty() && !resultingConfig.empty()) {
      this->CTest->SetConfigType(resultingConfig);
    }
    std::cout << "Using config sample with results: " << fullPath << " and "
              << resultingConfig << std::endl;
  }

  // we need to honor the timeout specified, the timeout include cmake, build
  // and test time
  auto clock_start = std::chrono::steady_clock::now();

  // make sure the binary dir is there
  std::cout << "Internal cmake changing into directory: " << this->BinaryDir
            << std::endl;
  if (!cmSystemTools::FileIsDirectory(this->BinaryDir)) {
    cmSystemTools::MakeDirectory(this->BinaryDir);
  }
  cmWorkingDirectory workdir(this->BinaryDir);
  if (workdir.Failed()) {
    std::cout << workdir.GetError() << '\n';
    return 1;
  }

  if (this->BuildNoCMake) {
    // Make the generator available for the Build call below.
    cm.SetGlobalGenerator(cm.CreateGlobalGenerator(this->BuildGenerator));
    if (!this->BuildGeneratorPlatform.empty()) {
      cmMakefile mf(cm.GetGlobalGenerator(), cm.GetCurrentSnapshot());
      if (!cm.GetGlobalGenerator()->SetGeneratorPlatform(
            this->BuildGeneratorPlatform, &mf)) {
        return 1;
      }
    }

    // Load the cache to make CMAKE_MAKE_PROGRAM available.
    cm.LoadCache(this->BinaryDir);
  } else {
    // do the cmake step, no timeout here since it is not a sub process
    if (!this->RunCMake(&cm)) {
      return 1;
    }
  }

  // do the build
  if (this->BuildTargets.empty()) {
    this->BuildTargets.emplace_back();
  }
  for (std::string const& tar : this->BuildTargets) {
    cmDuration remainingTime = std::chrono::seconds(0);
    if (this->Timeout > cmDuration::zero()) {
      remainingTime =
        this->Timeout - (std::chrono::steady_clock::now() - clock_start);
      if (remainingTime <= std::chrono::seconds(0)) {
        std::cout << "--build-and-test timeout exceeded. ";
        return 1;
      }
    }
    char const* config = nullptr;
    if (!this->CTest->GetConfigType().empty()) {
      config = this->CTest->GetConfigType().c_str();
    }
    if (!config) {
      config = "Debug";
    }

    cmBuildOptions buildOptions(!this->BuildNoClean, false,
                                PackageResolveMode::Disable);
    int retVal = cm.GetGlobalGenerator()->Build(
      cmake::NO_BUILD_PARALLEL_LEVEL, this->SourceDir, this->BinaryDir,
      this->BuildProject, { tar }, std::cout, this->BuildMakeProgram, config,
      buildOptions, false, remainingTime, cmSystemTools::OUTPUT_PASSTHROUGH);
    // if the build failed then return
    if (retVal) {
      return 1;
    }
  }

  // if no test was specified then we are done
  if (this->TestCommand.empty()) {
    return 0;
  }

  // now run the compiled test if we can find it
  // store the final location in fullPath
  std::string fullPath;
  std::string resultingConfig;
  std::vector<std::string> extraPaths;
  // if this->ExecutableDirectory is set try that as well
  if (!this->ExecutableDirectory.empty()) {
    std::string tempPath =
      cmStrCat(this->ExecutableDirectory, '/', this->TestCommand);
    extraPaths.push_back(tempPath);
  }
  std::vector<std::string> failed;
  fullPath = cmCTestTestHandler::FindExecutable(
    this->CTest, this->TestCommand, resultingConfig, extraPaths, failed);

  if (!cmSystemTools::FileExists(fullPath)) {
    std::cout
      << "Could not find path to executable, perhaps it was not built: "
      << this->TestCommand << "\n"
      << "tried to find it in these places:\n"
      << fullPath << '\n';
    for (std::string const& fail : failed) {
      std::cout << fail << '\n';
    }
    return 1;
  }

  std::vector<std::string> testCommand;
  testCommand.push_back(fullPath);
  for (std::string const& testCommandArg : this->TestCommandArgs) {
    testCommand.push_back(testCommandArg);
  }
  int retval = 0;
  // run the test from the this->BuildRunDir if set
  if (!this->BuildRunDir.empty()) {
    std::cout << "Run test in directory: " << this->BuildRunDir << '\n';
    if (!workdir.SetDirectory(this->BuildRunDir)) {
      std::cout << workdir.GetError() << '\n';
      return 1;
    }
  }
  std::cout << "Running test command: \"" << fullPath << '"';
  for (std::string const& testCommandArg : this->TestCommandArgs) {
    std::cout << " \"" << testCommandArg << '"';
  }
  std::cout << '\n';

  // how much time is remaining
  cmDuration remainingTime = std::chrono::seconds(0);
  if (this->Timeout > cmDuration::zero()) {
    remainingTime =
      this->Timeout - (std::chrono::steady_clock::now() - clock_start);
    if (remainingTime <= std::chrono::seconds(0)) {
      std::cout << "--build-and-test timeout exceeded. ";
      return 1;
    }
  }

  bool runTestRes = this->RunTest(testCommand, &retval, remainingTime);

  if (!runTestRes || retval != 0) {
    std::cout << "\nTest command failed: " << testCommand[0] << '\n';
    retval = 1;
  }

  return retval;
}
