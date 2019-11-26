/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#define _SCL_SECURE_NO_WARNINGS

#include "cmStringCommand.h"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <memory>

#include <cm/iterator>

#include "cmsys/RegularExpression.hxx"

#include "cm_static_string_view.hxx"

#include "cmCryptoHash.h"
#include "cmExecutionStatus.h"
#include "cmGeneratorExpression.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmRange.h"
#include "cmStringAlgorithms.h"
#include "cmStringReplaceHelper.h"
#include "cmSubcommandTable.h"
#include "cmSystemTools.h"
#include "cmTimestamp.h"
#include "cmUuid.h"

namespace {

bool RegexMatch(std::vector<std::string> const& args,
                cmExecutionStatus& status);
bool RegexMatchAll(std::vector<std::string> const& args,
                   cmExecutionStatus& status);
bool RegexReplace(std::vector<std::string> const& args,
                  cmExecutionStatus& status);

bool joinImpl(std::vector<std::string> const& args, std::string const& glue,
              size_t varIdx, cmMakefile& makefile);

bool HandleHashCommand(std::vector<std::string> const& args,
                       cmExecutionStatus& status)
{
#if !defined(CMAKE_BOOTSTRAP)
  if (args.size() != 3) {
    status.SetError(
      cmStrCat(args[0], " requires an output variable and an input string"));
    return false;
  }

  std::unique_ptr<cmCryptoHash> hash(cmCryptoHash::New(args[0]));
  if (hash) {
    std::string out = hash->HashString(args[2]);
    status.GetMakefile().AddDefinition(args[1], out);
    return true;
  }
  return false;
#else
  status.SetError(cmStrCat(args[0], " not available during bootstrap"));
  return false;
#endif
}

bool HandleToUpperLowerCommand(std::vector<std::string> const& args,
                               bool toUpper, cmExecutionStatus& status)
{
  if (args.size() < 3) {
    status.SetError("no output variable specified");
    return false;
  }

  std::string const& outvar = args[2];
  std::string output;

  if (toUpper) {
    output = cmSystemTools::UpperCase(args[1]);
  } else {
    output = cmSystemTools::LowerCase(args[1]);
  }

  // Store the output in the provided variable.
  status.GetMakefile().AddDefinition(outvar, output);
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
  std::string::size_type cc;
  std::string const& outvar = args.back();
  std::string output;
  for (cc = 1; cc < args.size() - 1; cc++) {
    int ch = atoi(args[cc].c_str());
    if (ch > 0 && ch < 256) {
      output += static_cast<char>(ch);
    } else {
      std::string error =
        cmStrCat("Character with code ", args[cc], " does not exist.");
      status.SetError(error);
      return false;
    }
  }
  // Store the output in the provided variable.
  status.GetMakefile().AddDefinition(outvar, output);
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
      status.SetError(cmStrCat("Unrecognized argument \"", args[i], "\""));
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

  std::string e = "sub-command REGEX does not recognize mode " + mode;
  status.SetError(e);
  return false;
}

bool RegexMatch(std::vector<std::string> const& args,
                cmExecutionStatus& status)
{
  //"STRING(REGEX MATCH <regular_expression> <output variable>
  // <input> [<input>...])\n";
  std::string const& regex = args[2];
  std::string const& outvar = args[3];

  status.GetMakefile().ClearMatches();
  // Compile the regular expression.
  cmsys::RegularExpression re;
  if (!re.compile(regex.c_str())) {
    std::string e =
      "sub-command REGEX, mode MATCH failed to compile regex \"" + regex +
      "\".";
    status.SetError(e);
    return false;
  }

  // Concatenate all the last arguments together.
  std::string input = cmJoin(cmMakeRange(args).advance(4), std::string());

  // Scan through the input for all matches.
  std::string output;
  if (re.find(input)) {
    status.GetMakefile().StoreMatches(re);
    std::string::size_type l = re.start();
    std::string::size_type r = re.end();
    if (r - l == 0) {
      std::string e = "sub-command REGEX, mode MATCH regex \"" + regex +
        "\" matched an empty string.";
      status.SetError(e);
      return false;
    }
    output = input.substr(l, r - l);
  }

  // Store the output in the provided variable.
  status.GetMakefile().AddDefinition(outvar, output);
  return true;
}

bool RegexMatchAll(std::vector<std::string> const& args,
                   cmExecutionStatus& status)
{
  //"STRING(REGEX MATCHALL <regular_expression> <output variable> <input>
  // [<input>...])\n";
  std::string const& regex = args[2];
  std::string const& outvar = args[3];

  status.GetMakefile().ClearMatches();
  // Compile the regular expression.
  cmsys::RegularExpression re;
  if (!re.compile(regex.c_str())) {
    std::string e =
      "sub-command REGEX, mode MATCHALL failed to compile regex \"" + regex +
      "\".";
    status.SetError(e);
    return false;
  }

  // Concatenate all the last arguments together.
  std::string input = cmJoin(cmMakeRange(args).advance(4), std::string());

  // Scan through the input for all matches.
  std::string output;
  const char* p = input.c_str();
  while (re.find(p)) {
    status.GetMakefile().ClearMatches();
    status.GetMakefile().StoreMatches(re);
    std::string::size_type l = re.start();
    std::string::size_type r = re.end();
    if (r - l == 0) {
      std::string e = "sub-command REGEX, mode MATCHALL regex \"" + regex +
        "\" matched an empty string.";
      status.SetError(e);
      return false;
    }
    if (!output.empty()) {
      output += ";";
    }
    output += std::string(p + l, r - l);
    p += r;
  }

  // Store the output in the provided variable.
  status.GetMakefile().AddDefinition(outvar, output);
  return true;
}

bool RegexReplace(std::vector<std::string> const& args,
                  cmExecutionStatus& status)
{
  //"STRING(REGEX REPLACE <regular_expression> <replace_expression>
  // <output variable> <input> [<input>...])\n"
  std::string const& regex = args[2];
  std::string const& replace = args[3];
  std::string const& outvar = args[4];
  cmStringReplaceHelper replaceHelper(regex, replace, &status.GetMakefile());

  if (!replaceHelper.IsReplaceExpressionValid()) {
    status.SetError(
      "sub-command REGEX, mode REPLACE: " + replaceHelper.GetError() + ".");
    return false;
  }

  status.GetMakefile().ClearMatches();

  if (!replaceHelper.IsRegularExpressionValid()) {
    std::string e =
      "sub-command REGEX, mode REPLACE failed to compile regex \"" + regex +
      "\".";
    status.SetError(e);
    return false;
  }

  // Concatenate all the last arguments together.
  const std::string input =
    cmJoin(cmMakeRange(args).advance(5), std::string());
  std::string output;

  if (!replaceHelper.Replace(input, output)) {
    status.SetError(
      "sub-command REGEX, mode REPLACE: " + replaceHelper.GetError() + ".");
    return false;
  }

  // Store the output in the provided variable.
  status.GetMakefile().AddDefinition(outvar, output);
  return true;
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
  const std::string& sstring = args[1];
  const std::string& schar = args[2];
  const std::string& outvar = args[3];

  // ensure that the user cannot accidentally specify REVERSE as a variable
  if (outvar == "REVERSE") {
    status.SetError("sub-command FIND does not allow one to select REVERSE as "
                    "the output variable.  "
                    "Maybe you missed the actual output variable?");
    return false;
  }

  // try to find the character and return its position
  size_t pos;
  if (!reverseMode) {
    pos = sstring.find(schar);
  } else {
    pos = sstring.rfind(schar);
  }
  if (std::string::npos != pos) {
    status.GetMakefile().AddDefinition(outvar, std::to_string(pos));
    return true;
  }

  // the character was not found, but this is not really an error
  status.GetMakefile().AddDefinition(outvar, "-1");
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

    const std::string& left = args[2];
    const std::string& right = args[3];
    const std::string& outvar = args[4];
    bool result;
    if (mode == "LESS") {
      result = (left < right);
    } else if (mode == "LESS_EQUAL") {
      result = (left <= right);
    } else if (mode == "GREATER") {
      result = (left > right);
    } else if (mode == "GREATER_EQUAL") {
      result = (left >= right);
    } else if (mode == "EQUAL") {
      result = (left == right);
    } else // if(mode == "NOTEQUAL")
    {
      result = !(left == right);
    }
    if (result) {
      status.GetMakefile().AddDefinition(outvar, "1");
    } else {
      status.GetMakefile().AddDefinition(outvar, "0");
    }
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

  const std::string& matchExpression = args[1];
  const std::string& replaceExpression = args[2];
  const std::string& variableName = args[3];

  std::string input = cmJoin(cmMakeRange(args).advance(4), std::string());

  cmsys::SystemTools::ReplaceString(input, matchExpression.c_str(),
                                    replaceExpression.c_str());

  status.GetMakefile().AddDefinition(variableName, input);
  return true;
}

bool HandleSubstringCommand(std::vector<std::string> const& args,
                            cmExecutionStatus& status)
{
  if (args.size() != 5) {
    status.SetError("sub-command SUBSTRING requires four arguments.");
    return false;
  }

  const std::string& stringValue = args[1];
  int begin = atoi(args[2].c_str());
  int end = atoi(args[3].c_str());
  const std::string& variableName = args[4];

  size_t stringLength = stringValue.size();
  int intStringLength = static_cast<int>(stringLength);
  if (begin < 0 || begin > intStringLength) {
    status.SetError(
      cmStrCat("begin index: ", begin, " is out of range 0 - ", stringLength));
    return false;
  }
  if (end < -1) {
    status.SetError(cmStrCat("end index: ", end, " should be -1 or greater"));
    return false;
  }

  status.GetMakefile().AddDefinition(variableName,
                                     stringValue.substr(begin, end));
  return true;
}

bool HandleLengthCommand(std::vector<std::string> const& args,
                         cmExecutionStatus& status)
{
  if (args.size() != 3) {
    status.SetError("sub-command LENGTH requires two arguments.");
    return false;
  }

  const std::string& stringValue = args[1];
  const std::string& variableName = args[2];

  size_t length = stringValue.size();
  char buffer[1024];
  sprintf(buffer, "%d", static_cast<int>(length));

  status.GetMakefile().AddDefinition(variableName, buffer);
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

  const std::string& variable = args[1];

  std::string value;
  const char* oldValue = status.GetMakefile().GetDefinition(variable);
  if (oldValue) {
    value = oldValue;
  }
  value += cmJoin(cmMakeRange(args).advance(2), std::string());
  status.GetMakefile().AddDefinition(variable, value);
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

  const std::string& variable = args[1];

  std::string value = cmJoin(cmMakeRange(args).advance(2), std::string());
  const char* oldValue = status.GetMakefile().GetDefinition(variable);
  if (oldValue) {
    value += oldValue;
  }
  status.GetMakefile().AddDefinition(variable, value);
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
              const size_t varIdx, cmMakefile& makefile)
{
  std::string const& variableName = args[varIdx];
  // NOTE Items to concat/join placed right after the variable for
  // both `CONCAT` and `JOIN` sub-commands.
  std::string value = cmJoin(cmMakeRange(args).advance(varIdx + 1), glue);

  makefile.AddDefinition(variableName, value);
  return true;
}

bool HandleMakeCIdentifierCommand(std::vector<std::string> const& args,
                                  cmExecutionStatus& status)
{
  if (args.size() != 3) {
    status.SetError("sub-command MAKE_C_IDENTIFIER requires two arguments.");
    return false;
  }

  const std::string& input = args[1];
  const std::string& variableName = args[2];

  status.GetMakefile().AddDefinition(variableName,
                                     cmSystemTools::MakeCidentifier(input));
  return true;
}

bool HandleGenexStripCommand(std::vector<std::string> const& args,
                             cmExecutionStatus& status)
{
  if (args.size() != 3) {
    status.SetError("sub-command GENEX_STRIP requires two arguments.");
    return false;
  }

  const std::string& input = args[1];

  std::string result = cmGeneratorExpression::Preprocess(
    input, cmGeneratorExpression::StripAllGeneratorExpressions);

  const std::string& variableName = args[2];

  status.GetMakefile().AddDefinition(variableName, result);
  return true;
}

bool HandleStripCommand(std::vector<std::string> const& args,
                        cmExecutionStatus& status)
{
  if (args.size() != 3) {
    status.SetError("sub-command STRIP requires two arguments.");
    return false;
  }

  const std::string& stringValue = args[1];
  const std::string& variableName = args[2];
  size_t inStringLength = stringValue.size();
  size_t startPos = inStringLength + 1;
  size_t endPos = 0;
  const char* ptr = stringValue.c_str();
  size_t cc;
  for (cc = 0; cc < inStringLength; ++cc) {
    if (!isspace(*ptr)) {
      if (startPos > inStringLength) {
        startPos = cc;
      }
      endPos = cc;
    }
    ++ptr;
  }

  size_t outLength = 0;

  // if the input string didn't contain any non-space characters, return
  // an empty string
  if (startPos > inStringLength) {
    outLength = 0;
    startPos = 0;
  } else {
    outLength = endPos - startPos + 1;
  }

  status.GetMakefile().AddDefinition(variableName,
                                     stringValue.substr(startPos, outLength));
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

  const auto& stringValue = args[ArgPos::VALUE];
  const auto& variableName = args[ArgPos::OUTPUT_VARIABLE];
  const auto inStringLength = stringValue.size();

  std::string result;
  switch (inStringLength) {
    case 0u:
      // Nothing to do for zero length input strings
      break;
    case 1u:
      // NOTE If the string to repeat consists of the only character,
      // use the appropriate constructor.
      result = std::string(times, stringValue[0]);
      break;
    default:
      result = std::string(inStringLength * times, char{});
      for (auto i = 0u; i < times; ++i) {
        std::copy(cm::cbegin(stringValue), cm::cend(stringValue),
                  &result[i * inStringLength]);
      }
      break;
  }

  makefile.AddDefinition(variableName, result);
  return true;
}

bool HandleRandomCommand(std::vector<std::string> const& args,
                         cmExecutionStatus& status)
{
  if (args.size() < 2 || args.size() == 3 || args.size() == 5) {
    status.SetError("sub-command RANDOM requires at least one argument.");
    return false;
  }

  static bool seeded = false;
  bool force_seed = false;
  unsigned int seed = 0;
  int length = 5;
  const char cmStringCommandDefaultAlphabet[] = "qwertyuiopasdfghjklzxcvbnm"
                                                "QWERTYUIOPASDFGHJKLZXCVBNM"
                                                "0123456789";
  std::string alphabet;

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
  if (alphabet.empty()) {
    alphabet = cmStringCommandDefaultAlphabet;
  }

  double sizeofAlphabet = static_cast<double>(alphabet.size());
  if (sizeofAlphabet < 1) {
    status.SetError("sub-command RANDOM invoked with bad alphabet.");
    return false;
  }
  if (length < 1) {
    status.SetError("sub-command RANDOM invoked with bad length.");
    return false;
  }
  const std::string& variableName = args.back();

  std::vector<char> result;

  if (!seeded || force_seed) {
    seeded = true;
    srand(force_seed ? seed : cmSystemTools::RandomSeed());
  }

  const char* alphaPtr = alphabet.c_str();
  for (int cc = 0; cc < length; cc++) {
    int idx = static_cast<int>(sizeofAlphabet * rand() / (RAND_MAX + 1.0));
    result.push_back(*(alphaPtr + idx));
  }
  result.push_back(0);

  status.GetMakefile().AddDefinition(variableName, result.data());
  return true;
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

  const std::string& outputVariable = args[argsIndex++];

  std::string formatString;
  if (args.size() > argsIndex && args[argsIndex] != "UTC") {
    formatString = args[argsIndex++];
  }

  bool utcFlag = false;
  if (args.size() > argsIndex) {
    if (args[argsIndex] == "UTC") {
      utcFlag = true;
    } else {
      std::string e = " TIMESTAMP sub-command does not recognize option " +
        args[argsIndex] + ".";
      status.SetError(e);
      return false;
    }
  }

  cmTimestamp timestamp;
  std::string result = timestamp.CurrentTime(formatString, utcFlag);
  status.GetMakefile().AddDefinition(outputVariable, result);

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

  const std::string& outputVariable = args[argsIndex++];

  std::string uuidNamespaceString;
  std::string uuidName;
  std::string uuidType;
  bool uuidUpperCase = false;

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
      uuidType = args[argsIndex++];
    } else if (args[argsIndex] == "UPPER") {
      ++argsIndex;
      uuidUpperCase = true;
    } else {
      std::string e =
        "UUID sub-command does not recognize option " + args[argsIndex] + ".";
      status.SetError(e);
      return false;
    }
  }

  std::string uuid;
  cmUuid uuidGenerator;

  std::vector<unsigned char> uuidNamespace;
  if (!uuidGenerator.StringToBinary(uuidNamespaceString, uuidNamespace)) {
    status.SetError("UUID sub-command, malformed NAMESPACE UUID.");
    return false;
  }

  if (uuidType == "MD5") {
    uuid = uuidGenerator.FromMd5(uuidNamespace, uuidName);
  } else if (uuidType == "SHA1") {
    uuid = uuidGenerator.FromSha1(uuidNamespace, uuidName);
  } else {
    std::string e = "UUID sub-command, unknown TYPE '" + uuidType + "'.";
    status.SetError(e);
    return false;
  }

  if (uuid.empty()) {
    status.SetError("UUID sub-command, generation failed.");
    return false;
  }

  if (uuidUpperCase) {
    uuid = cmSystemTools::UpperCase(uuid);
  }

  status.GetMakefile().AddDefinition(outputVariable, uuid);
  return true;
#else
  status.SetError(cmStrCat(args[0], " not available during bootstrap"));
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
  };

  return subcommand(args[0], args, status);
}
