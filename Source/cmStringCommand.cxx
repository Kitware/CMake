/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
// NOLINTNEXTLINE(bugprone-reserved-identifier)
#define _SCL_SECURE_NO_WARNINGS

#include "cmStringCommand.h"

#include <cstdio>
#include <cstdlib>
#include <exception>
#include <limits>
#include <memory>
#include <stdexcept>
#include <utility>

#include <cm/iterator>
#include <cm/optional>
#include <cm/string_view>
#include <cmext/string_view>

#include <cm3p/json/reader.h>
#include <cm3p/json/value.h>
#include <cm3p/json/writer.h>

#include "cmCMakeString.hxx"
#include "cmExecutionStatus.h"
#include "cmList.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmRange.h"
#include "cmStringAlgorithms.h"
#include "cmSubcommandTable.h"

namespace {

bool RegexMatch(std::vector<std::string> const& args,
                cmExecutionStatus& status);
bool RegexMatchAll(std::vector<std::string> const& args,
                   cmExecutionStatus& status);
bool RegexReplace(std::vector<std::string> const& args,
                  cmExecutionStatus& status);
bool RegexQuote(std::vector<std::string> const& args,
                cmExecutionStatus& status);

bool joinImpl(std::vector<std::string> const& args, std::string const& glue,
              size_t varIdx, cmMakefile& makefile);

bool HandleHashCommand(std::vector<std::string> const& args,
                       cmExecutionStatus& status)
{
  if (args.size() != 3) {
    status.SetError(
      cmStrCat(args[0], " requires an output variable and an input string"));
    return false;
  }

  cm::CMakeString data{ args[2] };

  try {
    data.Hash(args[0]);
    status.GetMakefile().AddDefinition(args[1], data);
    return true;
  } catch (std::exception const& e) {
    status.SetError(e.what());
    return false;
  }
}

bool HandleToUpperLowerCommand(std::vector<std::string> const& args,
                               bool toUpper, cmExecutionStatus& status)
{
  if (args.size() < 3) {
    status.SetError("no output variable specified");
    return false;
  }

  std::string const& outvar = args[2];
  cm::CMakeString data{ args[1] };

  if (toUpper) {
    data.ToUpper();
  } else {
    data.ToLower();
  }

  // Store the output in the provided variable.
  status.GetMakefile().AddDefinition(outvar, data);
  return true;
}

bool HandleToUpperCommand(std::vector<std::string> const& args,
                          cmExecutionStatus& status)
{
  return HandleToUpperLowerCommand(args, true, status);
}

bool HandleToLowerCommand(std::vector<std::string> const& args,
                          cmExecutionStatus& status)
{
  return HandleToUpperLowerCommand(args, false, status);
}

bool HandleAsciiCommand(std::vector<std::string> const& args,
                        cmExecutionStatus& status)
{
  if (args.size() < 3) {
    status.SetError("No output variable specified");
    return false;
  }

  try {
    std::string const& outvar = args.back();
    cm::CMakeString data;
    data.FromASCII(cmMakeRange(args).advance(1).retreat(1));
    status.GetMakefile().AddDefinition(outvar, data);
    return true;
  } catch (std::exception const& e) {
    status.SetError(e.what());
    return false;
  }
}

bool HandleHexCommand(std::vector<std::string> const& args,
                      cmExecutionStatus& status)
{
  if (args.size() != 3) {
    status.SetError("Incorrect number of arguments");
    return false;
  }

  auto const& outvar = args[2];
  cm::CMakeString data{ args[1] };

  data.ToHexadecimal();

  status.GetMakefile().AddDefinition(outvar, data);
  return true;
}

bool HandleConfigureCommand(std::vector<std::string> const& args,
                            cmExecutionStatus& status)
{
  if (args.size() < 2) {
    status.SetError("No input string specified.");
    return false;
  }
  if (args.size() < 3) {
    status.SetError("No output variable specified.");
    return false;
  }

  // Parse options.
  bool escapeQuotes = false;
  bool atOnly = false;
  for (unsigned int i = 3; i < args.size(); ++i) {
    if (args[i] == "@ONLY") {
      atOnly = true;
    } else if (args[i] == "ESCAPE_QUOTES") {
      escapeQuotes = true;
    } else {
      status.SetError(cmStrCat("Unrecognized argument \"", args[i], '"'));
      return false;
    }
  }

  // Configure the string.
  std::string output;
  status.GetMakefile().ConfigureString(args[1], output, atOnly, escapeQuotes);

  // Store the output in the provided variable.
  status.GetMakefile().AddDefinition(args[2], output);

  return true;
}

bool HandleRegexCommand(std::vector<std::string> const& args,
                        cmExecutionStatus& status)
{
  if (args.size() < 2) {
    status.SetError("sub-command REGEX requires a mode to be specified.");
    return false;
  }
  std::string const& mode = args[1];
  if (mode == "MATCH") {
    if (args.size() < 5) {
      status.SetError("sub-command REGEX, mode MATCH needs "
                      "at least 5 arguments total to command.");
      return false;
    }
    return RegexMatch(args, status);
  }
  if (mode == "MATCHALL") {
    if (args.size() < 5) {
      status.SetError("sub-command REGEX, mode MATCHALL needs "
                      "at least 5 arguments total to command.");
      return false;
    }
    return RegexMatchAll(args, status);
  }
  if (mode == "REPLACE") {
    if (args.size() < 6) {
      status.SetError("sub-command REGEX, mode REPLACE needs "
                      "at least 6 arguments total to command.");
      return false;
    }
    return RegexReplace(args, status);
  }
  if (mode == "QUOTE") {
    if (args.size() < 4) {
      status.SetError("sub-command REGEX, mode QUOTE needs "
                      "at least 4 arguments total to command.");
      return false;
    }
    return RegexQuote(args, status);
  }

  std::string e = "sub-command REGEX does not recognize mode " + mode;
  status.SetError(e);
  return false;
}

bool RegexMatch(std::vector<std::string> const& args,
                cmExecutionStatus& status)
{
  //"STRING(REGEX MATCH <regular_expression> <output variable>
  // <input> [<input>...])\n";
  try {
    std::string const& regex = args[2];
    std::string const& outvar = args[3];
    cm::CMakeString data{ cmMakeRange(args).advance(4) };

    auto result = data.Match(regex, cm::CMakeString::MatchItems::Once,
                             &status.GetMakefile());
    // Store the result in the provided variable.
    status.GetMakefile().AddDefinition(outvar, result.to_string());
    return true;
  } catch (std::exception const& e) {
    status.SetError(
      cmStrCat("sub-command REGEX, mode MATCH: ", e.what(), '.'));
    return false;
  }
}

bool RegexMatchAll(std::vector<std::string> const& args,
                   cmExecutionStatus& status)
{
  //"STRING(REGEX MATCHALL <regular_expression> <output variable> <input>
  // [<input>...])\n";
  try {
    std::string const& regex = args[2];
    std::string const& outvar = args[3];
    cm::CMakeString data{ cmMakeRange(args).advance(4) };

    auto result = data.Match(regex, cm::CMakeString::MatchItems::All,
                             &status.GetMakefile());
    // Store the result in the provided variable.
    status.GetMakefile().AddDefinition(outvar, result.to_string());
    return true;
  } catch (std::exception const& e) {
    status.SetError(
      cmStrCat("sub-command REGEX, mode MATCHALL: ", e.what(), '.'));
    return false;
  }
}

bool RegexReplace(std::vector<std::string> const& args,
                  cmExecutionStatus& status)
{
  //"STRING(REGEX REPLACE <regular_expression> <replace_expression>
  // <output variable> <input> [<input>...])\n"
  std::string const& regex = args[2];
  std::string const& replace = args[3];
  std::string const& outvar = args[4];

  try {
    cm::CMakeString data{ cmMakeRange(args).advance(5) };

    data.Replace(regex, replace, cm::CMakeString::Regex::Yes,
                 &status.GetMakefile());
    // Store the result in the provided variable.
    status.GetMakefile().AddDefinition(outvar, data);
    return true;
  } catch (std::exception const& e) {
    status.SetError(
      cmStrCat("sub-command REGEX, mode REPLACE: ", e.what(), '.'));
    return false;
  }
}

bool RegexQuote(std::vector<std::string> const& args,
                cmExecutionStatus& status)
{
  //"STRING(REGEX QUOTE <output variable> <input> [<input>...]\n"
  std::string const& outvar = args[2];
  cm::CMakeString data{ cmMakeRange(args).advance(3) };

  try {
    // Escape all regex special characters
    data.Quote();
    // Store the output in the provided variable.
    status.GetMakefile().AddDefinition(outvar, data);
    return true;
  } catch (std::exception const& e) {
    status.SetError(
      cmStrCat("sub-command REGEX, mode QUOTE: ", e.what(), '.'));
    return false;
  }
}

bool HandleFindCommand(std::vector<std::string> const& args,
                       cmExecutionStatus& status)
{
  // check if all required parameters were passed
  if (args.size() < 4 || args.size() > 5) {
    status.SetError("sub-command FIND requires 3 or 4 parameters.");
    return false;
  }

  // check if the reverse flag was set or not
  bool reverseMode = false;
  if (args.size() == 5 && args[4] == "REVERSE") {
    reverseMode = true;
  }

  // if we have 5 arguments the last one must be REVERSE
  if (args.size() == 5 && args[4] != "REVERSE") {
    status.SetError("sub-command FIND: unknown last parameter");
    return false;
  }

  // local parameter names.
  std::string const& sstring = args[1];
  std::string const& schar = args[2];
  std::string const& outvar = args[3];

  // ensure that the user cannot accidentally specify REVERSE as a variable
  if (outvar == "REVERSE") {
    status.SetError("sub-command FIND does not allow one to select REVERSE as "
                    "the output variable.  "
                    "Maybe you missed the actual output variable?");
    return false;
  }

  // try to find the character and return its position
  auto pos = cm::CMakeString{ sstring }.Find(
    schar,
    reverseMode ? cm::CMakeString::FindFrom::End
                : cm::CMakeString::FindFrom::Begin);

  status.GetMakefile().AddDefinition(
    outvar, pos != cm::CMakeString::npos ? std::to_string(pos) : "-1");

  return true;
}

bool HandleCompareCommand(std::vector<std::string> const& args,
                          cmExecutionStatus& status)
{
  if (args.size() < 2) {
    status.SetError("sub-command COMPARE requires a mode to be specified.");
    return false;
  }
  std::string const& mode = args[1];
  if ((mode == "EQUAL") || (mode == "NOTEQUAL") || (mode == "LESS") ||
      (mode == "LESS_EQUAL") || (mode == "GREATER") ||
      (mode == "GREATER_EQUAL")) {
    if (args.size() < 5) {
      std::string e =
        cmStrCat("sub-command COMPARE, mode ", mode,
                 " needs at least 5 arguments total to command.");
      status.SetError(e);
      return false;
    }

    std::string const& left = args[2];
    std::string const& right = args[3];
    std::string const& outvar = args[4];
    bool result;
    cm::CMakeString::CompOperator op = cm::CMakeString::CompOperator::EQUAL;
    if (mode == "LESS") {
      op = cm::CMakeString::CompOperator::LESS;
    } else if (mode == "LESS_EQUAL") {
      op = cm::CMakeString::CompOperator::LESS_EQUAL;
    } else if (mode == "GREATER") {
      op = cm::CMakeString::CompOperator::GREATER;
    } else if (mode == "GREATER_EQUAL") {
      op = cm::CMakeString::CompOperator::GREATER_EQUAL;
    }
    result = cm::CMakeString{ left }.Compare(op, right);
    if (mode == "NOTEQUAL") {
      result = !result;
    }

    status.GetMakefile().AddDefinition(outvar, result ? "1" : "0");
    return true;
  }
  std::string e = "sub-command COMPARE does not recognize mode " + mode;
  status.SetError(e);
  return false;
}

bool HandleReplaceCommand(std::vector<std::string> const& args,
                          cmExecutionStatus& status)
{
  if (args.size() < 5) {
    status.SetError("sub-command REPLACE requires at least four arguments.");
    return false;
  }

  try {
    std::string const& matchExpression = args[1];
    std::string const& replaceExpression = args[2];
    std::string const& variableName = args[3];
    cm::CMakeString data{ cmMakeRange(args).advance(4) };

    data.Replace(matchExpression, replaceExpression);
    status.GetMakefile().AddDefinition(variableName, data);
    return true;
  } catch (std::exception const& e) {
    status.SetError(cmStrCat("sub-command REPLACE: ", e.what(), '.'));
    return false;
  }
}

bool HandleSubstringCommand(std::vector<std::string> const& args,
                            cmExecutionStatus& status)
{
  if (args.size() != 5) {
    status.SetError("sub-command SUBSTRING requires four arguments.");
    return false;
  }

  try {
    std::string const& stringValue = args[1];
    int begin = atoi(args[2].c_str());
    int end = atoi(args[3].c_str());
    std::string const& variableName = args[4];

    cm::CMakeString data{ stringValue };
    status.GetMakefile().AddDefinition(variableName,
                                       data.Substring(begin, end));
  } catch (std::exception const& e) {
    status.SetError(e.what());
    return false;
  }
  return true;
}

bool HandleLengthCommand(std::vector<std::string> const& args,
                         cmExecutionStatus& status)
{
  if (args.size() != 3) {
    status.SetError("sub-command LENGTH requires two arguments.");
    return false;
  }

  std::string const& stringValue = args[1];
  std::string const& variableName = args[2];

  status.GetMakefile().AddDefinition(
    variableName, std::to_string(cm::CMakeString{ stringValue }.Length()));
  return true;
}

bool HandleAppendCommand(std::vector<std::string> const& args,
                         cmExecutionStatus& status)
{
  if (args.size() < 2) {
    status.SetError("sub-command APPEND requires at least one argument.");
    return false;
  }

  // Skip if nothing to append.
  if (args.size() < 3) {
    return true;
  }

  auto const& variableName = args[1];
  cm::CMakeString data{ status.GetMakefile().GetDefinition(variableName) };

  data.Append(cmMakeRange(args).advance(2));
  status.GetMakefile().AddDefinition(variableName, data);

  return true;
}

bool HandlePrependCommand(std::vector<std::string> const& args,
                          cmExecutionStatus& status)
{
  if (args.size() < 2) {
    status.SetError("sub-command PREPEND requires at least one argument.");
    return false;
  }

  // Skip if nothing to prepend.
  if (args.size() < 3) {
    return true;
  }

  std::string const& variable = args[1];
  cm::CMakeString data{ status.GetMakefile().GetDefinition(variable) };

  data.Prepend(cmMakeRange(args).advance(2));
  status.GetMakefile().AddDefinition(variable, data);
  return true;
}

bool HandleConcatCommand(std::vector<std::string> const& args,
                         cmExecutionStatus& status)
{
  if (args.size() < 2) {
    status.SetError("sub-command CONCAT requires at least one argument.");
    return false;
  }

  return joinImpl(args, std::string(), 1, status.GetMakefile());
}

bool HandleJoinCommand(std::vector<std::string> const& args,
                       cmExecutionStatus& status)
{
  if (args.size() < 3) {
    status.SetError("sub-command JOIN requires at least two arguments.");
    return false;
  }

  return joinImpl(args, args[1], 2, status.GetMakefile());
}

bool joinImpl(std::vector<std::string> const& args, std::string const& glue,
              size_t const varIdx, cmMakefile& makefile)
{
  std::string const& variableName = args[varIdx];
  // NOTE Items to concat/join placed right after the variable for
  // both `CONCAT` and `JOIN` sub-commands.
  cm::CMakeString data{ cmMakeRange(args).advance(varIdx + 1), glue };

  makefile.AddDefinition(variableName, data);
  return true;
}

bool HandleMakeCIdentifierCommand(std::vector<std::string> const& args,
                                  cmExecutionStatus& status)
{
  if (args.size() != 3) {
    status.SetError("sub-command MAKE_C_IDENTIFIER requires two arguments.");
    return false;
  }

  std::string const& input = args[1];
  std::string const& variableName = args[2];

  status.GetMakefile().AddDefinition(variableName,
                                     cm::CMakeString{}.MakeCIdentifier(input));
  return true;
}

bool HandleGenexStripCommand(std::vector<std::string> const& args,
                             cmExecutionStatus& status)
{
  if (args.size() != 3) {
    status.SetError("sub-command GENEX_STRIP requires two arguments.");
    return false;
  }

  cm::CMakeString data{ args[1] };
  std::string const& variableName = args[2];

  status.GetMakefile().AddDefinition(
    variableName, data.Strip(cm::CMakeString::StripItems::Genex));
  return true;
}

bool HandleStripCommand(std::vector<std::string> const& args,
                        cmExecutionStatus& status)
{
  if (args.size() != 3) {
    status.SetError("sub-command STRIP requires two arguments.");
    return false;
  }

  cm::CMakeString data{ args[1] };
  std::string const& variableName = args[2];

  status.GetMakefile().AddDefinition(variableName, data.Strip());
  return true;
}

bool HandleRepeatCommand(std::vector<std::string> const& args,
                         cmExecutionStatus& status)
{
  cmMakefile& makefile = status.GetMakefile();

  // `string(REPEAT "<str>" <times> OUTPUT_VARIABLE)`
  enum ArgPos : std::size_t
  {
    SUB_COMMAND,
    VALUE,
    TIMES,
    OUTPUT_VARIABLE,
    TOTAL_ARGS
  };

  if (args.size() != ArgPos::TOTAL_ARGS) {
    makefile.IssueMessage(MessageType::FATAL_ERROR,
                          "sub-command REPEAT requires three arguments.");
    return true;
  }

  unsigned long times;
  if (!cmStrToULong(args[ArgPos::TIMES], &times)) {
    makefile.IssueMessage(MessageType::FATAL_ERROR,
                          "repeat count is not a positive number.");
    return true;
  }

  cm::CMakeString data{ args[ArgPos::VALUE] };
  data.Repeat(times);

  auto const& variableName = args[ArgPos::OUTPUT_VARIABLE];

  makefile.AddDefinition(variableName, data);
  return true;
}

bool HandleRandomCommand(std::vector<std::string> const& args,
                         cmExecutionStatus& status)
{
  if (args.size() < 2 || args.size() == 3 || args.size() == 5) {
    status.SetError("sub-command RANDOM requires at least one argument.");
    return false;
  }

  int length = 5;
  cm::string_view alphabet;
  bool force_seed = false;
  unsigned int seed = 0;

  if (args.size() > 3) {
    size_t i = 1;
    size_t stopAt = args.size() - 2;

    for (; i < stopAt; ++i) {
      if (args[i] == "LENGTH") {
        ++i;
        length = atoi(args[i].c_str());
      } else if (args[i] == "ALPHABET") {
        ++i;
        alphabet = args[i];
      } else if (args[i] == "RANDOM_SEED") {
        ++i;
        seed = static_cast<unsigned int>(atoi(args[i].c_str()));
        force_seed = true;
      }
    }
  }

  try {
    cm::CMakeString data;
    std::string const& variableName = args.back();

    if (force_seed) {
      data.Random(seed, length, alphabet);
    } else {
      data.Random(length, alphabet);
    }
    status.GetMakefile().AddDefinition(variableName, data);
    return true;
  } catch (std::exception const& e) {
    status.SetError(cmStrCat("sub-command RANDOM: ", e.what(), '.'));
    return false;
  }
}

bool HandleTimestampCommand(std::vector<std::string> const& args,
                            cmExecutionStatus& status)
{
  if (args.size() < 2) {
    status.SetError("sub-command TIMESTAMP requires at least one argument.");
    return false;
  }
  if (args.size() > 4) {
    status.SetError("sub-command TIMESTAMP takes at most three arguments.");
    return false;
  }

  unsigned int argsIndex = 1;

  std::string const& outputVariable = args[argsIndex++];

  cm::string_view formatString;
  if (args.size() > argsIndex && args[argsIndex] != "UTC") {
    formatString = args[argsIndex++];
  }

  cm::CMakeString::UTC utcFlag = cm::CMakeString::UTC::No;
  if (args.size() > argsIndex) {
    if (args[argsIndex] == "UTC") {
      utcFlag = cm::CMakeString::UTC::Yes;
    } else {
      std::string e =
        cmStrCat(" TIMESTAMP sub-command does not recognize option ",
                 args[argsIndex], '.');
      status.SetError(e);
      return false;
    }
  }

  cm::CMakeString data;

  status.GetMakefile().AddDefinition(outputVariable,
                                     data.Timestamp(formatString, utcFlag));

  return true;
}

bool HandleUuidCommand(std::vector<std::string> const& args,
                       cmExecutionStatus& status)
{
#if !defined(CMAKE_BOOTSTRAP)
  unsigned int argsIndex = 1;

  if (args.size() < 2) {
    status.SetError("UUID sub-command requires an output variable.");
    return false;
  }

  std::string const& outputVariable = args[argsIndex++];

  cm::string_view uuidNamespaceString;
  cm::string_view uuidName;
  cm::CMakeString::UUIDType uuidType = cm::CMakeString::UUIDType::MD5;
  cm::CMakeString::Case uuidCase = cm::CMakeString::Case::Lower;

  while (args.size() > argsIndex) {
    if (args[argsIndex] == "NAMESPACE") {
      ++argsIndex;
      if (argsIndex >= args.size()) {
        status.SetError("UUID sub-command, NAMESPACE requires a value.");
        return false;
      }
      uuidNamespaceString = args[argsIndex++];
    } else if (args[argsIndex] == "NAME") {
      ++argsIndex;
      if (argsIndex >= args.size()) {
        status.SetError("UUID sub-command, NAME requires a value.");
        return false;
      }
      uuidName = args[argsIndex++];
    } else if (args[argsIndex] == "TYPE") {
      ++argsIndex;
      if (argsIndex >= args.size()) {
        status.SetError("UUID sub-command, TYPE requires a value.");
        return false;
      }
      if (args[argsIndex] == "MD5") {
        uuidType = cm::CMakeString::UUIDType::MD5;
      } else if (args[argsIndex] == "SHA1") {
        uuidType = cm::CMakeString::UUIDType::SHA1;
      } else {
        status.SetError(
          cmStrCat("UUID sub-command, unknown TYPE '", args[argsIndex], "'."));
        return false;
      }
      argsIndex++;
    } else if (args[argsIndex] == "UPPER") {
      ++argsIndex;
      uuidCase = cm::CMakeString::Case::Upper;
    } else {
      std::string e = cmStrCat("UUID sub-command does not recognize option ",
                               args[argsIndex], '.');
      status.SetError(e);
      return false;
    }
  }

  try {
    cm::CMakeString data;

    data.UUID(uuidNamespaceString, uuidName, uuidType, uuidCase);
    status.GetMakefile().AddDefinition(outputVariable, data);
    return true;
  } catch (std::exception const& e) {
    status.SetError(cmStrCat("UUID sub-command, ", e.what(), '.'));
    return false;
  }
#else
  status.SetError("UUID sub-command not available during bootstrap.");
  return false;
#endif
}

#if !defined(CMAKE_BOOTSTRAP)

// Helpers for string(JSON ...)
struct Args : cmRange<typename std::vector<std::string>::const_iterator>
{
  using cmRange<typename std::vector<std::string>::const_iterator>::cmRange;

  auto PopFront(cm::string_view error) -> std::string const&;
  auto PopBack(cm::string_view error) -> std::string const&;
};

class json_error : public std::runtime_error
{
public:
  json_error(std::string const& message,
             cm::optional<Args> errorPath = cm::nullopt)
    : std::runtime_error(message)
    , ErrorPath{
      std::move(errorPath) // NOLINT(performance-move-const-arg)
    }
  {
  }
  cm::optional<Args> ErrorPath;
};

std::string const& Args::PopFront(cm::string_view error)
{
  if (this->empty()) {
    throw json_error(std::string(error));
  }
  std::string const& res = *this->begin();
  this->advance(1);
  return res;
}

std::string const& Args::PopBack(cm::string_view error)
{
  if (this->empty()) {
    throw json_error(std::string(error));
  }
  std::string const& res = *(this->end() - 1);
  this->retreat(1);
  return res;
}

cm::string_view JsonTypeToString(Json::ValueType type)
{
  switch (type) {
    case Json::ValueType::nullValue:
      return "NULL"_s;
    case Json::ValueType::intValue:
    case Json::ValueType::uintValue:
    case Json::ValueType::realValue:
      return "NUMBER"_s;
    case Json::ValueType::stringValue:
      return "STRING"_s;
    case Json::ValueType::booleanValue:
      return "BOOLEAN"_s;
    case Json::ValueType::arrayValue:
      return "ARRAY"_s;
    case Json::ValueType::objectValue:
      return "OBJECT"_s;
  }
  throw json_error("invalid JSON type found");
}

int ParseIndex(
  std::string const& str, cm::optional<Args> const& progress = cm::nullopt,
  Json::ArrayIndex max = std::numeric_limits<Json::ArrayIndex>::max())
{
  unsigned long lindex;
  if (!cmStrToULong(str, &lindex)) {
    throw json_error(cmStrCat("expected an array index, got: '"_s, str, "'"_s),
                     progress);
  }
  Json::ArrayIndex index = static_cast<Json::ArrayIndex>(lindex);
  if (index >= max) {
    cmAlphaNum sizeStr{ max };
    throw json_error(cmStrCat("expected an index less than "_s, sizeStr.View(),
                              " got '"_s, str, "'"_s),
                     progress);
  }
  return index;
}

Json::Value& ResolvePath(Json::Value& json, Args path)
{
  Json::Value* search = &json;

  for (auto curr = path.begin(); curr != path.end(); ++curr) {
    std::string const& field = *curr;
    Args progress{ path.begin(), curr + 1 };

    if (search->isArray()) {
      auto index = ParseIndex(field, progress, search->size());
      search = &(*search)[index];

    } else if (search->isObject()) {
      if (!search->isMember(field)) {
        auto const progressStr = cmJoin(progress, " "_s);
        throw json_error(cmStrCat("member '"_s, progressStr, "' not found"_s),
                         progress);
      }
      search = &(*search)[field];
    } else {
      auto const progressStr = cmJoin(progress, " "_s);
      throw json_error(
        cmStrCat("invalid path '"_s, progressStr,
                 "', need element of OBJECT or ARRAY type to lookup '"_s,
                 field, "' got "_s, JsonTypeToString(search->type())),
        progress);
    }
  }
  return *search;
}

Json::Value ReadJson(std::string const& jsonstr)
{
  Json::CharReaderBuilder builder;
  builder["collectComments"] = false;
  auto jsonReader = std::unique_ptr<Json::CharReader>(builder.newCharReader());
  Json::Value json;
  std::string error;
  if (!jsonReader->parse(jsonstr.data(), jsonstr.data() + jsonstr.size(),
                         &json, &error)) {
    throw json_error(
      cmStrCat("failed parsing json string:\n"_s, jsonstr, '\n', error));
  }
  return json;
}
std::string WriteJson(Json::Value const& value)
{
  Json::StreamWriterBuilder writer;
  writer["indentation"] = "  ";
  writer["commentStyle"] = "None";
  return Json::writeString(writer, value);
}

#endif

bool HandleJSONCommand(std::vector<std::string> const& arguments,
                       cmExecutionStatus& status)
{
#if !defined(CMAKE_BOOTSTRAP)

  auto& makefile = status.GetMakefile();
  Args args{ arguments.begin() + 1, arguments.end() };

  std::string const* errorVariable = nullptr;
  std::string const* outputVariable = nullptr;
  bool success = true;

  try {
    outputVariable = &args.PopFront("missing out-var argument"_s);

    if (!args.empty() && *args.begin() == "ERROR_VARIABLE"_s) {
      args.PopFront("");
      errorVariable = &args.PopFront("missing error-var argument"_s);
      makefile.AddDefinition(*errorVariable, "NOTFOUND"_s);
    }

    auto const& mode = args.PopFront("missing mode argument"_s);
    if (mode != "GET"_s && mode != "GET_RAW"_s && mode != "TYPE"_s &&
        mode != "MEMBER"_s && mode != "LENGTH"_s && mode != "REMOVE"_s &&
        mode != "SET"_s && mode != "EQUAL"_s && mode != "STRING_ENCODE"_s) {
      throw json_error(cmStrCat(
        "got an invalid mode '"_s, mode,
        "', expected one of GET, GET_RAW, TYPE, MEMBER, LENGTH, REMOVE, SET, "
        " EQUAL, STRING_ENCODE"_s));
    }

    auto const& jsonstr = args.PopFront("missing json string argument"_s);

    if (mode == "STRING_ENCODE"_s) {
      Json::Value json(jsonstr);
      makefile.AddDefinition(*outputVariable, WriteJson(json));
    } else {
      Json::Value json = ReadJson(jsonstr);

      if (mode == "GET"_s) {
        auto const& value = ResolvePath(json, args);
        if (value.isObject() || value.isArray()) {
          makefile.AddDefinition(*outputVariable, WriteJson(value));
        } else if (value.isBool()) {
          makefile.AddDefinitionBool(*outputVariable, value.asBool());
        } else {
          makefile.AddDefinition(*outputVariable, value.asString());
        }

      } else if (mode == "GET_RAW"_s) {
        auto const& value = ResolvePath(json, args);
        makefile.AddDefinition(*outputVariable, WriteJson(value));

      } else if (mode == "TYPE"_s) {
        auto const& value = ResolvePath(json, args);
        makefile.AddDefinition(*outputVariable,
                               JsonTypeToString(value.type()));

      } else if (mode == "MEMBER"_s) {
        auto const& indexStr = args.PopBack("missing member index"_s);
        auto const& value = ResolvePath(json, args);
        if (!value.isObject()) {
          throw json_error(
            cmStrCat("MEMBER needs to be called with an element of "
                     "type OBJECT, got "_s,
                     JsonTypeToString(value.type())),
            args);
        }
        auto const index = ParseIndex(
          indexStr, Args{ args.begin(), args.end() + 1 }, value.size());
        auto const memIt = std::next(value.begin(), index);
        makefile.AddDefinition(*outputVariable, memIt.name());

      } else if (mode == "LENGTH"_s) {
        auto const& value = ResolvePath(json, args);
        if (!value.isArray() && !value.isObject()) {
          throw json_error(cmStrCat("LENGTH needs to be called with an "
                                    "element of type ARRAY or OBJECT, got "_s,
                                    JsonTypeToString(value.type())),
                           args);
        }

        cmAlphaNum sizeStr{ value.size() };
        makefile.AddDefinition(*outputVariable, sizeStr.View());

      } else if (mode == "REMOVE"_s) {
        auto const& toRemove =
          args.PopBack("missing member or index to remove"_s);
        auto& value = ResolvePath(json, args);

        if (value.isArray()) {
          auto const index = ParseIndex(
            toRemove, Args{ args.begin(), args.end() + 1 }, value.size());
          Json::Value removed;
          value.removeIndex(index, &removed);

        } else if (value.isObject()) {
          Json::Value removed;
          value.removeMember(toRemove, &removed);

        } else {
          throw json_error(cmStrCat("REMOVE needs to be called with an "
                                    "element of type ARRAY or OBJECT, got "_s,
                                    JsonTypeToString(value.type())),
                           args);
        }
        makefile.AddDefinition(*outputVariable, WriteJson(json));

      } else if (mode == "SET"_s) {
        auto const& newValueStr = args.PopBack("missing new value remove"_s);
        auto const& toAdd = args.PopBack("missing member name to add"_s);
        auto& value = ResolvePath(json, args);

        Json::Value newValue = ReadJson(newValueStr);
        if (value.isObject()) {
          value[toAdd] = newValue;
        } else if (value.isArray()) {
          auto const index =
            ParseIndex(toAdd, Args{ args.begin(), args.end() + 1 });
          if (value.isValidIndex(index)) {
            value[static_cast<int>(index)] = newValue;
          } else {
            value.append(newValue);
          }
        } else {
          throw json_error(cmStrCat("SET needs to be called with an "
                                    "element of type OBJECT or ARRAY, got "_s,
                                    JsonTypeToString(value.type())));
        }

        makefile.AddDefinition(*outputVariable, WriteJson(json));

      } else if (mode == "EQUAL"_s) {
        auto const& jsonstr2 =
          args.PopFront("missing second json string argument"_s);
        Json::Value json2 = ReadJson(jsonstr2);
        makefile.AddDefinitionBool(*outputVariable, json == json2);
      }
    }

  } catch (json_error const& e) {
    if (outputVariable && e.ErrorPath) {
      auto const errorPath = cmJoin(*e.ErrorPath, "-");
      makefile.AddDefinition(*outputVariable,
                             cmStrCat(errorPath, "-NOTFOUND"_s));
    } else if (outputVariable) {
      makefile.AddDefinition(*outputVariable, "NOTFOUND"_s);
    }

    if (errorVariable) {
      makefile.AddDefinition(*errorVariable, e.what());
    } else {
      status.SetError(cmStrCat("sub-command JSON "_s, e.what(), "."_s));
      success = false;
    }
  }
  return success;
#else
  status.SetError(cmStrCat(arguments[0], " not available during bootstrap"_s));
  return false;
#endif
}

} // namespace

bool cmStringCommand(std::vector<std::string> const& args,
                     cmExecutionStatus& status)
{
  if (args.empty()) {
    status.SetError("must be called with at least one argument.");
    return false;
  }

  static cmSubcommandTable const subcommand{
    { "REGEX"_s, HandleRegexCommand },
    { "REPLACE"_s, HandleReplaceCommand },
    { "MD5"_s, HandleHashCommand },
    { "SHA1"_s, HandleHashCommand },
    { "SHA224"_s, HandleHashCommand },
    { "SHA256"_s, HandleHashCommand },
    { "SHA384"_s, HandleHashCommand },
    { "SHA512"_s, HandleHashCommand },
    { "SHA3_224"_s, HandleHashCommand },
    { "SHA3_256"_s, HandleHashCommand },
    { "SHA3_384"_s, HandleHashCommand },
    { "SHA3_512"_s, HandleHashCommand },
    { "TOLOWER"_s, HandleToLowerCommand },
    { "TOUPPER"_s, HandleToUpperCommand },
    { "COMPARE"_s, HandleCompareCommand },
    { "ASCII"_s, HandleAsciiCommand },
    { "HEX"_s, HandleHexCommand },
    { "CONFIGURE"_s, HandleConfigureCommand },
    { "LENGTH"_s, HandleLengthCommand },
    { "APPEND"_s, HandleAppendCommand },
    { "PREPEND"_s, HandlePrependCommand },
    { "CONCAT"_s, HandleConcatCommand },
    { "JOIN"_s, HandleJoinCommand },
    { "SUBSTRING"_s, HandleSubstringCommand },
    { "STRIP"_s, HandleStripCommand },
    { "REPEAT"_s, HandleRepeatCommand },
    { "RANDOM"_s, HandleRandomCommand },
    { "FIND"_s, HandleFindCommand },
    { "TIMESTAMP"_s, HandleTimestampCommand },
    { "MAKE_C_IDENTIFIER"_s, HandleMakeCIdentifierCommand },
    { "GENEX_STRIP"_s, HandleGenexStripCommand },
    { "UUID"_s, HandleUuidCommand },
    { "JSON"_s, HandleJSONCommand },
  };

  return subcommand(args[0], args, status);
}
