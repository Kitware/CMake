/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmCTestCoverageCommand.h"

#include <set>

#include <cmext/string_view>

#include "cmCTest.h"
#include "cmCTestCoverageHandler.h"
#include "cmCTestGenericHandler.h"

void cmCTestCoverageCommand::BindArguments()
{
  this->cmCTestHandlerCommand::BindArguments();
  this->Bind("LABELS"_s, this->Labels);
}

std::unique_ptr<cmCTestGenericHandler>
cmCTestCoverageCommand::InitializeHandler()
{
  this->CTest->SetCTestConfigurationFromCMakeVariable(
    this->Makefile, "CoverageCommand", "CTEST_COVERAGE_COMMAND", this->Quiet);
  this->CTest->SetCTestConfigurationFromCMakeVariable(
    this->Makefile, "CoverageExtraFlags", "CTEST_COVERAGE_EXTRA_FLAGS",
    this->Quiet);
  auto handler = cm::make_unique<cmCTestCoverageHandler>(this->CTest);

  // If a LABELS option was given, select only files with the labels.
  if (this->Labels) {
    handler->SetLabelFilter(
      std::set<std::string>(this->Labels->begin(), this->Labels->end()));
  }

  handler->SetQuiet(this->Quiet);
  return std::unique_ptr<cmCTestGenericHandler>(std::move(handler));
}
