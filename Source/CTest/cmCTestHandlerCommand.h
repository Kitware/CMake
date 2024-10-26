/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <memory>
#include <string>
#include <vector>

#include <cm/string_view>

#include "cmArgumentParser.h"
#include "cmCTestCommand.h"

class cmCTestGenericHandler;
class cmExecutionStatus;

class cmCTestHandlerCommand
  : public cmCTestCommand
  , public cmArgumentParser<void>
{
public:
  using cmCTestCommand::cmCTestCommand;

protected:
  virtual void BindArguments();
  std::vector<cm::string_view> ParsedKeywords;
  bool Append = false;
  bool Quiet = false;
  std::string CaptureCMakeError;
  std::string ReturnValue;
  std::string Build;
  std::string Source;
  std::string SubmitIndex;

protected:
  bool InitialPass(std::vector<std::string> const& args,
                   cmExecutionStatus& status) override;

private:
  virtual std::string GetName() const = 0;

  virtual void CheckArguments();

  virtual std::unique_ptr<cmCTestGenericHandler> InitializeHandler();

  virtual void ProcessAdditionalValues(cmCTestGenericHandler*);
};
