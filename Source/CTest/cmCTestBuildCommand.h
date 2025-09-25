/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <memory>
#include <string>
#include <vector>

#include "cmCTestHandlerCommand.h"

class cmExecutionStatus;
class cmCTestGenericHandler;

class cmCTestBuildCommand : public cmCTestHandlerCommand
{
public:
  using cmCTestHandlerCommand::cmCTestHandlerCommand;

protected:
  struct BuildArguments : HandlerArguments
  {
    std::string NumberErrors;
    std::string NumberWarnings;
    std::string Target;
    std::string Configuration;
    std::string Flags;
    std::string ProjectName;
    std::string ParallelLevel;
  };

private:
  std::string GetName() const override { return "ctest_build"; }

  std::unique_ptr<cmCTestGenericHandler> InitializeHandler(
    HandlerArguments& arguments, cmExecutionStatus& status) const override;

  void ProcessAdditionalValues(cmCTestGenericHandler* handler,
                               HandlerArguments const& arguments,
                               cmExecutionStatus& status) const override;

  bool InitialPass(std::vector<std::string> const& args,
                   cmExecutionStatus& status) const override;
};
