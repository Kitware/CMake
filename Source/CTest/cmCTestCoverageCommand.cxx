/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmCTestCoverageCommand.h"

#include <set>
#include <utility>

#include <cm/memory>
#include <cmext/string_view>

#include "cmCTest.h"
#include "cmCTestCoverageHandler.h"
#include "cmCTestGenericHandler.h"
#include "cmCommand.h"

std::unique_ptr<cmCommand> cmCTestCoverageCommand::Clone()
{
  auto ni = cm::make_unique<cmCTestCoverageCommand>();
  ni->CTest = this->CTest;
  return std::unique_ptr<cmCommand>(std::move(ni));
}

void cmCTestCoverageCommand::BindArguments()
{
  this->cmCTestHandlerCommand::BindArguments();
  this->Bind("LABELS"_s, this->Labels);
}

std::unique_ptr<cmCTestGenericHandler>
cmCTestCoverageCommand::InitializeHandler()
{
  auto const& args = *this;
  this->CTest->SetCTestConfigurationFromCMakeVariable(
    this->Makefile, "CoverageCommand", "CTEST_COVERAGE_COMMAND", args.Quiet);
  this->CTest->SetCTestConfigurationFromCMakeVariable(
    this->Makefile, "CoverageExtraFlags", "CTEST_COVERAGE_EXTRA_FLAGS",
    args.Quiet);
  auto handler = cm::make_unique<cmCTestCoverageHandler>(this->CTest);

  // If a LABELS option was given, select only files with the labels.
  if (args.Labels) {
    handler->SetLabelFilter(
      std::set<std::string>(args.Labels->begin(), args.Labels->end()));
  }

  handler->SetQuiet(args.Quiet);
  return std::unique_ptr<cmCTestGenericHandler>(std::move(handler));
}
