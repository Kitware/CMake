/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmCTestMemCheckCommand.h"

#include <utility>

#include <cm/memory>
#include <cmext/string_view>

#include "cmArgumentParser.h"
#include "cmCTest.h"
#include "cmCTestMemCheckHandler.h"
#include "cmCTestTestHandler.h"
#include "cmExecutionStatus.h"
#include "cmMakefile.h"

std::unique_ptr<cmCTestTestHandler>
cmCTestMemCheckCommand::InitializeActualHandler(
  HandlerArguments& args, cmExecutionStatus& status) const
{
  cmMakefile& mf = status.GetMakefile();
  auto handler = cm::make_unique<cmCTestMemCheckHandler>(this->CTest);

  this->CTest->SetCTestConfigurationFromCMakeVariable(
    &mf, "MemoryCheckType", "CTEST_MEMORYCHECK_TYPE", args.Quiet);
  this->CTest->SetCTestConfigurationFromCMakeVariable(
    &mf, "MemoryCheckSanitizerOptions", "CTEST_MEMORYCHECK_SANITIZER_OPTIONS",
    args.Quiet);
  this->CTest->SetCTestConfigurationFromCMakeVariable(
    &mf, "MemoryCheckCommand", "CTEST_MEMORYCHECK_COMMAND", args.Quiet);
  this->CTest->SetCTestConfigurationFromCMakeVariable(
    &mf, "MemoryCheckCommandOptions", "CTEST_MEMORYCHECK_COMMAND_OPTIONS",
    args.Quiet);
  this->CTest->SetCTestConfigurationFromCMakeVariable(
    &mf, "MemoryCheckSuppressionFile", "CTEST_MEMORYCHECK_SUPPRESSIONS_FILE",
    args.Quiet);

  handler->SetQuiet(args.Quiet);
  return std::unique_ptr<cmCTestTestHandler>(std::move(handler));
}

void cmCTestMemCheckCommand::ProcessAdditionalValues(
  cmCTestGenericHandler* handler, HandlerArguments const& arguments,
  cmExecutionStatus& status) const
{
  cmMakefile& mf = status.GetMakefile();
  auto const& args = static_cast<MemCheckArguments const&>(arguments);
  if (!args.DefectCount.empty()) {
    mf.AddDefinition(
      args.DefectCount,
      std::to_string(
        static_cast<cmCTestMemCheckHandler*>(handler)->GetDefectCount()));
  }
}

bool cmCTestMemCheckCommand::InitialPass(std::vector<std::string> const& args,
                                         cmExecutionStatus& status) const
{
  static auto const parser =
    cmArgumentParser<MemCheckArguments>{ MakeTestParser<MemCheckArguments>() }
      .Bind("DEFECT_COUNT"_s, &MemCheckArguments::DefectCount);

  return this->Invoke(parser, args, status, [&](MemCheckArguments& a) {
    return this->ExecuteHandlerCommand(a, status);
  });
}
