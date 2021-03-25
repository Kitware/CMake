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

  std::string InvalidSyntaxMessage;
  std::string InvalidValueMessage;
  std::string Name;
  Values Type;
  std::function<FunctionSignature> StoreCall;

  template <typename FunctionType>
  cmCommandLineArgument(std::string n, Values t, FunctionType&& func)
    : InvalidSyntaxMessage(cmStrCat("Invalid syntax used with ", n))
    , InvalidValueMessage(cmStrCat("Invalid value used with ", n))
    , Name(std::move(n))
    , Type(t)
    , StoreCall(std::forward<FunctionType>(func))
  {
  }

  template <typename FunctionType>
  cmCommandLineArgument(std::string n, std::string failedMsg, Values t,
                        FunctionType&& func)
    : InvalidSyntaxMessage(cmStrCat("Invalid syntax used with ", n))
    , InvalidValueMessage(std::move(failedMsg))
    , Name(std::move(n))
    , Type(t)
    , StoreCall(std::forward<FunctionType>(func))
  {
  }

  bool matches(std::string const& input) const
  {
    return (this->Type == Values::Zero) ? (input == this->Name)
                                        : cmHasPrefix(input, this->Name);
  }

  template <typename T, typename... CallState>
  bool parse(std::string const& input, T& index,
             std::vector<std::string> const& allArgs,
             CallState&&... state) const
  {
    enum class ParseMode
    {
      Valid,
      Invalid,
      SyntaxError,
      ValueError
    };
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
        ++index;
        if (index >= allArgs.size() || allArgs[index][0] == '-') {
          if (this->Type == Values::ZeroOrOne) {
            parseState =
              this->StoreCall(std::string{}, std::forward<CallState>(state)...)
              ? ParseMode::Valid
              : ParseMode::Invalid;
          } else {
            parseState = ParseMode::ValueError;
          }
        } else {
          parseState =
            this->StoreCall(allArgs[index], std::forward<CallState>(state)...)
            ? ParseMode::Valid
            : ParseMode::Invalid;
        }
      } else {
        // parse the string to get the value
        auto possible_value = cm::string_view(input).substr(this->Name.size());
        if (possible_value.empty()) {
          parseState = ParseMode::SyntaxError;
          parseState = ParseMode::ValueError;
        } else if (possible_value[0] == '=') {
          possible_value.remove_prefix(1);
          if (possible_value.empty()) {
            parseState = ParseMode::ValueError;
          } else {
            parseState = this->StoreCall(std::string(possible_value),
                                         std::forward<CallState>(state)...)
              ? ParseMode::Valid
              : ParseMode::Invalid;
          }
        }
        if (parseState == ParseMode::Valid) {
          parseState = this->StoreCall(std::string(possible_value),
                                       std::forward<CallState>(state)...)
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
        if (nextValueIndex >= allArgs.size() || allArgs[index + 1][0] == '-') {
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
        }
      }
    }

    if (parseState == ParseMode::SyntaxError) {
      cmSystemTools::Error(this->InvalidSyntaxMessage);
    } else if (parseState == ParseMode::ValueError) {
      cmSystemTools::Error(this->InvalidValueMessage);
    }
    return (parseState == ParseMode::Valid);
  }
};
