/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmCTestBuildAndTest.h"

#include <chrono>
#include <cstring>
#include <ratio>

#include "cmBuildOptions.h"
#include "cmCTest.h"
#include "cmCTestTestHandler.h"
#include "cmGlobalGenerator.h"
#include "cmMakefile.h"
#include "cmState.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmWorkingDirectory.h"
#include "cmake.h"

struct cmMessageMetadata;

cmCTestBuildAndTest::cmCTestBuildAndTest(cmCTest* ctest)
  : CTest(ctest)
{
}

const char* cmCTestBuildAndTest::GetOutput()
{
  return this->Output.c_str();
}

int cmCTestBuildAndTest::Run()
{
  this->Output.clear();
  cmSystemTools::ResetErrorOccurredFlag();
  int retv = this->RunCMakeAndTest();
  cmSystemTools::ResetErrorOccurredFlag();
  return retv;
}

int cmCTestBuildAndTest::RunCMake(std::ostringstream& out,
                                  std::string& cmakeOutString, cmake* cm)
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

  const char* config = nullptr;
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
  if (cm->Run(args) != 0) {
    out << "Error: cmake execution failed\n";
    out << cmakeOutString << "\n";
    this->Output = out.str();
    return 1;
  }
  // do another config?
  if (this->BuildTwoConfig) {
    if (cm->Run(args) != 0) {
      out << "Error: cmake execution failed\n";
      out << cmakeOutString << "\n";
      this->Output = out.str();
      return 1;
    }
  }
  out << "======== CMake output     ======\n";
  out << cmakeOutString;
  out << "======== End CMake output ======\n";
  return 0;
}

class cmCTestBuildAndTestCaptureRAII
{
  cmake& CM;

public:
  cmCTestBuildAndTestCaptureRAII(cmake& cm, std::string& s)
    : CM(cm)
  {
    cmSystemTools::SetMessageCallback(
      [&s](const std::string& msg, const cmMessageMetadata& /* unused */) {
        s += msg;
        s += "\n";
      });

    cmSystemTools::SetStdoutCallback([&s](std::string const& m) { s += m; });
    cmSystemTools::SetStderrCallback([&s](std::string const& m) { s += m; });

    this->CM.SetProgressCallback([&s](const std::string& msg, float prog) {
      if (prog < 0) {
        s += msg;
        s += "\n";
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

  cmCTestBuildAndTestCaptureRAII(const cmCTestBuildAndTestCaptureRAII&) =
    delete;
  cmCTestBuildAndTestCaptureRAII& operator=(
    const cmCTestBuildAndTestCaptureRAII&) = delete;
};

int cmCTestBuildAndTest::RunCMakeAndTest()
{
  // if the generator and make program are not specified then it is an error
  if (this->BuildGenerator.empty()) {
    this->Output = "--build-and-test requires that the generator "
                   "be provided using the --build-generator "
                   "command line option.\n";
    return 1;
  }

  cmake cm(cmake::RoleProject, cmState::Project);
  cm.SetHomeDirectory("");
  cm.SetHomeOutputDirectory("");
  std::string cmakeOutString;
  cmCTestBuildAndTestCaptureRAII captureRAII(cm, cmakeOutString);
  static_cast<void>(captureRAII);
  std::ostringstream out;

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
    out << "Using config sample with results: " << fullPath << " and "
        << resultingConfig << std::endl;
  }

  // we need to honor the timeout specified, the timeout include cmake, build
  // and test time
  auto clock_start = std::chrono::steady_clock::now();

  // make sure the binary dir is there
  out << "Internal cmake changing into directory: " << this->BinaryDir
      << std::endl;
  if (!cmSystemTools::FileIsDirectory(this->BinaryDir)) {
    cmSystemTools::MakeDirectory(this->BinaryDir);
  }
  cmWorkingDirectory workdir(this->BinaryDir);
  if (workdir.Failed()) {
    auto msg = "Failed to change working directory to " + this->BinaryDir +
      " : " + std::strerror(workdir.GetLastResult()) + "\n";
    this->Output = msg;
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
    if (this->RunCMake(out, cmakeOutString, &cm)) {
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
        this->Output = "--build-and-test timeout exceeded. ";
        return 1;
      }
    }
    const char* config = nullptr;
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
      this->BuildProject, { tar }, out, this->BuildMakeProgram, config,
      buildOptions, false, remainingTime);
    // if the build failed then return
    if (retVal) {
      this->Output = out.str();
      return 1;
    }
  }
  this->Output = out.str();

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
    out << "Could not find path to executable, perhaps it was not built: "
        << this->TestCommand << "\n";
    out << "tried to find it in these places:\n";
    out << fullPath << "\n";
    for (std::string const& fail : failed) {
      out << fail << "\n";
    }
    this->Output = out.str();
    return 1;
  }

  std::vector<std::string> testCommand;
  testCommand.push_back(fullPath);
  for (std::string const& testCommandArg : this->TestCommandArgs) {
    testCommand.push_back(testCommandArg);
  }
  std::string outs;
  int retval = 0;
  // run the test from the this->BuildRunDir if set
  if (!this->BuildRunDir.empty()) {
    out << "Run test in directory: " << this->BuildRunDir << "\n";
    if (!workdir.SetDirectory(this->BuildRunDir)) {
      out << "Failed to change working directory : "
          << std::strerror(workdir.GetLastResult()) << "\n";
      this->Output = out.str();
      return 1;
    }
  }
  out << "Running test command: \"" << fullPath << "\"";
  for (std::string const& testCommandArg : this->TestCommandArgs) {
    out << " \"" << testCommandArg << "\"";
  }
  out << "\n";

  // how much time is remaining
  cmDuration remainingTime = std::chrono::seconds(0);
  if (this->Timeout > cmDuration::zero()) {
    remainingTime =
      this->Timeout - (std::chrono::steady_clock::now() - clock_start);
    if (remainingTime <= std::chrono::seconds(0)) {
      this->Output = "--build-and-test timeout exceeded. ";
      return 1;
    }
  }

  bool runTestRes =
    this->CTest->RunTest(testCommand, &outs, &retval, remainingTime, nullptr);

  if (!runTestRes || retval != 0) {
    out << "Test command failed: " << testCommand[0] << "\n";
    retval = 1;
  }

  out << outs << "\n";
  this->Output = out.str();
  return retval;
}
