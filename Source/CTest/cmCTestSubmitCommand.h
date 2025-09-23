/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <memory>
#include <string>
#include <vector>

#include <cm/optional>

#include "cmArgumentParserTypes.h"
#include "cmCTestHandlerCommand.h"

class cmExecutionStatus;
class cmCTestGenericHandler;

class cmCTestSubmitCommand : public cmCTestHandlerCommand
{
public:
  using cmCTestHandlerCommand::cmCTestHandlerCommand;

protected:
  struct SubmitArguments : HandlerArguments
  {
    bool CDashUpload = false;
    bool InternalTest = false;

    std::string BuildID;
    std::string CDashUploadFile;
    std::string CDashUploadType;
    std::string RetryCount;
    std::string RetryDelay;
    std::string SubmitURL;

    cm::optional<ArgumentParser::MaybeEmpty<std::vector<std::string>>> Files;
    ArgumentParser::MaybeEmpty<std::vector<std::string>> HttpHeaders;
    cm::optional<ArgumentParser::MaybeEmpty<std::vector<std::string>>> Parts;
  };

private:
  std::string GetName() const override { return "ctest_submit"; }

  void CheckArguments(HandlerArguments& arguments,
                      cmExecutionStatus& status) const override;

  std::unique_ptr<cmCTestGenericHandler> InitializeHandler(
    HandlerArguments& arguments, cmExecutionStatus& status) const override;

  void ProcessAdditionalValues(cmCTestGenericHandler* handler,
                               HandlerArguments const& arguments,
                               cmExecutionStatus& status) const override;

  bool InitialPass(std::vector<std::string> const& args,
                   cmExecutionStatus& status) const override;
};
