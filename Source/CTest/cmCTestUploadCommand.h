/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <memory>
#include <string>
#include <vector>

#include "cmArgumentParserTypes.h"
#include "cmCTestHandlerCommand.h"

class cmCTestGenericHandler;
class cmCommand;

class cmCTestUploadCommand : public cmCTestHandlerCommand
{
public:
  using cmCTestHandlerCommand::cmCTestHandlerCommand;

protected:
  void BindArguments() override;
  ArgumentParser::MaybeEmpty<std::vector<std::string>> Files;

private:
  std::unique_ptr<cmCommand> Clone() override;

  std::string GetName() const override { return "ctest_upload"; }

  void CheckArguments() override;

  std::unique_ptr<cmCTestGenericHandler> InitializeHandler() override;
};
