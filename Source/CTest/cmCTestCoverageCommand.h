/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <memory>
#include <string>
#include <vector>

#include <cm/optional>

#include "cmArgumentParserTypes.h" // IWYU pragma: keep
#include "cmCTestHandlerCommand.h"

class cmExecutionStatus;
class cmCTestGenericHandler;
class cmCommand;

class cmCTestCoverageCommand : public cmCTestHandlerCommand
{
public:
  using cmCTestHandlerCommand::cmCTestHandlerCommand;

protected:
  struct CoverageArguments : HandlerArguments
  {
    cm::optional<ArgumentParser::MaybeEmpty<std::vector<std::string>>> Labels;
  };

private:
  std::unique_ptr<cmCommand> Clone() override;

  std::string GetName() const override { return "ctest_coverage"; }

  std::unique_ptr<cmCTestGenericHandler> InitializeHandler(
    HandlerArguments& arguments, cmExecutionStatus& status) override;

  bool InitialPass(std::vector<std::string> const& args,
                   cmExecutionStatus& status) override;
};
