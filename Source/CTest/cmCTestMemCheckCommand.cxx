/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmCTestMemCheckCommand.h"

#include <cmext/string_view>

#include "cmCTest.h"
#include "cmCTestMemCheckHandler.h"
#include "cmMakefile.h"

void cmCTestMemCheckCommand::BindArguments()
{
  this->cmCTestTestCommand::BindArguments();
  this->Bind("DEFECT_COUNT"_s, this->DefectCount);
}

cmCTestTestHandler* cmCTestMemCheckCommand::InitializeActualHandler()
{
  cmCTestMemCheckHandler* handler = this->CTest->GetMemCheckHandler();
  handler->Initialize();

  this->CTest->SetCTestConfigurationFromCMakeVariable(
    this->Makefile, "MemoryCheckType", "CTEST_MEMORYCHECK_TYPE", this->Quiet);
  this->CTest->SetCTestConfigurationFromCMakeVariable(
    this->Makefile, "MemoryCheckSanitizerOptions",
    "CTEST_MEMORYCHECK_SANITIZER_OPTIONS", this->Quiet);
  this->CTest->SetCTestConfigurationFromCMakeVariable(
    this->Makefile, "MemoryCheckCommand", "CTEST_MEMORYCHECK_COMMAND",
    this->Quiet);
  this->CTest->SetCTestConfigurationFromCMakeVariable(
    this->Makefile, "MemoryCheckCommandOptions",
    "CTEST_MEMORYCHECK_COMMAND_OPTIONS", this->Quiet);
  this->CTest->SetCTestConfigurationFromCMakeVariable(
    this->Makefile, "MemoryCheckSuppressionFile",
    "CTEST_MEMORYCHECK_SUPPRESSIONS_FILE", this->Quiet);

  handler->SetQuiet(this->Quiet);
  return handler;
}

void cmCTestMemCheckCommand::ProcessAdditionalValues(
  cmCTestGenericHandler* handler)
{
  if (!this->DefectCount.empty()) {
    this->Makefile->AddDefinition(
      this->DefectCount,
      std::to_string(
        static_cast<cmCTestMemCheckHandler*>(handler)->GetDefectCount()));
  }
}
