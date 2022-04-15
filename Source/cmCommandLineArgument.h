/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"

template <typename FunctionSignature>
struct cmCommandLineArgument
{
  enum class Values
  {
    Zero,
    One,
    Two,
    ZeroOrOne,
    OneOrMore
  };

  enum class RequiresSeparator
  {
    Yes,
    No
  };

  enum class ParseMode
  {
    Valid,
    Invalid,
    SyntaxError,
    ValueError
  };

  std::string InvalidSyntaxMessage;
  std::string InvalidValueMessage;
  std::string Name;
  Values Type;
  RequiresSeparator SeparatorNeeded;
  std::function<FunctionSignature> StoreCall;

  template <typename FunctionType>
  cmCommandLineArgument(std::string n, Values t, FunctionType&& func)
    : InvalidSyntaxMessage(cmStrCat(" is invalid syntax for ", n))
    , InvalidValueMessage(cmStrCat("Invalid value used with ", n))
    , Name(std::move(n))
    , Type(t)
    , SeparatorNeeded(RequiresSeparator::Yes)
    , StoreCall(std::forward<FunctionType>(func))
  {
  }

  template <typename FunctionType>
  cmCommandLineArgument(std::string n, Values t, RequiresSeparator s,
                        FunctionType&& func)
    : InvalidSyntaxMessage(cmStrCat(" is invalid syntax for ", n))
    , InvalidValueMessage(cmStrCat("Invalid value used with ", n))
    , Name(std::move(n))
    , Type(t)
    , SeparatorNeeded(s)
    , StoreCall(std::forward<FunctionType>(func))
  {
  }

  template <typename FunctionType>
  cmCommandLineArgument(std::string n, std::string failedMsg, Values t,
                        FunctionType&& func)
    : InvalidSyntaxMessage(cmStrCat(" is invalid syntax for ", n))
    , InvalidValueMessage(std::move(failedMsg))
    , Name(std::move(n))
    , Type(t)
    , SeparatorNeeded(RequiresSeparator::Yes)
    , StoreCall(std::forward<FunctionType>(func))
  {
  }

  template <typename FunctionType>
  cmCommandLineArgument(std::string n, std::string failedMsg, Values t,
                        RequiresSeparator s, FunctionType&& func)
    : InvalidSyntaxMessage(cmStrCat(" is invalid syntax for ", n))
    , InvalidValueMessage(std::move(failedMsg))
    , Name(std::move(n))
    , Type(t)
    , SeparatorNeeded(s)
    , StoreCall(std::forward<FunctionType>(func))
  {
  }

  bool matches(std::string const& input) const
  {
    bool matched = false;
    if (this->Type == Values::Zero) {
      matched = (input == this->Name);
    } else if (this->SeparatorNeeded == RequiresSeparator::No) {
      matched = cmHasPrefix(input, this->Name);
    } else if (cmHasPrefix(input, this->Name)) {
      if (input.size() == this->Name.size()) {
        matched = true;
      } else {
        matched =
          (input[this->Name.size()] == '=' || input[this->Name.size()] == ' ');
      }
    }
    return matched;
  }

  template <typename T, typename... CallState>
  bool parse(std::string const& input, T& index,
             std::vector<std::string> const& allArgs,
             CallState&&... state) const
  {
    ParseMode parseState = ParseMode::Valid;

    if (this->Type == Values::Zero) {
      if (input.size() == this->Name.size()) {
        parseState =
          this->StoreCall(std::string{}, std::forward<CallState>(state)...)
          ? ParseMode::Valid
          : ParseMode::Invalid;
      } else {
        parseState = ParseMode::SyntaxError;
      }

    } else if (this->Type == Values::One || this->Type == Values::ZeroOrOne) {
      if (input.size() == this->Name.size()) {
        auto nextValueIndex = index + 1;
        if (nextValueIndex >= allArgs.size() ||
            allArgs[nextValueIndex][0] == '-') {
          if (this->Type == Values::ZeroOrOne) {
            parseState =
              this->StoreCall(std::string{}, std::forward<CallState>(state)...)
              ? ParseMode::Valid
              : ParseMode::Invalid;
          } else {
            parseState = ParseMode::ValueError;
          }
        } else {
          parseState = this->StoreCall(allArgs[nextValueIndex],
                                       std::forward<CallState>(state)...)
            ? ParseMode::Valid
            : ParseMode::Invalid;
          index = nextValueIndex;
        }
      } else {
        auto value = this->extract_single_value(input, parseState);
        if (parseState == ParseMode::Valid) {
          parseState =
            this->StoreCall(value, std::forward<CallState>(state)...)
            ? ParseMode::Valid
            : ParseMode::Invalid;
        }
      }
    } else if (this->Type == Values::Two) {
      if (input.size() == this->Name.size()) {
        if (index + 2 >= allArgs.size() || allArgs[index + 1][0] == '-' ||
            allArgs[index + 2][0] == '-') {
          parseState = ParseMode::ValueError;
        } else {
          index += 2;
          parseState =
            this->StoreCall(cmStrCat(allArgs[index - 1], ";", allArgs[index]),
                            std::forward<CallState>(state)...)
            ? ParseMode::Valid
            : ParseMode::Invalid;
        }
      }
    } else if (this->Type == Values::OneOrMore) {
      if (input.size() == this->Name.size()) {
        auto nextValueIndex = index + 1;
        if (nextValueIndex >= allArgs.size() ||
            allArgs[nextValueIndex][0] == '-') {
          parseState = ParseMode::ValueError;
        } else {
          std::string buffer = allArgs[nextValueIndex++];
          while (nextValueIndex < allArgs.size() &&
                 allArgs[nextValueIndex][0] != '-') {
            buffer = cmStrCat(buffer, ";", allArgs[nextValueIndex++]);
          }
          parseState =
            this->StoreCall(buffer, std::forward<CallState>(state)...)
            ? ParseMode::Valid
            : ParseMode::Invalid;
          index = (nextValueIndex - 1);
        }
      } else {
        auto value = this->extract_single_value(input, parseState);
        if (parseState == ParseMode::Valid) {
          parseState =
            this->StoreCall(value, std::forward<CallState>(state)...)
            ? ParseMode::Valid
            : ParseMode::Invalid;
        }
      }
    }

    if (parseState == ParseMode::SyntaxError) {
      cmSystemTools::Error(
        cmStrCat("'", input, "'", this->InvalidSyntaxMessage));
    } else if (parseState == ParseMode::ValueError) {
      cmSystemTools::Error(this->InvalidValueMessage);
    }
    return (parseState == ParseMode::Valid);
  }

  template <typename... Values>
  static std::function<FunctionSignature> setToTrue(Values&&... values)
  {
    return ArgumentLambdaHelper<FunctionSignature>::generateSetToTrue(
      std::forward<Values>(values)...);
  }

  template <typename... Values>
  static std::function<FunctionSignature> setToValue(Values&&... values)
  {
    return ArgumentLambdaHelper<FunctionSignature>::generateSetToValue(
      std::forward<Values>(values)...);
  }

private:
  template <typename T>
  class ArgumentLambdaHelper;

  template <typename... CallState>
  class ArgumentLambdaHelper<bool(const std::string&, CallState...)>
  {
  public:
    static std::function<bool(const std::string&, CallState...)>
    generateSetToTrue(bool& value1)
    {
      return [&value1](const std::string&, CallState&&...) -> bool {
        value1 = true;
        return true;
      };
    }

    static std::function<bool(const std::string&, CallState...)>
    generateSetToTrue(bool& value1, bool& value2)
    {
      return [&value1, &value2](const std::string&, CallState&&...) -> bool {
        value1 = true;
        value2 = true;
        return true;
      };
    }

    static std::function<bool(const std::string&, CallState...)>
    generateSetToValue(std::string& value1)
    {
      return [&value1](const std::string& arg, CallState&&...) -> bool {
        value1 = arg;
        return true;
      };
    }
  };

  std::string extract_single_value(std::string const& input,
                                   ParseMode& parseState) const
  {
    // parse the string to get the value
    auto possible_value = cm::string_view(input).substr(this->Name.size());
    if (possible_value.empty()) {
      parseState = ParseMode::ValueError;
    } else if (possible_value[0] == '=') {
      possible_value.remove_prefix(1);
      if (possible_value.empty()) {
        parseState = ParseMode::ValueError;
      }
    }
    if (parseState == ParseMode::Valid && possible_value[0] == ' ') {
      possible_value.remove_prefix(1);
    }
    return std::string(possible_value);
  }
};
