/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <functional>
#include <memory>
#include <string>
#include <type_traits>
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
  struct BasicArguments : ArgumentParser::ParseResult
  {
    std::string CaptureCMakeError;
    std::vector<cm::string_view> ParsedKeywords;
  };

  template <typename Args>
  static auto MakeBasicParser() -> cmArgumentParser<Args>
  {
    static_assert(std::is_base_of<BasicArguments, Args>::value, "");
    return cmArgumentParser<Args>{}
      .Bind("CAPTURE_CMAKE_ERROR"_s, &BasicArguments::CaptureCMakeError)
      .BindParsedKeywords(&BasicArguments::ParsedKeywords);
  }

  struct HandlerArguments : BasicArguments
  {
    bool Append = false;
    bool Quiet = false;
    std::string ReturnValue;
    std::string Build;
    std::string Source;
    std::string SubmitIndex;
  };

  template <typename Args>
  static auto MakeHandlerParser() -> cmArgumentParser<Args>
  {
    static_assert(std::is_base_of<HandlerArguments, Args>::value, "");
    return cmArgumentParser<Args>{ MakeBasicParser<Args>() }
      .Bind("APPEND"_s, &HandlerArguments::Append)
      .Bind("QUIET"_s, &HandlerArguments::Quiet)
      .Bind("RETURN_VALUE"_s, &HandlerArguments::ReturnValue)
      .Bind("SOURCE"_s, &HandlerArguments::Source)
      .Bind("BUILD"_s, &HandlerArguments::Build)
      .Bind("SUBMIT_INDEX"_s, &HandlerArguments::SubmitIndex);
  }

protected:
  template <typename Args, typename Handler>
  bool Invoke(cmArgumentParser<Args> const& parser,
              std::vector<std::string> const& arguments,
              cmExecutionStatus& status, Handler handler) const
  {
    std::vector<std::string> unparsed;
    Args args = parser.Parse(arguments, &unparsed);
    return this->InvokeImpl(args, unparsed, status,
                            [&]() -> bool { return handler(args); });
  };

  bool ExecuteHandlerCommand(HandlerArguments& args,
                             cmExecutionStatus& status) const;

private:
  bool InvokeImpl(BasicArguments& args,
                  std::vector<std::string> const& unparsed,
                  cmExecutionStatus& status,
                  std::function<bool()> handler) const;

  virtual std::string GetName() const = 0;

  virtual void CheckArguments(HandlerArguments& arguments,
                              cmExecutionStatus& status) const;

  virtual std::unique_ptr<cmCTestGenericHandler> InitializeHandler(
    HandlerArguments& arguments, cmExecutionStatus& status) const;

  virtual void ProcessAdditionalValues(cmCTestGenericHandler*,
                                       HandlerArguments const& arguments,
                                       cmExecutionStatus& status) const;
};
