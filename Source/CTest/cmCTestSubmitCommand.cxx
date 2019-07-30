/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmCTestSubmitCommand.h"

#include "cmCTest.h"
#include "cmCTestSubmitHandler.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmSystemTools.h"

#include <sstream>

class cmExecutionStatus;

cmCTestSubmitCommand::cmCTestSubmitCommand()
{
  this->PartsMentioned = false;
  this->FilesMentioned = false;
  this->InternalTest = false;
  this->RetryCount = "";
  this->RetryDelay = "";
  this->CDashUpload = false;
  this->Arguments[cts_BUILD_ID] = "BUILD_ID";
  this->Last = cts_LAST;
}

/**
 * This is a virtual constructor for the command.
 */
cmCommand* cmCTestSubmitCommand::Clone()
{
  cmCTestSubmitCommand* ni = new cmCTestSubmitCommand;
  ni->CTest = this->CTest;
  ni->CTestScriptHandler = this->CTestScriptHandler;
  return ni;
}

cmCTestGenericHandler* cmCTestSubmitCommand::InitializeHandler()
{
  const char* submitURL = !this->SubmitURL.empty()
    ? this->SubmitURL.c_str()
    : this->Makefile->GetDefinition("CTEST_SUBMIT_URL");

  if (submitURL) {
    this->CTest->SetCTestConfiguration("SubmitURL", submitURL, this->Quiet);
  } else {
    this->CTest->SetCTestConfigurationFromCMakeVariable(
      this->Makefile, "DropMethod", "CTEST_DROP_METHOD", this->Quiet);
    this->CTest->SetCTestConfigurationFromCMakeVariable(
      this->Makefile, "DropSiteUser", "CTEST_DROP_SITE_USER", this->Quiet);
    this->CTest->SetCTestConfigurationFromCMakeVariable(
      this->Makefile, "DropSitePassword", "CTEST_DROP_SITE_PASSWORD",
      this->Quiet);
    this->CTest->SetCTestConfigurationFromCMakeVariable(
      this->Makefile, "DropSite", "CTEST_DROP_SITE", this->Quiet);
    this->CTest->SetCTestConfigurationFromCMakeVariable(
      this->Makefile, "DropLocation", "CTEST_DROP_LOCATION", this->Quiet);
  }

  this->CTest->SetCTestConfigurationFromCMakeVariable(
    this->Makefile, "CurlOptions", "CTEST_CURL_OPTIONS", this->Quiet);

  const char* notesFilesVariable =
    this->Makefile->GetDefinition("CTEST_NOTES_FILES");
  if (notesFilesVariable) {
    std::vector<std::string> notesFiles;
    cmSystemTools::ExpandListArgument(notesFilesVariable, notesFiles);
    this->CTest->GenerateNotesFile(notesFiles);
  }

  const char* extraFilesVariable =
    this->Makefile->GetDefinition("CTEST_EXTRA_SUBMIT_FILES");
  if (extraFilesVariable) {
    std::vector<std::string> extraFiles;
    cmSystemTools::ExpandListArgument(extraFilesVariable, extraFiles);
    if (!this->CTest->SubmitExtraFiles(extraFiles)) {
      this->SetError("problem submitting extra files.");
      return nullptr;
    }
  }

  cmCTestSubmitHandler* handler = this->CTest->GetSubmitHandler();
  handler->Initialize();

  // If no FILES or PARTS given, *all* PARTS are submitted by default.
  //
  // If FILES are given, but not PARTS, only the FILES are submitted
  // and *no* PARTS are submitted.
  //  (This is why we select the empty "noParts" set in the
  //   FilesMentioned block below...)
  //
  // If PARTS are given, only the selected PARTS are submitted.
  //
  // If both PARTS and FILES are given, only the selected PARTS *and*
  // all the given FILES are submitted.

  // If given explicit FILES to submit, pass them to the handler.
  //
  if (this->FilesMentioned) {
    // Intentionally select *no* PARTS. (Pass an empty set.) If PARTS
    // were also explicitly mentioned, they will be selected below...
    // But FILES with no PARTS mentioned should just submit the FILES
    // without any of the default parts.
    //
    handler->SelectParts(std::set<cmCTest::Part>());
    handler->SelectFiles(this->Files);
  }

  // If a PARTS option was given, select only the named parts for submission.
  //
  if (this->PartsMentioned) {
    handler->SelectParts(this->Parts);
  }

  // Pass along any HTTPHEADER to the handler if this option was given.
  if (!this->HttpHeaders.empty()) {
    handler->SetHttpHeaders(this->HttpHeaders);
  }

  handler->SetOption("RetryDelay", this->RetryDelay.c_str());
  handler->SetOption("RetryCount", this->RetryCount.c_str());
  handler->SetOption("InternalTest", this->InternalTest ? "ON" : "OFF");

  handler->SetQuiet(this->Quiet);

  if (this->CDashUpload) {
    handler->SetOption("CDashUploadFile", this->CDashUploadFile.c_str());
    handler->SetOption("CDashUploadType", this->CDashUploadType.c_str());
  }
  return handler;
}

bool cmCTestSubmitCommand::InitialPass(std::vector<std::string> const& args,
                                       cmExecutionStatus& status)
{
  this->CDashUpload = !args.empty() && args[0] == "CDASH_UPLOAD";

  bool ret = this->cmCTestHandlerCommand::InitialPass(args, status);

  if (this->Values[cts_BUILD_ID] && *this->Values[cts_BUILD_ID]) {
    this->Makefile->AddDefinition(this->Values[cts_BUILD_ID],
                                  this->CTest->GetBuildID().c_str());
  }

  return ret;
}

bool cmCTestSubmitCommand::CheckArgumentKeyword(std::string const& arg)
{
  if (this->CDashUpload) {
    // Arguments specific to the CDASH_UPLOAD signature.
    if (arg == "CDASH_UPLOAD") {
      this->ArgumentDoing = ArgumentDoingCDashUpload;
      return true;
    }

    if (arg == "CDASH_UPLOAD_TYPE") {
      this->ArgumentDoing = ArgumentDoingCDashUploadType;
      return true;
    }
  } else {
    // Arguments that cannot be used with CDASH_UPLOAD.
    if (arg == "PARTS") {
      this->ArgumentDoing = ArgumentDoingParts;
      this->PartsMentioned = true;
      return true;
    }

    if (arg == "FILES") {
      this->ArgumentDoing = ArgumentDoingFiles;
      this->FilesMentioned = true;
      return true;
    }
  }
  // Arguments used by both modes.
  if (arg == "HTTPHEADER") {
    this->ArgumentDoing = ArgumentDoingHttpHeader;
    return true;
  }

  if (arg == "RETRY_COUNT") {
    this->ArgumentDoing = ArgumentDoingRetryCount;
    return true;
  }

  if (arg == "RETRY_DELAY") {
    this->ArgumentDoing = ArgumentDoingRetryDelay;
    return true;
  }

  if (arg == "SUBMIT_URL") {
    this->ArgumentDoing = ArgumentDoingSubmitURL;
    return true;
  }

  if (arg == "INTERNAL_TEST_CHECKSUM") {
    this->InternalTest = true;
    return true;
  }

  // Look for other arguments.
  return this->Superclass::CheckArgumentKeyword(arg);
}

bool cmCTestSubmitCommand::CheckArgumentValue(std::string const& arg)
{
  // Handle states specific to this command.
  if (this->ArgumentDoing == ArgumentDoingParts) {
    cmCTest::Part p = this->CTest->GetPartFromName(arg.c_str());
    if (p != cmCTest::PartCount) {
      this->Parts.insert(p);
    } else {
      std::ostringstream e;
      e << "Part name \"" << arg << "\" is invalid.";
      this->Makefile->IssueMessage(MessageType::FATAL_ERROR, e.str());
      this->ArgumentDoing = ArgumentDoingError;
    }
    return true;
  }

  if (this->ArgumentDoing == ArgumentDoingFiles) {
    if (cmSystemTools::FileExists(arg)) {
      this->Files.insert(arg);
    } else {
      std::ostringstream e;
      e << "File \"" << arg << "\" does not exist. Cannot submit "
        << "a non-existent file.";
      this->Makefile->IssueMessage(MessageType::FATAL_ERROR, e.str());
      this->ArgumentDoing = ArgumentDoingError;
    }
    return true;
  }

  if (this->ArgumentDoing == ArgumentDoingHttpHeader) {
    this->HttpHeaders.push_back(arg);
    return true;
  }

  if (this->ArgumentDoing == ArgumentDoingRetryCount) {
    this->RetryCount = arg;
    return true;
  }

  if (this->ArgumentDoing == ArgumentDoingRetryDelay) {
    this->RetryDelay = arg;
    return true;
  }

  if (this->ArgumentDoing == ArgumentDoingCDashUpload) {
    this->ArgumentDoing = ArgumentDoingNone;
    this->CDashUploadFile = arg;
    return true;
  }

  if (this->ArgumentDoing == ArgumentDoingCDashUploadType) {
    this->ArgumentDoing = ArgumentDoingNone;
    this->CDashUploadType = arg;
    return true;
  }

  if (this->ArgumentDoing == ArgumentDoingSubmitURL) {
    this->ArgumentDoing = ArgumentDoingNone;
    this->SubmitURL = arg;
    return true;
  }

  // Look for other arguments.
  return this->Superclass::CheckArgumentValue(arg);
}
