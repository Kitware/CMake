/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <memory>
#include <string>

#include "cmCTestHandlerCommand.h"

class cmCTestGenericHandler;
class cmCommand;

class cmCTestBuildCommand : public cmCTestHandlerCommand
{
public:
  using cmCTestHandlerCommand::cmCTestHandlerCommand;

protected:
  void BindArguments() override;
  std::string NumberErrors;
  std::string NumberWarnings;
  std::string Target;
  std::string Configuration;
  std::string Flags;
  std::string ProjectName;
  std::string ParallelLevel;

private:
  std::unique_ptr<cmCommand> Clone() override;

  std::string GetName() const override { return "ctest_build"; }

  std::unique_ptr<cmCTestGenericHandler> InitializeHandler() override;

  void ProcessAdditionalValues(cmCTestGenericHandler* handler) override;
};
