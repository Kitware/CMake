/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>
#include <vector>

#include "cmCTestHandlerCommand.h"

class cmExecutionStatus;

class cmCTestConfigureCommand : public cmCTestHandlerCommand
{
public:
  using cmCTestHandlerCommand::cmCTestHandlerCommand;

protected:
  struct ConfigureArguments : HandlerArguments
  {
    std::string Options;
  };

private:
  std::string GetName() const override { return "ctest_configure"; }

  bool ExecuteConfigure(ConfigureArguments const& args,
                        cmExecutionStatus& status) const;

  bool InitialPass(std::vector<std::string> const& args,
                   cmExecutionStatus& status) const override;
};
