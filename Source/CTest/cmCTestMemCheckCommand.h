/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <memory>
#include <string>

#include "cmCTestTestCommand.h"

class cmCTestGenericHandler;
class cmCTestTestHandler;
class cmCommand;

class cmCTestMemCheckCommand : public cmCTestTestCommand
{
public:
  using cmCTestTestCommand::cmCTestTestCommand;

protected:
  void BindArguments() override;
  std::string DefectCount;

private:
  std::unique_ptr<cmCommand> Clone() override;

  std::string GetName() const override { return "ctest_memcheck"; }

  std::unique_ptr<cmCTestTestHandler> InitializeActualHandler() override;

  void ProcessAdditionalValues(cmCTestGenericHandler* handler) override;
};
