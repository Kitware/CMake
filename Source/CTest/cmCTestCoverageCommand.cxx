/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmCTestCoverageCommand.h"

#include <set>

#include "cm_static_string_view.hxx"

#include "cmAlgorithms.h"
#include "cmCTest.h"
#include "cmCTestCoverageHandler.h"

class cmCTestGenericHandler;

void cmCTestCoverageCommand::BindArguments()
{
  this->cmCTestHandlerCommand::BindArguments();
  this->Bind("LABELS"_s, this->Labels);
}

void cmCTestCoverageCommand::CheckArguments(
  std::vector<std::string> const& keywords)
{
  this->LabelsMentioned =
    !this->Labels.empty() || cmContains(keywords, "LABELS");
}

cmCTestGenericHandler* cmCTestCoverageCommand::InitializeHandler()
{
  this->CTest->SetCTestConfigurationFromCMakeVariable(
    this->Makefile, "CoverageCommand", "CTEST_COVERAGE_COMMAND", this->Quiet);
  this->CTest->SetCTestConfigurationFromCMakeVariable(
    this->Makefile, "CoverageExtraFlags", "CTEST_COVERAGE_EXTRA_FLAGS",
    this->Quiet);
  cmCTestCoverageHandler* handler = this->CTest->GetCoverageHandler();
  handler->Initialize();

  // If a LABELS option was given, select only files with the labels.
  if (this->LabelsMentioned) {
    handler->SetLabelFilter(
      std::set<std::string>(this->Labels.begin(), this->Labels.end()));
  }

  handler->SetQuiet(this->Quiet);
  return handler;
}
