/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>
#include <vector>

#include <cmext/string_view>

#include "cmListFileCache.h"
#include "cmMessageType.h" // IWYU pragma: keep
#include "cmPolicies.h"
#include "cmValue.h"

class cmExpandedCommandArgument;
class cmMakefile;

class cmConditionEvaluator
{
public:
  cmConditionEvaluator(cmMakefile& makefile, cmListFileBacktrace bt);

  // this is a shared function for both If and Else to determine if the
  // arguments were valid, and if so, was the response true. If there is
  // an error, the errorString will be set.
  bool IsTrue(const std::vector<cmExpandedCommandArgument>& args,
              std::string& errorString, MessageType& status);

private:
  class cmArgumentList;

  cmValue GetDefinitionIfUnquoted(
    const cmExpandedCommandArgument& argument) const;

  cmValue GetVariableOrString(const cmExpandedCommandArgument& argument) const;

  bool IsKeyword(cm::static_string_view keyword,
                 const cmExpandedCommandArgument& argument) const;

  bool GetBooleanValue(cmExpandedCommandArgument& arg) const;

  template <int N>
  int matchKeysImpl(const cmExpandedCommandArgument&);

  template <int N, typename T, typename... Keys>
  int matchKeysImpl(const cmExpandedCommandArgument&, T, Keys...);

  template <typename... Keys>
  int matchKeys(const cmExpandedCommandArgument&, Keys...);

  bool HandleLevel0(cmArgumentList& newArgs, std::string& errorString,
                    MessageType& status);

  bool HandleLevel1(cmArgumentList& newArgs, std::string&, MessageType&);

  bool HandleLevel2(cmArgumentList& newArgs, std::string& errorString,
                    MessageType& status);

  bool HandleLevel3(cmArgumentList& newArgs, std::string& errorString,
                    MessageType& status);

  bool HandleLevel4(cmArgumentList& newArgs, std::string& errorString,
                    MessageType& status);

  cmMakefile& Makefile;
  cmListFileBacktrace Backtrace;
  cmPolicies::PolicyStatus Policy64Status;
  cmPolicies::PolicyStatus Policy139Status;
};
