/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmCTestMemCheckCommand.h"

#include <utility>

#include <cm/memory>
#include <cmext/string_view>

#include "cmArgumentParser.h"
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

std::unique_ptr<cmCTestTestHandler>
cmCTestMemCheckCommand::InitializeActualHandler(HandlerArguments& args)
{
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
  cmCTestGenericHandler* handler, HandlerArguments const& arguments)
{
  auto const& args = static_cast<MemCheckArguments const&>(arguments);
  if (!args.DefectCount.empty()) {
    this->Makefile->AddDefinition(
      args.DefectCount,
      std::to_string(
        static_cast<cmCTestMemCheckHandler*>(handler)->GetDefectCount()));
  }
}

bool cmCTestMemCheckCommand::InitialPass(std::vector<std::string> const& args,
                                         cmExecutionStatus& status)
{
  static auto const parser =
    cmArgumentParser<MemCheckArguments>{ MakeTestParser<MemCheckArguments>() }
      .Bind("DEFECT_COUNT"_s, &MemCheckArguments::DefectCount);

  std::vector<std::string> unparsedArguments;
  MemCheckArguments arguments = parser.Parse(args, &unparsedArguments);
  return this->ExecuteHandlerCommand(arguments, unparsedArguments, status);
}
