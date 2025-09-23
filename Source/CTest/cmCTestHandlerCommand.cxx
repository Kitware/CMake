/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmCTestHandlerCommand.h"

#include <algorithm>
#include <cstdlib>
#include <sstream>

#include <cm/string_view>

#include "cmCTest.h"
#include "cmCTestGenericHandler.h"
#include "cmExecutionStatus.h"
#include "cmMakefile.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmValue.h"
#include "cmWorkingDirectory.h"

namespace {
// class to save and restore the error state for ctest_* commands
// if a ctest_* command has a CAPTURE_CMAKE_ERROR then put the error
// state into there and restore the system wide error to what
// it was before the command ran
class SaveRestoreErrorState
{
public:
  SaveRestoreErrorState()
  {
    this->InitialErrorState = cmSystemTools::GetErrorOccurredFlag();
    cmSystemTools::ResetErrorOccurredFlag(); // rest the error state
    this->CaptureCMakeErrorValue = false;
  }
  // if the function has a CAPTURE_CMAKE_ERROR then we should restore
  // the error state to what it was before the function was run
  // if not then let the error state be what it is
  void CaptureCMakeError() { this->CaptureCMakeErrorValue = true; }
  ~SaveRestoreErrorState()
  {
    // if we are not saving the return value then make sure
    // if it was in error it goes back to being in error
    // otherwise leave it be what it is
    if (!this->CaptureCMakeErrorValue) {
      if (this->InitialErrorState) {
        cmSystemTools::SetErrorOccurred();
      }
      return;
    }
    // if we have saved the error in a return variable
    // then put things back exactly like they were
    bool currentState = cmSystemTools::GetErrorOccurredFlag();
    // if the state changed during this command we need
    // to handle it, if not then nothing needs to be done
    if (currentState != this->InitialErrorState) {
      // restore the initial error state
      if (this->InitialErrorState) {
        cmSystemTools::SetErrorOccurred();
      } else {
        cmSystemTools::ResetErrorOccurredFlag();
      }
    }
  }
  SaveRestoreErrorState(SaveRestoreErrorState const&) = delete;
  SaveRestoreErrorState& operator=(SaveRestoreErrorState const&) = delete;

private:
  bool InitialErrorState;
  bool CaptureCMakeErrorValue;
};
}

bool cmCTestHandlerCommand::InvokeImpl(
  BasicArguments& args, std::vector<std::string> const& unparsed,
  cmExecutionStatus& status, std::function<bool()> handler) const
{
  // save error state and restore it if needed
  SaveRestoreErrorState errorState;
  if (!args.CaptureCMakeError.empty()) {
    errorState.CaptureCMakeError();
  }

  bool success = [&]() -> bool {
    if (args.MaybeReportError(status.GetMakefile())) {
      return true;
    }

    std::sort(args.ParsedKeywords.begin(), args.ParsedKeywords.end());
    auto const it = std::adjacent_find(args.ParsedKeywords.begin(),
                                       args.ParsedKeywords.end());
    if (it != args.ParsedKeywords.end()) {
      status.SetError(cmStrCat("called with more than one value for ", *it));
      return false;
    }

    if (!unparsed.empty()) {
      status.SetError(
        cmStrCat("called with unknown argument \"", unparsed.front(), "\"."));
      return false;
    }

    return handler();
  }();

  if (args.CaptureCMakeError.empty()) {
    return success;
  }

  if (!success) {
    cmCTestLog(this->CTest, ERROR_MESSAGE,
               this->GetName() << ' ' << status.GetError() << '\n');
  }

  cmMakefile& mf = status.GetMakefile();
  success = success && !cmSystemTools::GetErrorOccurredFlag();
  mf.AddDefinition(args.CaptureCMakeError, success ? "0" : "-1");
  return true;
}

bool cmCTestHandlerCommand::ExecuteHandlerCommand(
  HandlerArguments& args, cmExecutionStatus& status) const
{
  cmMakefile& mf = status.GetMakefile();

  // Process input arguments.
  this->CheckArguments(args, status);

  // Set the config type of this ctest to the current value of the
  // CTEST_CONFIGURATION_TYPE script variable if it is defined.
  // The current script value trumps the -C argument on the command
  // line.
  cmValue ctestConfigType = mf.GetDefinition("CTEST_CONFIGURATION_TYPE");
  if (ctestConfigType) {
    this->CTest->SetConfigType(*ctestConfigType);
  }

  if (!args.Build.empty()) {
    this->CTest->SetCTestConfiguration(
      "BuildDirectory", cmSystemTools::CollapseFullPath(args.Build),
      args.Quiet);
  } else {
    std::string const& bdir = mf.GetSafeDefinition("CTEST_BINARY_DIRECTORY");
    if (!bdir.empty()) {
      this->CTest->SetCTestConfiguration(
        "BuildDirectory", cmSystemTools::CollapseFullPath(bdir), args.Quiet);
    } else {
      cmCTestLog(this->CTest, ERROR_MESSAGE,
                 "CTEST_BINARY_DIRECTORY not set" << std::endl);
    }
  }
  if (!args.Source.empty()) {
    cmCTestLog(this->CTest, DEBUG,
               "Set source directory to: " << args.Source << std::endl);
    this->CTest->SetCTestConfiguration(
      "SourceDirectory", cmSystemTools::CollapseFullPath(args.Source),
      args.Quiet);
  } else {
    this->CTest->SetCTestConfiguration(
      "SourceDirectory",
      cmSystemTools::CollapseFullPath(
        mf.GetSafeDefinition("CTEST_SOURCE_DIRECTORY")),
      args.Quiet);
  }

  if (cmValue changeId = mf.GetDefinition("CTEST_CHANGE_ID")) {
    this->CTest->SetCTestConfiguration("ChangeId", *changeId, args.Quiet);
  }

  cmCTestLog(this->CTest, DEBUG, "Initialize handler" << std::endl);
  auto handler = this->InitializeHandler(args, status);
  if (!handler) {
    cmCTestLog(this->CTest, ERROR_MESSAGE,
               "Cannot instantiate test handler " << this->GetName()
                                                  << std::endl);
    return false;
  }

  handler->SetAppendXML(args.Append);

  handler->PopulateCustomVectors(&mf);
  if (!args.SubmitIndex.empty()) {
    handler->SetSubmitIndex(atoi(args.SubmitIndex.c_str()));
  }
  cmWorkingDirectory workdir(
    this->CTest->GetCTestConfiguration("BuildDirectory"));
  if (workdir.Failed()) {
    status.SetError(workdir.GetError());
    return false;
  }

  // reread time limit, as the variable may have been modified.
  this->CTest->SetTimeLimit(mf.GetDefinition("CTEST_TIME_LIMIT"));
  handler->SetCMakeInstance(mf.GetCMakeInstance());

  int res = handler->ProcessHandler();
  if (!args.ReturnValue.empty()) {
    mf.AddDefinition(args.ReturnValue, std::to_string(res));
  }
  this->ProcessAdditionalValues(handler.get(), args, status);
  return true;
}

void cmCTestHandlerCommand::CheckArguments(HandlerArguments&,
                                           cmExecutionStatus&) const
{
}

std::unique_ptr<cmCTestGenericHandler>
cmCTestHandlerCommand::InitializeHandler(HandlerArguments&,
                                         cmExecutionStatus&) const
{
  return nullptr;
};

void cmCTestHandlerCommand::ProcessAdditionalValues(cmCTestGenericHandler*,
                                                    HandlerArguments const&,
                                                    cmExecutionStatus&) const
{
}
