/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>
#include <vector>

#include "cmCTestHandlerCommand.h"

class cmExecutionStatus;

class cmCTestUpdateCommand : public cmCTestHandlerCommand
{
public:
  using cmCTestHandlerCommand::cmCTestHandlerCommand;

protected:
  struct UpdateArguments : BasicArguments
  {
    std::string Source;
    std::string ReturnValue;
    bool Quiet = false;
  };

private:
  std::string GetName() const override { return "ctest_update"; }

  bool ExecuteUpdate(UpdateArguments& args, cmExecutionStatus& status) const;

  bool InitialPass(std::vector<std::string> const& args,
                   cmExecutionStatus& status) const override;
};
