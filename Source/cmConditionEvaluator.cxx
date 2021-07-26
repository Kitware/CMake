/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmConditionEvaluator.h"

#include <array>
#include <cstdio>
#include <cstdlib>
#include <functional>
#include <iterator>
#include <sstream>
#include <utility>

#include <cm/string_view>
#include <cmext/algorithm>

#include "cmsys/RegularExpression.hxx"

#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmProperty.h"
#include "cmState.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmake.h"

namespace {
auto const keyAND = "AND"_s;
auto const keyCOMMAND = "COMMAND"_s;
auto const keyDEFINED = "DEFINED"_s;
auto const keyEQUAL = "EQUAL"_s;
auto const keyEXISTS = "EXISTS"_s;
auto const keyGREATER = "GREATER"_s;
auto const keyGREATER_EQUAL = "GREATER_EQUAL"_s;
auto const keyIN_LIST = "IN_LIST"_s;
auto const keyIS_ABSOLUTE = "IS_ABSOLUTE"_s;
auto const keyIS_DIRECTORY = "IS_DIRECTORY"_s;
auto const keyIS_NEWER_THAN = "IS_NEWER_THAN"_s;
auto const keyIS_SYMLINK = "IS_SYMLINK"_s;
auto const keyLESS = "LESS"_s;
auto const keyLESS_EQUAL = "LESS_EQUAL"_s;
auto const keyMATCHES = "MATCHES"_s;
auto const keyNOT = "NOT"_s;
auto const keyOR = "OR"_s;
auto const keyParenL = "("_s;
auto const keyParenR = ")"_s;
auto const keyPOLICY = "POLICY"_s;
auto const keySTREQUAL = "STREQUAL"_s;
auto const keySTRGREATER = "STRGREATER"_s;
auto const keySTRGREATER_EQUAL = "STRGREATER_EQUAL"_s;
auto const keySTRLESS = "STRLESS"_s;
auto const keySTRLESS_EQUAL = "STRLESS_EQUAL"_s;
auto const keyTARGET = "TARGET"_s;
auto const keyTEST = "TEST"_s;
auto const keyVERSION_EQUAL = "VERSION_EQUAL"_s;
auto const keyVERSION_GREATER = "VERSION_GREATER"_s;
auto const keyVERSION_GREATER_EQUAL = "VERSION_GREATER_EQUAL"_s;
auto const keyVERSION_LESS = "VERSION_LESS"_s;
auto const keyVERSION_LESS_EQUAL = "VERSION_LESS_EQUAL"_s;

std::array<const char* const, 2> const ZERO_ONE_XLAT = { "0", "1" };

inline void IncrementArguments(
  cmConditionEvaluator::cmArgumentList& newArgs,
  cmConditionEvaluator::cmArgumentList::iterator& argP1,
  cmConditionEvaluator::cmArgumentList::iterator& argP2)
{
  if (argP1 != newArgs.end()) {
    argP2 = ++argP1;
    using difference_type =
      cmConditionEvaluator::cmArgumentList::difference_type;
    std::advance(argP2, difference_type(argP1 != newArgs.end()));
  }
}

void HandlePredicate(const bool value, bool& reducible,
                     cmConditionEvaluator::cmArgumentList::iterator& arg,
                     cmConditionEvaluator::cmArgumentList& newArgs,
                     cmConditionEvaluator::cmArgumentList::iterator& argP1,
                     cmConditionEvaluator::cmArgumentList::iterator& argP2)
{
  *arg = cmExpandedCommandArgument(ZERO_ONE_XLAT[value], true);
  newArgs.erase(argP1);
  argP1 = arg;
  IncrementArguments(newArgs, argP1, argP2);
  reducible = true;
}

void HandleBinaryOp(const bool value, bool& reducible,
                    cmConditionEvaluator::cmArgumentList::iterator& arg,
                    cmConditionEvaluator::cmArgumentList& newArgs,
                    cmConditionEvaluator::cmArgumentList::iterator& argP1,
                    cmConditionEvaluator::cmArgumentList::iterator& argP2)
{
  *arg = cmExpandedCommandArgument(ZERO_ONE_XLAT[value], true);
  newArgs.erase(argP2);
  newArgs.erase(argP1);
  argP1 = arg;
  IncrementArguments(newArgs, argP1, argP2);
  reducible = true;
}

} // anonymous namespace

cmConditionEvaluator::cmConditionEvaluator(cmMakefile& makefile,
                                           cmListFileBacktrace bt)
  : Makefile(makefile)
  , Backtrace(std::move(bt))
  , Policy12Status(makefile.GetPolicyStatus(cmPolicies::CMP0012))
  , Policy54Status(makefile.GetPolicyStatus(cmPolicies::CMP0054))
  , Policy57Status(makefile.GetPolicyStatus(cmPolicies::CMP0057))
  , Policy64Status(makefile.GetPolicyStatus(cmPolicies::CMP0064))
{
}

//=========================================================================
// order of operations,
// 1.   ( )   -- parenthetical groups
// 2.  IS_DIRECTORY EXISTS COMMAND DEFINED etc predicates
// 3. MATCHES LESS GREATER EQUAL STRLESS STRGREATER STREQUAL etc binary ops
// 4. NOT
// 5. AND OR
//
// There is an issue on whether the arguments should be values of references,
// for example IF (FOO AND BAR) should that compare the strings FOO and BAR
// or should it really do IF (${FOO} AND ${BAR}) Currently IS_DIRECTORY
// EXISTS COMMAND and DEFINED all take values. EQUAL, LESS and GREATER can
// take numeric values or variable names. STRLESS and STRGREATER take
// variable names but if the variable name is not found it will use the name
// directly. AND OR take variables or the values 0 or 1.

bool cmConditionEvaluator::IsTrue(
  const std::vector<cmExpandedCommandArgument>& args, std::string& errorString,
  MessageType& status)
{
  errorString.clear();

  // handle empty invocation
  if (args.empty()) {
    return false;
  }

  // store the reduced args in this vector
  cmArgumentList newArgs(args.begin(), args.end());

  // now loop through the arguments and see if we can reduce any of them
  // we do this multiple times. Once for each level of precedence
  // parens
  if (!this->HandleLevel0(newArgs, errorString, status)) {
    return false;
  }
  // predicates
  if (!this->HandleLevel1(newArgs, errorString, status)) {
    return false;
  }
  // binary ops
  if (!this->HandleLevel2(newArgs, errorString, status)) {
    return false;
  }

  // NOT
  if (!this->HandleLevel3(newArgs, errorString, status)) {
    return false;
  }
  // AND OR
  if (!this->HandleLevel4(newArgs, errorString, status)) {
    return false;
  }

  // now at the end there should only be one argument left
  if (newArgs.size() != 1) {
    errorString = "Unknown arguments specified";
    status = MessageType::FATAL_ERROR;
    return false;
  }

  return this->GetBooleanValueWithAutoDereference(newArgs.front(), errorString,
                                                  status, true);
}

//=========================================================================
cmProp cmConditionEvaluator::GetDefinitionIfUnquoted(
  cmExpandedCommandArgument const& argument) const
{
  if ((this->Policy54Status != cmPolicies::WARN &&
       this->Policy54Status != cmPolicies::OLD) &&
      argument.WasQuoted()) {
    return nullptr;
  }

  cmProp def = this->Makefile.GetDefinition(argument.GetValue());

  if (def && argument.WasQuoted() &&
      this->Policy54Status == cmPolicies::WARN) {
    if (!this->Makefile.HasCMP0054AlreadyBeenReported(this->Backtrace.Top())) {
      std::ostringstream e;
      e << (cmPolicies::GetPolicyWarning(cmPolicies::CMP0054)) << "\n";
      e << "Quoted variables like \"" << argument.GetValue()
        << "\" will no longer be dereferenced "
           "when the policy is set to NEW.  "
           "Since the policy is not set the OLD behavior will be used.";

      this->Makefile.GetCMakeInstance()->IssueMessage(
        MessageType::AUTHOR_WARNING, e.str(), this->Backtrace);
    }
  }

  return def;
}

//=========================================================================
cmProp cmConditionEvaluator::GetVariableOrString(
  const cmExpandedCommandArgument& argument) const
{
  cmProp def = this->GetDefinitionIfUnquoted(argument);

  if (!def) {
    def = &argument.GetValue();
  }

  return def;
}

//=========================================================================
bool cmConditionEvaluator::IsKeyword(cm::string_view keyword,
                                     cmExpandedCommandArgument& argument) const
{
  if ((this->Policy54Status != cmPolicies::WARN &&
       this->Policy54Status != cmPolicies::OLD) &&
      argument.WasQuoted()) {
    return false;
  }

  const auto isKeyword = argument.GetValue() == keyword;

  if (isKeyword && argument.WasQuoted() &&
      this->Policy54Status == cmPolicies::WARN) {
    if (!this->Makefile.HasCMP0054AlreadyBeenReported(this->Backtrace.Top())) {
      std::ostringstream e;
      e << cmPolicies::GetPolicyWarning(cmPolicies::CMP0054) << "\n";
      e << "Quoted keywords like \"" << argument.GetValue()
        << "\" will no longer be interpreted as keywords "
           "when the policy is set to NEW.  "
           "Since the policy is not set the OLD behavior will be used.";

      this->Makefile.GetCMakeInstance()->IssueMessage(
        MessageType::AUTHOR_WARNING, e.str(), this->Backtrace);
    }
  }

  return isKeyword;
}

//=========================================================================
bool cmConditionEvaluator::GetBooleanValue(
  cmExpandedCommandArgument& arg) const
{
  // Check basic constants.
  if (arg == "0") {
    return false;
  }
  if (arg == "1") {
    return true;
  }

  // Check named constants.
  if (cmIsOn(arg.GetValue())) {
    return true;
  }
  if (cmIsOff(arg.GetValue())) {
    return false;
  }

  // Check for numbers.
  if (!arg.empty()) {
    char* end;
    const double d = std::strtod(arg.GetValue().c_str(), &end);
    if (*end == '\0') {
      // The whole string is a number.  Use C conversion to bool.
      return static_cast<bool>(d);
    }
  }

  // Check definition.
  cmProp def = this->GetDefinitionIfUnquoted(arg);
  return !cmIsOff(def);
}

//=========================================================================
// Boolean value behavior from CMake 2.6.4 and below.
bool cmConditionEvaluator::GetBooleanValueOld(
  cmExpandedCommandArgument const& arg, bool const one) const
{
  if (one) {
    // Old IsTrue behavior for single argument.
    if (arg == "0") {
      return false;
    }
    if (arg == "1") {
      return true;
    }
    cmProp def = this->GetDefinitionIfUnquoted(arg);
    return !cmIsOff(def);
  }
  // Old GetVariableOrNumber behavior.
  cmProp def = this->GetDefinitionIfUnquoted(arg);
  if (!def && std::atoi(arg.GetValue().c_str())) {
    def = &arg.GetValue();
  }
  return !cmIsOff(def);
}

//=========================================================================
// returns the resulting boolean value
bool cmConditionEvaluator::GetBooleanValueWithAutoDereference(
  cmExpandedCommandArgument& newArg, std::string& errorString,
  MessageType& status, bool const oneArg) const
{
  // Use the policy if it is set.
  if (this->Policy12Status == cmPolicies::NEW) {
    return this->GetBooleanValue(newArg);
  }
  if (this->Policy12Status == cmPolicies::OLD) {
    return this->GetBooleanValueOld(newArg, oneArg);
  }

  // Check policy only if old and new results differ.
  const auto newResult = this->GetBooleanValue(newArg);
  const auto oldResult = this->GetBooleanValueOld(newArg, oneArg);
  if (newResult != oldResult) {
    switch (this->Policy12Status) {
      case cmPolicies::WARN:
        errorString = "An argument named \"" + newArg.GetValue() +
          "\" appears in a conditional statement.  " +
          cmPolicies::GetPolicyWarning(cmPolicies::CMP0012);
        status = MessageType::AUTHOR_WARNING;
        CM_FALLTHROUGH;
      case cmPolicies::OLD:
        return oldResult;
      case cmPolicies::REQUIRED_IF_USED:
      case cmPolicies::REQUIRED_ALWAYS: {
        errorString = "An argument named \"" + newArg.GetValue() +
          "\" appears in a conditional statement.  " +
          cmPolicies::GetRequiredPolicyError(cmPolicies::CMP0012);
        status = MessageType::FATAL_ERROR;
      }
      case cmPolicies::NEW:
        break;
    }
  }
  return newResult;
}

//=========================================================================
// level 0 processes parenthetical expressions
bool cmConditionEvaluator::HandleLevel0(cmArgumentList& newArgs,
                                        std::string& errorString,
                                        MessageType& status)
{
  bool reducible;
  do {
    reducible = false;
    for (auto arg = newArgs.begin(); arg != newArgs.end(); ++arg) {
      if (this->IsKeyword(keyParenL, *arg)) {
        // search for the closing paren for this opening one
        cmArgumentList::iterator argClose;
        argClose = arg;
        argClose++;
        auto depth = 1u;
        while (argClose != newArgs.end() && depth) {
          if (this->IsKeyword(keyParenL, *argClose)) {
            depth++;
          }
          if (this->IsKeyword(keyParenR, *argClose)) {
            depth--;
          }
          argClose++;
        }
        if (depth) {
          errorString = "mismatched parenthesis in condition";
          status = MessageType::FATAL_ERROR;
          return false;
        }
        // store the reduced args in this vector
        std::vector<cmExpandedCommandArgument> newArgs2;

        // copy to the list structure
        auto argP1 = std::next(arg);
        cm::append(newArgs2, argP1, argClose);
        newArgs2.pop_back();
        // now recursively invoke IsTrue to handle the values inside the
        // parenthetical expression
        const auto value = this->IsTrue(newArgs2, errorString, status);
        *arg = cmExpandedCommandArgument(ZERO_ONE_XLAT[value], true);
        argP1 = std::next(arg);
        // remove the now evaluated parenthetical expression
        newArgs.erase(argP1, argClose);
      }
    }
  } while (reducible);
  return true;
}

//=========================================================================
// level one handles most predicates except for NOT
bool cmConditionEvaluator::HandleLevel1(cmArgumentList& newArgs, std::string&,
                                        MessageType&)
{
  bool reducible;
  do {
    reducible = false;
    for (auto arg = newArgs.begin(), argP1 = arg, argP2 = arg;
         arg != newArgs.end(); argP1 = ++arg) {
      IncrementArguments(newArgs, argP1, argP2);
      // does a file exist
      if (this->IsKeyword(keyEXISTS, *arg) && argP1 != newArgs.end()) {
        HandlePredicate(cmSystemTools::FileExists(argP1->GetValue()),
                        reducible, arg, newArgs, argP1, argP2);
      }
      // does a directory with this name exist
      if (this->IsKeyword(keyIS_DIRECTORY, *arg) && argP1 != newArgs.end()) {
        HandlePredicate(cmSystemTools::FileIsDirectory(argP1->GetValue()),
                        reducible, arg, newArgs, argP1, argP2);
      }
      // does a symlink with this name exist
      if (this->IsKeyword(keyIS_SYMLINK, *arg) && argP1 != newArgs.end()) {
        HandlePredicate(cmSystemTools::FileIsSymlink(argP1->GetValue()),
                        reducible, arg, newArgs, argP1, argP2);
      }
      // is the given path an absolute path ?
      if (this->IsKeyword(keyIS_ABSOLUTE, *arg) && argP1 != newArgs.end()) {
        HandlePredicate(cmSystemTools::FileIsFullPath(argP1->GetValue()),
                        reducible, arg, newArgs, argP1, argP2);
      }
      // does a command exist
      if (this->IsKeyword(keyCOMMAND, *arg) && argP1 != newArgs.end()) {
        HandlePredicate(
          this->Makefile.GetState()->GetCommand(argP1->GetValue()) != nullptr,
          reducible, arg, newArgs, argP1, argP2);
      }
      // does a policy exist
      if (this->IsKeyword(keyPOLICY, *arg) && argP1 != newArgs.end()) {
        cmPolicies::PolicyID pid;
        HandlePredicate(
          cmPolicies::GetPolicyID(argP1->GetValue().c_str(), pid), reducible,
          arg, newArgs, argP1, argP2);
      }
      // does a target exist
      if (this->IsKeyword(keyTARGET, *arg) && argP1 != newArgs.end()) {
        HandlePredicate(this->Makefile.FindTargetToUse(argP1->GetValue()) !=
                          nullptr,
                        reducible, arg, newArgs, argP1, argP2);
      }
      // does a test exist
      if (this->Policy64Status != cmPolicies::OLD &&
          this->Policy64Status != cmPolicies::WARN) {
        if (this->IsKeyword(keyTEST, *arg) && argP1 != newArgs.end()) {
          HandlePredicate(this->Makefile.GetTest(argP1->GetValue()) != nullptr,
                          reducible, arg, newArgs, argP1, argP2);
        }
      } else if (this->Policy64Status == cmPolicies::WARN &&
                 this->IsKeyword(keyTEST, *arg)) {
        std::ostringstream e;
        e << cmPolicies::GetPolicyWarning(cmPolicies::CMP0064) << "\n";
        e << "TEST will be interpreted as an operator "
             "when the policy is set to NEW.  "
             "Since the policy is not set the OLD behavior will be used.";

        this->Makefile.IssueMessage(MessageType::AUTHOR_WARNING, e.str());
      }
      // is a variable defined
      if (this->IsKeyword(keyDEFINED, *arg) && argP1 != newArgs.end()) {
        const auto argP1len = argP1->GetValue().size();
        auto bdef = false;
        if (argP1len > 4 && cmHasLiteralPrefix(argP1->GetValue(), "ENV{") &&
            argP1->GetValue().operator[](argP1len - 1) == '}') {
          const auto env = argP1->GetValue().substr(4, argP1len - 5);
          bdef = cmSystemTools::HasEnv(env);
        } else if (argP1len > 6 &&
                   cmHasLiteralPrefix(argP1->GetValue(), "CACHE{") &&
                   argP1->GetValue().operator[](argP1len - 1) == '}') {
          const auto cache = argP1->GetValue().substr(6, argP1len - 7);
          bdef =
            this->Makefile.GetState()->GetCacheEntryValue(cache) != nullptr;
        } else {
          bdef = this->Makefile.IsDefinitionSet(argP1->GetValue());
        }
        HandlePredicate(bdef, reducible, arg, newArgs, argP1, argP2);
      }
    }
  } while (reducible);
  return true;
}

//=========================================================================
// level two handles most binary operations except for AND  OR
bool cmConditionEvaluator::HandleLevel2(cmArgumentList& newArgs,
                                        std::string& errorString,
                                        MessageType& status)
{
  bool reducible;
  std::string def_buf;
  cmProp def;
  cmProp def2;
  do {
    reducible = false;
    for (auto arg = newArgs.begin(), argP1 = arg, argP2 = arg;
         arg != newArgs.end(); argP1 = ++arg) {
      IncrementArguments(newArgs, argP1, argP2);
      if (argP1 != newArgs.end() && argP2 != newArgs.end() &&
          this->IsKeyword(keyMATCHES, *argP1)) {
        def = this->GetDefinitionIfUnquoted(*arg);
        if (!def) {
          def = &arg->GetValue();
        } else if (cmHasLiteralPrefix(arg->GetValue(), "CMAKE_MATCH_")) {
          // The string to match is owned by our match result variables.
          // Move it to our own buffer before clearing them.
          def_buf = *def;
          def = &def_buf;
        }
        const auto& rex = argP2->GetValue();
        this->Makefile.ClearMatches();
        cmsys::RegularExpression regEntry;
        if (!regEntry.compile(rex)) {
          std::ostringstream error;
          error << "Regular expression \"" << rex << "\" cannot compile";
          errorString = error.str();
          status = MessageType::FATAL_ERROR;
          return false;
        }
        if (regEntry.find(*def)) {
          this->Makefile.StoreMatches(regEntry);
          *arg = cmExpandedCommandArgument("1", true);
        } else {
          *arg = cmExpandedCommandArgument("0", true);
        }
        newArgs.erase(argP2);
        newArgs.erase(argP1);
        argP1 = arg;
        IncrementArguments(newArgs, argP1, argP2);
        reducible = true;
      }

      if (argP1 != newArgs.end() && this->IsKeyword(keyMATCHES, *arg)) {
        *arg = cmExpandedCommandArgument("0", true);
        newArgs.erase(argP1);
        argP1 = arg;
        IncrementArguments(newArgs, argP1, argP2);
        reducible = true;
      }

      if (argP1 != newArgs.end() && argP2 != newArgs.end() &&
          (this->IsKeyword(keyLESS, *argP1) ||
           this->IsKeyword(keyLESS_EQUAL, *argP1) ||
           this->IsKeyword(keyGREATER, *argP1) ||
           this->IsKeyword(keyGREATER_EQUAL, *argP1) ||
           this->IsKeyword(keyEQUAL, *argP1))) {
        def = this->GetVariableOrString(*arg);
        def2 = this->GetVariableOrString(*argP2);
        double lhs;
        double rhs;
        bool result;
        if (std::sscanf(def->c_str(), "%lg", &lhs) != 1 ||
            std::sscanf(def2->c_str(), "%lg", &rhs) != 1) {
          result = false;
        } else if (argP1->GetValue() == keyLESS) {
          result = (lhs < rhs);
        } else if (argP1->GetValue() == keyLESS_EQUAL) {
          result = (lhs <= rhs);
        } else if (argP1->GetValue() == keyGREATER) {
          result = (lhs > rhs);
        } else if (argP1->GetValue() == keyGREATER_EQUAL) {
          result = (lhs >= rhs);
        } else {
          result = (lhs == rhs);
        }
        HandleBinaryOp(result, reducible, arg, newArgs, argP1, argP2);
      }

      if (argP1 != newArgs.end() && argP2 != newArgs.end() &&
          (this->IsKeyword(keySTRLESS, *argP1) ||
           this->IsKeyword(keySTRLESS_EQUAL, *argP1) ||
           this->IsKeyword(keySTRGREATER, *argP1) ||
           this->IsKeyword(keySTRGREATER_EQUAL, *argP1) ||
           this->IsKeyword(keySTREQUAL, *argP1))) {
        def = this->GetVariableOrString(*arg);
        def2 = this->GetVariableOrString(*argP2);
        const int val = (*def).compare(*def2);
        bool result;
        if (argP1->GetValue() == keySTRLESS) {
          result = (val < 0);
        } else if (argP1->GetValue() == keySTRLESS_EQUAL) {
          result = (val <= 0);
        } else if (argP1->GetValue() == keySTRGREATER) {
          result = (val > 0);
        } else if (argP1->GetValue() == keySTRGREATER_EQUAL) {
          result = (val >= 0);
        } else // strequal
        {
          result = (val == 0);
        }
        HandleBinaryOp(result, reducible, arg, newArgs, argP1, argP2);
      }

      if (argP1 != newArgs.end() && argP2 != newArgs.end() &&
          (this->IsKeyword(keyVERSION_LESS, *argP1) ||
           this->IsKeyword(keyVERSION_LESS_EQUAL, *argP1) ||
           this->IsKeyword(keyVERSION_GREATER, *argP1) ||
           this->IsKeyword(keyVERSION_GREATER_EQUAL, *argP1) ||
           this->IsKeyword(keyVERSION_EQUAL, *argP1))) {
        def = this->GetVariableOrString(*arg);
        def2 = this->GetVariableOrString(*argP2);
        cmSystemTools::CompareOp op;
        if (argP1->GetValue() == keyVERSION_LESS) {
          op = cmSystemTools::OP_LESS;
        } else if (argP1->GetValue() == keyVERSION_LESS_EQUAL) {
          op = cmSystemTools::OP_LESS_EQUAL;
        } else if (argP1->GetValue() == keyVERSION_GREATER) {
          op = cmSystemTools::OP_GREATER;
        } else if (argP1->GetValue() == keyVERSION_GREATER_EQUAL) {
          op = cmSystemTools::OP_GREATER_EQUAL;
        } else { // version_equal
          op = cmSystemTools::OP_EQUAL;
        }
        const auto result =
          cmSystemTools::VersionCompare(op, def->c_str(), def2->c_str());
        HandleBinaryOp(result, reducible, arg, newArgs, argP1, argP2);
      }

      // is file A newer than file B
      if (argP1 != newArgs.end() && argP2 != newArgs.end() &&
          this->IsKeyword(keyIS_NEWER_THAN, *argP1)) {
        auto fileIsNewer = 0;
        cmsys::Status ftcStatus = cmSystemTools::FileTimeCompare(
          arg->GetValue(), argP2->GetValue(), &fileIsNewer);
        HandleBinaryOp((!ftcStatus || fileIsNewer == 1 || fileIsNewer == 0),
                       reducible, arg, newArgs, argP1, argP2);
      }

      if (argP1 != newArgs.end() && argP2 != newArgs.end() &&
          this->IsKeyword(keyIN_LIST, *argP1)) {
        if (this->Policy57Status != cmPolicies::OLD &&
            this->Policy57Status != cmPolicies::WARN) {
          auto result = false;

          def = this->GetVariableOrString(*arg);
          def2 = this->Makefile.GetDefinition(argP2->GetValue());

          if (def2) {
            result = cm::contains(cmExpandedList(*def2, true), *def);
          }

          HandleBinaryOp(result, reducible, arg, newArgs, argP1, argP2);
        } else if (this->Policy57Status == cmPolicies::WARN) {
          std::ostringstream e;
          e << cmPolicies::GetPolicyWarning(cmPolicies::CMP0057) << "\n";
          e << "IN_LIST will be interpreted as an operator "
               "when the policy is set to NEW.  "
               "Since the policy is not set the OLD behavior will be used.";

          this->Makefile.IssueMessage(MessageType::AUTHOR_WARNING, e.str());
        }
      }
    }
  } while (reducible);
  return true;
}

//=========================================================================
// level 3 handles NOT
bool cmConditionEvaluator::HandleLevel3(cmArgumentList& newArgs,
                                        std::string& errorString,
                                        MessageType& status)
{
  bool reducible;
  do {
    reducible = false;
    for (auto arg = newArgs.begin(), argP1 = arg, argP2 = arg;
         arg != newArgs.end(); argP1 = ++arg) {
      IncrementArguments(newArgs, argP1, argP2);
      if (argP1 != newArgs.end() && this->IsKeyword(keyNOT, *arg)) {
        const auto rhs = this->GetBooleanValueWithAutoDereference(
          *argP1, errorString, status);
        HandlePredicate(!rhs, reducible, arg, newArgs, argP1, argP2);
      }
    }
  } while (reducible);
  return true;
}

//=========================================================================
// level 4 handles AND OR
bool cmConditionEvaluator::HandleLevel4(cmArgumentList& newArgs,
                                        std::string& errorString,
                                        MessageType& status)
{
  bool reducible;
  bool lhs;
  bool rhs;
  do {
    reducible = false;
    for (auto arg = newArgs.begin(), argP1 = arg, argP2 = arg;
         arg != newArgs.end(); argP1 = ++arg) {
      IncrementArguments(newArgs, argP1, argP2);
      if (argP1 != newArgs.end() && this->IsKeyword(keyAND, *argP1) &&
          argP2 != newArgs.end()) {
        lhs =
          this->GetBooleanValueWithAutoDereference(*arg, errorString, status);
        rhs = this->GetBooleanValueWithAutoDereference(*argP2, errorString,
                                                       status);
        HandleBinaryOp((lhs && rhs), reducible, arg, newArgs, argP1, argP2);
      }

      if (argP1 != newArgs.end() && this->IsKeyword(keyOR, *argP1) &&
          argP2 != newArgs.end()) {
        lhs =
          this->GetBooleanValueWithAutoDereference(*arg, errorString, status);
        rhs = this->GetBooleanValueWithAutoDereference(*argP2, errorString,
                                                       status);
        HandleBinaryOp((lhs || rhs), reducible, arg, newArgs, argP1, argP2);
      }
    }
  } while (reducible);
  return true;
}
