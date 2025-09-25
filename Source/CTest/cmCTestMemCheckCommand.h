/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <memory>
#include <string>
#include <vector>

#include "cmCTestTestCommand.h"

class cmExecutionStatus;
class cmCTestGenericHandler;
class cmCTestTestHandler;

class cmCTestMemCheckCommand : public cmCTestTestCommand
{
public:
  using cmCTestTestCommand::cmCTestTestCommand;

protected:
  struct MemCheckArguments : TestArguments
  {
    std::string DefectCount;
  };

private:
  std::string GetName() const override { return "ctest_memcheck"; }

  std::unique_ptr<cmCTestTestHandler> InitializeActualHandler(
    HandlerArguments& arguments, cmExecutionStatus& status) const override;

  void ProcessAdditionalValues(cmCTestGenericHandler* handler,
                               HandlerArguments const& arguments,
                               cmExecutionStatus& status) const override;

  bool InitialPass(std::vector<std::string> const& args,
                   cmExecutionStatus& status) const override;
};
