/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmCTestMemCheckCommand.h"

#include <utility>

#include <cm/memory>
#include <cmext/string_view>

#include "cmCTest.h"
#include "cmCTestMemCheckHandler.h"
#include "cmCTestTestHandler.h"
#include "cmCommand.h"
#include "cmMakefile.h"

std::unique_ptr<cmCommand> cmCTestMemCheckCommand::Clone()
{
  auto ni = cm::make_unique<cmCTestMemCheckCommand>();
  ni->CTest = this->CTest;
  return std::unique_ptr<cmCommand>(std::move(ni));
}

void cmCTestMemCheckCommand::BindArguments()
{
  this->cmCTestTestCommand::BindArguments();
  this->Bind("DEFECT_COUNT"_s, this->DefectCount);
}

std::unique_ptr<cmCTestTestHandler>
cmCTestMemCheckCommand::InitializeActualHandler()
{
  auto const& args = *this;
  auto handler = cm::make_unique<cmCTestMemCheckHandler>(this->CTest);

  this->CTest->SetCTestConfigurationFromCMakeVariable(
    this->Makefile, "MemoryCheckType", "CTEST_MEMORYCHECK_TYPE", args.Quiet);
  this->CTest->SetCTestConfigurationFromCMakeVariable(
    this->Makefile, "MemoryCheckSanitizerOptions",
    "CTEST_MEMORYCHECK_SANITIZER_OPTIONS", args.Quiet);
  this->CTest->SetCTestConfigurationFromCMakeVariable(
    this->Makefile, "MemoryCheckCommand", "CTEST_MEMORYCHECK_COMMAND",
    args.Quiet);
  this->CTest->SetCTestConfigurationFromCMakeVariable(
    this->Makefile, "MemoryCheckCommandOptions",
    "CTEST_MEMORYCHECK_COMMAND_OPTIONS", args.Quiet);
  this->CTest->SetCTestConfigurationFromCMakeVariable(
    this->Makefile, "MemoryCheckSuppressionFile",
    "CTEST_MEMORYCHECK_SUPPRESSIONS_FILE", args.Quiet);

  handler->SetQuiet(args.Quiet);
  return std::unique_ptr<cmCTestTestHandler>(std::move(handler));
}

void cmCTestMemCheckCommand::ProcessAdditionalValues(
  cmCTestGenericHandler* handler)
{
  auto const& args = *this;
  if (!args.DefectCount.empty()) {
    this->Makefile->AddDefinition(
      args.DefectCount,
      std::to_string(
        static_cast<cmCTestMemCheckHandler*>(handler)->GetDefectCount()));
  }
}
