/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <memory>
#include <string>
#include <vector>

#include <cm/string_view>
#include <cmext/string_view>

#include "cmArgumentParser.h"
#include "cmCTestCommand.h"

class cmExecutionStatus;
class cmCTestGenericHandler;

class cmCTestHandlerCommand : public cmCTestCommand
{
public:
  using cmCTestCommand::cmCTestCommand;

protected:
  struct HandlerArguments
  {
    std::vector<cm::string_view> ParsedKeywords;
    bool Append = false;
    bool Quiet = false;
    std::string CaptureCMakeError;
    std::string ReturnValue;
    std::string Build;
    std::string Source;
    std::string SubmitIndex;
  };

  template <typename Args>
  static auto MakeHandlerParser() -> cmArgumentParser<Args>
  {
    return cmArgumentParser<Args>{}
      .BindParsedKeywords(&HandlerArguments::ParsedKeywords)
      .Bind("APPEND"_s, &HandlerArguments::Append)
      .Bind("QUIET"_s, &HandlerArguments::Quiet)
      .Bind("CAPTURE_CMAKE_ERROR"_s, &HandlerArguments::CaptureCMakeError)
      .Bind("RETURN_VALUE"_s, &HandlerArguments::ReturnValue)
      .Bind("SOURCE"_s, &HandlerArguments::Source)
      .Bind("BUILD"_s, &HandlerArguments::Build)
      .Bind("SUBMIT_INDEX"_s, &HandlerArguments::SubmitIndex);
  }

protected:
  bool ExecuteHandlerCommand(HandlerArguments& args,
                             std::vector<std::string> const& unparsedArguments,
                             cmExecutionStatus& status);

private:
  virtual std::string GetName() const = 0;

  virtual void CheckArguments(HandlerArguments& arguments);

  virtual std::unique_ptr<cmCTestGenericHandler> InitializeHandler(
    HandlerArguments& arguments);

  virtual void ProcessAdditionalValues(cmCTestGenericHandler*,
                                       HandlerArguments const& arguments);
};
