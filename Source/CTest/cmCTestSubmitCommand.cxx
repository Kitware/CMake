/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmCTestSubmitCommand.h"

#include <set>
#include <sstream>
#include <utility>

#include <cm/memory>
#include <cm/vector>

#include "cm_static_string_view.hxx"

#include "cmAlgorithms.h"
#include "cmCTest.h"
#include "cmCTestSubmitHandler.h"
#include "cmCommand.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmRange.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"

class cmExecutionStatus;

/**
 * This is a virtual constructor for the command.
 */
std::unique_ptr<cmCommand> cmCTestSubmitCommand::Clone()
{
  auto ni = cm::make_unique<cmCTestSubmitCommand>();
  ni->CTest = this->CTest;
  ni->CTestScriptHandler = this->CTestScriptHandler;
  return std::unique_ptr<cmCommand>(std::move(ni));
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
    std::vector<std::string> notesFiles = cmExpandedList(notesFilesVariable);
    this->CTest->GenerateNotesFile(notesFiles);
  }

  const char* extraFilesVariable =
    this->Makefile->GetDefinition("CTEST_EXTRA_SUBMIT_FILES");
  if (extraFilesVariable) {
    std::vector<std::string> extraFiles = cmExpandedList(extraFilesVariable);
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
    handler->SelectFiles(
      std::set<std::string>(this->Files.begin(), this->Files.end()));
  }

  // If a PARTS option was given, select only the named parts for submission.
  //
  if (this->PartsMentioned) {
    auto parts =
      cmMakeRange(this->Parts).transform([this](std::string const& arg) {
        return this->CTest->GetPartFromName(arg.c_str());
      });
    handler->SelectParts(std::set<cmCTest::Part>(parts.begin(), parts.end()));
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

  if (!this->BuildID.empty()) {
    this->Makefile->AddDefinition(this->BuildID, this->CTest->GetBuildID());
  }

  return ret;
}

void cmCTestSubmitCommand::BindArguments()
{
  if (this->CDashUpload) {
    // Arguments specific to the CDASH_UPLOAD signature.
    this->Bind("CDASH_UPLOAD", this->CDashUploadFile);
    this->Bind("CDASH_UPLOAD_TYPE", this->CDashUploadType);
  } else {
    // Arguments that cannot be used with CDASH_UPLOAD.
    this->Bind("PARTS"_s, this->Parts);
    this->Bind("FILES"_s, this->Files);
  }
  // Arguments used by both modes.
  this->Bind("BUILD_ID"_s, this->BuildID);
  this->Bind("HTTPHEADER"_s, this->HttpHeaders);
  this->Bind("RETRY_COUNT"_s, this->RetryCount);
  this->Bind("RETRY_DELAY"_s, this->RetryDelay);
  this->Bind("SUBMIT_URL"_s, this->SubmitURL);
  this->Bind("INTERNAL_TEST_CHECKSUM", this->InternalTest);

  // Look for other arguments.
  this->cmCTestHandlerCommand::BindArguments();
}

void cmCTestSubmitCommand::CheckArguments(
  std::vector<std::string> const& keywords)
{
  this->PartsMentioned = !this->Parts.empty() || cmContains(keywords, "PARTS");
  this->FilesMentioned = !this->Files.empty() || cmContains(keywords, "FILES");

  cm::erase_if(this->Parts, [this](std::string const& arg) -> bool {
    cmCTest::Part p = this->CTest->GetPartFromName(arg.c_str());
    if (p == cmCTest::PartCount) {
      std::ostringstream e;
      e << "Part name \"" << arg << "\" is invalid.";
      this->Makefile->IssueMessage(MessageType::FATAL_ERROR, e.str());
      return true;
    }
    return false;
  });

  cm::erase_if(this->Files, [this](std::string const& arg) -> bool {
    if (!cmSystemTools::FileExists(arg)) {
      std::ostringstream e;
      e << "File \"" << arg << "\" does not exist. Cannot submit "
        << "a non-existent file.";
      this->Makefile->IssueMessage(MessageType::FATAL_ERROR, e.str());
      return true;
    }
    return false;
  });
}
