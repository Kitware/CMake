/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmMathCommand.h"

#include <cstdio>
#include <limits>
#include <stdexcept>

#include <cm/string_view>
#include <cmext/string_view>

#include <cm3p/kwiml/int.h>

#include "cmDiagnostics.h"
#include "cmExecutionStatus.h"
#include "cmExprParserHelper.h"
#include "cmMakefile.h"
#include "cmStringAlgorithms.h"
#include "cmValue.h"

namespace {
bool HandleExprCommand(std::vector<std::string> const& args,
                       cmExecutionStatus& status);
bool HandleIncDecCommand(std::vector<std::string> const& args,
                         cmExecutionStatus& status, int amount,
                         long long overflowFrom, long long overflowTo,
                         cm::string_view verbing);
}

bool cmMathCommand(std::vector<std::string> const& args,
                   cmExecutionStatus& status)
{
  if (args.empty()) {
    status.SetError("must be called with at least one argument.");
    return false;
  }
  std::string const& subCommand = args[0];
  if (subCommand == "EXPR") {
    return HandleExprCommand(args, status);
  }
  if (subCommand == "INCREMENT") {
    return HandleIncDecCommand(
      args, status, 1, std::numeric_limits<long long>::max(),
      std::numeric_limits<long long>::min(), "incrementing"_s);
  }
  if (subCommand == "DECREMENT") {
    return HandleIncDecCommand(
      args, status, -1, std::numeric_limits<long long>::min(),
      std::numeric_limits<long long>::max(), "decrementing"_s);
  }
  status.SetError(cmStrCat("does not recognize sub-command ", subCommand));
  return false;
}

namespace {
bool HandleExprCommand(std::vector<std::string> const& args,
                       cmExecutionStatus& status)
{
  if ((args.size() != 3) && (args.size() != 5)) {
    status.SetError("EXPR called with incorrect arguments.");
    return false;
  }

  enum class NumericFormat
  {
    UNINITIALIZED,
    DECIMAL,
    HEXADECIMAL,
  };

  std::string const& outputVariable = args[1];
  std::string const& expression = args[2];
  size_t argumentIndex = 3;
  NumericFormat outputFormat = NumericFormat::UNINITIALIZED;

  status.GetMakefile().AddDefinition(outputVariable, "ERROR");

  if (argumentIndex < args.size()) {
    std::string const messageHint = "sub-command EXPR ";
    std::string const& option = args[argumentIndex++];
    if (option == "OUTPUT_FORMAT") {
      if (argumentIndex < args.size()) {
        std::string const& argument = args[argumentIndex++];
        if (argument == "DECIMAL") {
          outputFormat = NumericFormat::DECIMAL;
        } else if (argument == "HEXADECIMAL") {
          outputFormat = NumericFormat::HEXADECIMAL;
        } else {
          status.SetError(cmStrCat(messageHint, "value \"", argument,
                                   "\" for option \"", option,
                                   "\" is invalid."));
          return false;
        }
      } else {
        status.SetError(cmStrCat(messageHint, "missing argument for option \"",
                                 option, "\"."));
        return false;
      }
    } else {
      status.SetError(
        cmStrCat(messageHint, "option \"", option, "\" is unknown."));
      return false;
    }
  }

  if (outputFormat == NumericFormat::UNINITIALIZED) {
    outputFormat = NumericFormat::DECIMAL;
  }

  cmExprParserHelper helper;
  if (!helper.ParseString(expression.c_str(), 0)) {
    status.SetError(helper.GetError());
    return false;
  }

  char buffer[1024];
  char const* fmt;
  switch (outputFormat) {
    case NumericFormat::HEXADECIMAL:
      fmt = "0x%" KWIML_INT_PRIx64;
      break;
    case NumericFormat::DECIMAL:
      CM_FALLTHROUGH;
    default:
      fmt = "%" KWIML_INT_PRId64;
      break;
  }
  snprintf(buffer, sizeof(buffer), fmt, helper.GetResult());

  std::string const& w = helper.GetWarning();
  if (!w.empty()) {
    status.GetMakefile().IssueDiagnostic(cmDiagnostics::CMD_AUTHOR, w);
  }

  status.GetMakefile().AddDefinition(outputVariable, buffer);
  return true;
}

bool HandleIncDecCommand(std::vector<std::string> const& args,
                         cmExecutionStatus& status, int amount,
                         long long overflowFrom, long long overflowTo,
                         cm::string_view verbing)
{
  std::string const messageHint = cmStrCat("sub-command ", args[0], " ");
  if (args.size() != 2) {
    status.SetError(cmStrCat(messageHint, "wrong number of arguments"));
    return false;
  }
  auto value = status.GetMakefile().GetDefinition(args[1]);
  if (!value) {
    status.SetError(
      cmStrCat(messageHint, "variable \"", args[1], "\" is not defined"));
    return false;
  }
  if (value->empty()) {
    status.SetError(
      cmStrCat(messageHint, "value \"\" is not a valid integer"));
    return false;
  }
  std::size_t pos = 0;
  long long intValue = 0;
  try {
    intValue = std::stoll(*value, &pos, 10);
  } catch (std::invalid_argument&) {
    // Do nothing, leave pos as is, which will trigger the error
  }
  if (pos != value->length()) {
    status.SetError(
      cmStrCat(messageHint, "value \"", *value, "\" is not a valid integer"));
    return false;
  }
  auto newValue = intValue + amount;
  if (intValue == overflowFrom) {
    status.GetMakefile().IssueDiagnostic(
      cmDiagnosticCategory::CMD_AUTHOR,
      cmStrCat("signed integer overflow while ", verbing, ":\n  ", intValue,
               "\n"));
    // Overflow is undefined behavior in C++, so define it manually
    newValue = overflowTo;
  }
  status.GetMakefile().AddDefinition(args[1], std::to_string(newValue));
  return true;
}
}
