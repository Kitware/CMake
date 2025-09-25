/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmConditionEvaluator.h"

#include <array>
#include <cstdio>
#include <cstdlib>
#include <functional>
#include <iterator>
#include <list>
#include <sstream>
#include <utility>

#include <cm/string_view>
#include <cmext/algorithm>

#include "cmsys/RegularExpression.hxx"

#include "cmCMakePath.h"
#include "cmExpandedCommandArgument.h"
#include "cmList.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmState.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmValue.h"

namespace {
auto const keyAND = "AND"_s;
auto const keyCOMMAND = "COMMAND"_s;
auto const keyDEFINED = "DEFINED"_s;
auto const keyEQUAL = "EQUAL"_s;
auto const keyEXISTS = "EXISTS"_s;
auto const keyIS_READABLE = "IS_READABLE"_s;
auto const keyIS_WRITABLE = "IS_WRITABLE"_s;
auto const keyIS_EXECUTABLE = "IS_EXECUTABLE"_s;
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
auto const keyPATH_EQUAL = "PATH_EQUAL"_s;

cmSystemTools::CompareOp const MATCH2CMPOP[5] = {
  cmSystemTools::OP_LESS, cmSystemTools::OP_LESS_EQUAL,
  cmSystemTools::OP_GREATER, cmSystemTools::OP_GREATER_EQUAL,
  cmSystemTools::OP_EQUAL
};

// Run-Time to Compile-Time template selector
template <template <typename> class Comp, template <typename> class... Ops>
struct cmRt2CtSelector
{
  template <typename T>
  static bool eval(int r, T lhs, T rhs)
  {
    switch (r) {
      case 0:
        return false;
      case 1:
        return Comp<T>()(lhs, rhs);
      default:
        return cmRt2CtSelector<Ops...>::eval(r - 1, lhs, rhs);
    }
  }
};

template <template <typename> class Comp>
struct cmRt2CtSelector<Comp>
{
  template <typename T>
  static bool eval(int r, T lhs, T rhs)
  {
    return r == 1 && Comp<T>()(lhs, rhs);
  }
};

std::string bool2string(bool const value)
{
  return std::string(static_cast<std::size_t>(1),
                     static_cast<char>('0' + static_cast<int>(value)));
}

bool looksLikeSpecialVariable(std::string const& var,
                              cm::static_string_view prefix,
                              std::size_t const varNameLen)
{
  // NOTE Expecting a variable name at least 1 char length:
  // <prefix> + `{` + <varname> + `}`
  return ((prefix.size() + 3) <= varNameLen) &&
    cmHasPrefix(var, cmStrCat(prefix, '{')) && var[varNameLen - 1] == '}';
}
} // anonymous namespace

#if defined(__SUNPRO_CC)
#  define CM_INHERIT_CTOR(Class, Base, Tpl)                                   \
    template <typename... Args>                                               \
    Class(Args&&... args)                                                     \
      : Base Tpl(std::forward<Args>(args)...)                                 \
    {                                                                         \
    }
#else
#  define CM_INHERIT_CTOR(Class, Base, Tpl) using Base Tpl ::Base
#endif

// BEGIN cmConditionEvaluator::cmArgumentList
class cmConditionEvaluator::cmArgumentList
  : public std::list<cmExpandedCommandArgument>
{
  using base_t = std::list<cmExpandedCommandArgument>;

public:
  CM_INHERIT_CTOR(cmArgumentList, list, <cmExpandedCommandArgument>);

  class CurrentAndNextIter
  {
    friend class cmConditionEvaluator::cmArgumentList;

  public:
    base_t::iterator current;
    base_t::iterator next;

    CurrentAndNextIter advance(base_t& args)
    {
      this->current = std::next(this->current);
      this->next =
        std::next(this->current,
                  static_cast<difference_type>(this->current != args.end()));
      return *this;
    }

  private:
    CurrentAndNextIter(base_t& args)
      : current(args.begin())
      , next(
          std::next(this->current,
                    static_cast<difference_type>(this->current != args.end())))
    {
    }
  };

  class CurrentAndTwoMoreIter
  {
    friend class cmConditionEvaluator::cmArgumentList;

  public:
    base_t::iterator current;
    base_t::iterator next;
    base_t::iterator nextnext;

    CurrentAndTwoMoreIter advance(base_t& args)
    {
      this->current = std::next(this->current);
      this->next =
        std::next(this->current,
                  static_cast<difference_type>(this->current != args.end()));
      this->nextnext = std::next(
        this->next, static_cast<difference_type>(this->next != args.end()));
      return *this;
    }

  private:
    CurrentAndTwoMoreIter(base_t& args)
      : current(args.begin())
      , next(
          std::next(this->current,
                    static_cast<difference_type>(this->current != args.end())))
      , nextnext(std::next(
          this->next, static_cast<difference_type>(this->next != args.end())))
    {
    }
  };

  CurrentAndNextIter make2ArgsIterator() { return *this; }
  CurrentAndTwoMoreIter make3ArgsIterator() { return *this; }

  template <typename Iter>
  void ReduceOneArg(bool const value, Iter args)
  {
    *args.current = cmExpandedCommandArgument(bool2string(value), true);
    this->erase(args.next);
  }

  void ReduceTwoArgs(bool const value, CurrentAndTwoMoreIter args)
  {
    *args.current = cmExpandedCommandArgument(bool2string(value), true);
    this->erase(args.nextnext);
    this->erase(args.next);
  }
};

// END cmConditionEvaluator::cmArgumentList

cmConditionEvaluator::cmConditionEvaluator(cmMakefile& makefile,
                                           cmListFileBacktrace bt)
  : Makefile(makefile)
  , Backtrace(std::move(bt))
  , Policy139Status(makefile.GetPolicyStatus(cmPolicies::CMP0139))
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
  std::vector<cmExpandedCommandArgument> const& args, std::string& errorString,
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
  using handlerFn_t = bool (cmConditionEvaluator::*)(
    cmArgumentList&, std::string&, MessageType&);
  std::array<handlerFn_t, 5> const handlers = { {
    &cmConditionEvaluator::HandleLevel0, // parenthesis
    &cmConditionEvaluator::HandleLevel1, // predicates
    &cmConditionEvaluator::HandleLevel2, // binary ops
    &cmConditionEvaluator::HandleLevel3, // NOT
    &cmConditionEvaluator::HandleLevel4  // AND OR
  } };
  for (auto fn : handlers) {
    // Call the reducer 'till there is anything to reduce...
    // (i.e., if after an iteration the size becomes smaller)
    auto levelResult = true;
    for (auto beginSize = newArgs.size();
         (levelResult = (this->*fn)(newArgs, errorString, status)) &&
         newArgs.size() < beginSize;
         beginSize = newArgs.size()) {
    }

    if (!levelResult) {
      // NOTE `errorString` supposed to be set already
      return false;
    }
  }

  // now at the end there should only be one argument left
  if (newArgs.size() != 1) {
    errorString = "Unknown arguments specified";
    status = MessageType::FATAL_ERROR;
    return false;
  }

  return this->GetBooleanValue(newArgs.front());
}

//=========================================================================
cmValue cmConditionEvaluator::GetDefinitionIfUnquoted(
  cmExpandedCommandArgument const& argument) const
{
  if (argument.WasQuoted()) {
    return nullptr;
  }

  return this->Makefile.GetDefinition(argument.GetValue());
}

//=========================================================================
cmValue cmConditionEvaluator::GetVariableOrString(
  cmExpandedCommandArgument const& argument) const
{
  cmValue def = this->GetDefinitionIfUnquoted(argument);

  if (!def) {
    def = cmValue(argument.GetValue());
  }

  return def;
}

//=========================================================================
bool cmConditionEvaluator::IsKeyword(
  cm::static_string_view keyword,
  cmExpandedCommandArgument const& argument) const
{
  if (argument.WasQuoted()) {
    return false;
  }

  return argument.GetValue() == keyword;
}

//=========================================================================
bool cmConditionEvaluator::GetBooleanValue(
  cmExpandedCommandArgument& arg) const
{
  // Check basic and named constants.
  if (cmIsOn(arg.GetValue())) {
    return true;
  }
  if (cmIsOff(arg.GetValue())) {
    return false;
  }

  // Check for numbers.
  if (!arg.empty()) {
    char* end;
    double const d = std::strtod(arg.GetValue().c_str(), &end);
    if (*end == '\0') {
      // The whole string is a number.  Use C conversion to bool.
      return static_cast<bool>(d);
    }
  }

  // Check definition.
  cmValue def = this->GetDefinitionIfUnquoted(arg);
  return !def.IsOff();
}

template <int N>
inline int cmConditionEvaluator::matchKeysImpl(
  cmExpandedCommandArgument const&)
{
  // Zero means "not found"
  return 0;
}

template <int N, typename T, typename... Keys>
inline int cmConditionEvaluator::matchKeysImpl(
  cmExpandedCommandArgument const& arg, T current, Keys... key)
{
  if (this->IsKeyword(current, arg)) {
    // Stop searching as soon as smth has found
    return N;
  }
  return matchKeysImpl<N + 1>(arg, key...);
}

template <typename... Keys>
inline int cmConditionEvaluator::matchKeys(
  cmExpandedCommandArgument const& arg, Keys... key)
{
  // Get index of the matched key (1-based)
  return matchKeysImpl<1>(arg, key...);
}

//=========================================================================
// level 0 processes parenthetical expressions
bool cmConditionEvaluator::HandleLevel0(cmArgumentList& newArgs,
                                        std::string& errorString,
                                        MessageType& status)
{
  for (auto arg = newArgs.begin(); arg != newArgs.end(); ++arg) {
    if (this->IsKeyword(keyParenL, *arg)) {
      // search for the closing paren for this opening one
      auto depth = 1;
      auto argClose = std::next(arg);
      for (; argClose != newArgs.end() && depth; ++argClose) {
        depth += int(this->IsKeyword(keyParenL, *argClose)) -
          int(this->IsKeyword(keyParenR, *argClose));
      }
      if (depth) {
        errorString = "mismatched parenthesis in condition";
        status = MessageType::FATAL_ERROR;
        return false;
      }

      // store the reduced args in this vector
      auto argOpen = std::next(arg);
      std::vector<cmExpandedCommandArgument> const subExpr(
        argOpen, std::prev(argClose));

      // now recursively invoke IsTrue to handle the values inside the
      // parenthetical expression
      auto const value = this->IsTrue(subExpr, errorString, status);
      *arg = cmExpandedCommandArgument(bool2string(value), true);
      argOpen = std::next(arg);
      // remove the now evaluated parenthetical expression
      newArgs.erase(argOpen, argClose);
    }
  }
  return true;
}

//=========================================================================
// level one handles most predicates except for NOT
bool cmConditionEvaluator::HandleLevel1(cmArgumentList& newArgs, std::string&,
                                        MessageType&)
{
  for (auto args = newArgs.make2ArgsIterator(); args.current != newArgs.end();
       args.advance(newArgs)) {
    // NOTE Fail fast: All the predicates below require the next arg to be
    // valid
    if (args.next == newArgs.end()) {
      continue;
    }

    // does a file exist
    if (this->IsKeyword(keyEXISTS, *args.current)) {
      newArgs.ReduceOneArg(cmSystemTools::FileExists(args.next->GetValue()),
                           args);
    }
    // check if a file is readable
    else if (this->IsKeyword(keyIS_READABLE, *args.current)) {
      newArgs.ReduceOneArg(cmSystemTools::TestFileAccess(
                             args.next->GetValue(), cmsys::TEST_FILE_READ),
                           args);
    }
    // check if a file is writable
    else if (this->IsKeyword(keyIS_WRITABLE, *args.current)) {
      newArgs.ReduceOneArg(cmSystemTools::TestFileAccess(
                             args.next->GetValue(), cmsys::TEST_FILE_WRITE),
                           args);
    }
    // check if a file is executable
    else if (this->IsKeyword(keyIS_EXECUTABLE, *args.current)) {
      newArgs.ReduceOneArg(cmSystemTools::TestFileAccess(
                             args.next->GetValue(), cmsys::TEST_FILE_EXECUTE),
                           args);
    }
    // does a directory with this name exist
    else if (this->IsKeyword(keyIS_DIRECTORY, *args.current)) {
      newArgs.ReduceOneArg(
        cmSystemTools::FileIsDirectory(args.next->GetValue()), args);
    }
    // does a symlink with this name exist
    else if (this->IsKeyword(keyIS_SYMLINK, *args.current)) {
      newArgs.ReduceOneArg(cmSystemTools::FileIsSymlink(args.next->GetValue()),
                           args);
    }
    // is the given path an absolute path ?
    else if (this->IsKeyword(keyIS_ABSOLUTE, *args.current)) {
      newArgs.ReduceOneArg(
        cmSystemTools::FileIsFullPath(args.next->GetValue()), args);
    }
    // does a command exist
    else if (this->IsKeyword(keyCOMMAND, *args.current)) {
      newArgs.ReduceOneArg(
        static_cast<bool>(
          this->Makefile.GetState()->GetCommand(args.next->GetValue())),
        args);
    }
    // does a policy exist
    else if (this->IsKeyword(keyPOLICY, *args.current)) {
      cmPolicies::PolicyID pid;
      newArgs.ReduceOneArg(
        cmPolicies::GetPolicyID(args.next->GetValue().c_str(), pid), args);
    }
    // does a target exist
    else if (this->IsKeyword(keyTARGET, *args.current)) {
      newArgs.ReduceOneArg(static_cast<bool>(this->Makefile.FindTargetToUse(
                             args.next->GetValue())),
                           args);
    }
    // is a variable defined
    else if (this->IsKeyword(keyDEFINED, *args.current)) {
      auto const& var = args.next->GetValue();
      auto const varNameLen = var.size();

      auto result = false;
      if (looksLikeSpecialVariable(var, "ENV"_s, varNameLen)) {
        auto const env = args.next->GetValue().substr(4, varNameLen - 5);
        result = cmSystemTools::HasEnv(env);
      }

      else if (looksLikeSpecialVariable(var, "CACHE"_s, varNameLen)) {
        auto const cache = args.next->GetValue().substr(6, varNameLen - 7);
        result = static_cast<bool>(
          this->Makefile.GetState()->GetCacheEntryValue(cache));
      }

      else {
        result = this->Makefile.IsDefinitionSet(args.next->GetValue());
      }
      newArgs.ReduceOneArg(result, args);
    }
    // does a test exist
    else if (this->IsKeyword(keyTEST, *args.current)) {
      newArgs.ReduceOneArg(
        static_cast<bool>(this->Makefile.GetTest(args.next->GetValue())),
        args);
    }
  }
  return true;
}

//=========================================================================
// level two handles most binary operations except for AND  OR
bool cmConditionEvaluator::HandleLevel2(cmArgumentList& newArgs,
                                        std::string& errorString,
                                        MessageType& status)
{
  for (auto args = newArgs.make3ArgsIterator(); args.current != newArgs.end();
       args.advance(newArgs)) {

    int matchNo;

    // NOTE Handle special case `if(... BLAH_BLAH MATCHES)`
    // (i.e., w/o regex to match which is possibly result of
    // variable expansion to an empty string)
    if (args.next != newArgs.end() &&
        this->IsKeyword(keyMATCHES, *args.current)) {
      newArgs.ReduceOneArg(false, args);
    }

    // NOTE Fail fast: All the binary ops below require 2 arguments.
    else if (args.next == newArgs.end() || args.nextnext == newArgs.end()) {
      continue;
    }

    else if (this->IsKeyword(keyMATCHES, *args.next)) {
      cmValue def = this->GetDefinitionIfUnquoted(*args.current);

      std::string def_buf;
      if (!def) {
        def = cmValue(args.current->GetValue());
      } else if (cmHasLiteralPrefix(args.current->GetValue(),
                                    "CMAKE_MATCH_")) {
        // The string to match is owned by our match result variables.
        // Move it to our own buffer before clearing them.
        def_buf = *def;
        def = cmValue(def_buf);
      }

      this->Makefile.ClearMatches();

      auto const& rex = args.nextnext->GetValue();
      cmsys::RegularExpression regEntry;
      if (!regEntry.compile(rex)) {
        std::ostringstream error;
        error << "Regular expression \"" << rex << "\" cannot compile";
        errorString = error.str();
        status = MessageType::FATAL_ERROR;
        return false;
      }

      auto const match = regEntry.find(*def);
      if (match) {
        this->Makefile.StoreMatches(regEntry);
      }
      newArgs.ReduceTwoArgs(match, args);
    }

    else if ((matchNo =
                this->matchKeys(*args.next, keyLESS, keyLESS_EQUAL, keyGREATER,
                                keyGREATER_EQUAL, keyEQUAL))) {

      cmValue ldef = this->GetVariableOrString(*args.current);
      cmValue rdef = this->GetVariableOrString(*args.nextnext);

      double lhs;
      double rhs;
      auto parseDoubles = [&]() {
        return std::sscanf(ldef->c_str(), "%lg", &lhs) == 1 &&
          std::sscanf(rdef->c_str(), "%lg", &rhs) == 1;
      };
      // clang-format off
      const auto result = parseDoubles() &&
        cmRt2CtSelector<
            std::less, std::less_equal,
            std::greater, std::greater_equal,
            std::equal_to
          >::eval(matchNo, lhs, rhs);
      // clang-format on
      newArgs.ReduceTwoArgs(result, args);
    }

    else if ((matchNo = this->matchKeys(*args.next, keySTRLESS,
                                        keySTRLESS_EQUAL, keySTRGREATER,
                                        keySTRGREATER_EQUAL, keySTREQUAL))) {

      cmValue const lhs = this->GetVariableOrString(*args.current);
      cmValue const rhs = this->GetVariableOrString(*args.nextnext);
      auto const val = (*lhs).compare(*rhs);
      // clang-format off
      const auto result = cmRt2CtSelector<
            std::less, std::less_equal,
            std::greater, std::greater_equal,
            std::equal_to
          >::eval(matchNo, val, 0);
      // clang-format on
      newArgs.ReduceTwoArgs(result, args);
    }

    else if ((matchNo =
                this->matchKeys(*args.next, keyVERSION_LESS,
                                keyVERSION_LESS_EQUAL, keyVERSION_GREATER,
                                keyVERSION_GREATER_EQUAL, keyVERSION_EQUAL))) {
      auto const op = MATCH2CMPOP[matchNo - 1];
      cmValue const lhs = this->GetVariableOrString(*args.current);
      cmValue const rhs = this->GetVariableOrString(*args.nextnext);
      auto const result = cmSystemTools::VersionCompare(op, lhs, rhs);
      newArgs.ReduceTwoArgs(result, args);
    }

    // is file A newer than file B
    else if (this->IsKeyword(keyIS_NEWER_THAN, *args.next)) {
      auto fileIsNewer = 0;
      cmsys::Status ftcStatus = cmSystemTools::FileTimeCompare(
        args.current->GetValue(), args.nextnext->GetValue(), &fileIsNewer);
      newArgs.ReduceTwoArgs(
        (!ftcStatus || fileIsNewer == 1 || fileIsNewer == 0), args);
    }

    else if (this->IsKeyword(keyIN_LIST, *args.next)) {
      cmValue lhs = this->GetVariableOrString(*args.current);
      cmValue rhs = this->Makefile.GetDefinition(args.nextnext->GetValue());

      newArgs.ReduceTwoArgs(
        rhs && cm::contains(cmList{ *rhs, cmList::EmptyElements::Yes }, *lhs),
        args);
    }

    else if (this->IsKeyword(keyPATH_EQUAL, *args.next)) {

      if (this->Policy139Status != cmPolicies::OLD &&
          this->Policy139Status != cmPolicies::WARN) {

        cmValue lhs = this->GetVariableOrString(*args.current);
        cmValue rhs = this->GetVariableOrString(*args.nextnext);
        auto const result = cmCMakePath{ *lhs } == cmCMakePath{ *rhs };
        newArgs.ReduceTwoArgs(result, args);
      }

      else if (this->Policy139Status == cmPolicies::WARN) {
        std::ostringstream e;
        e << cmPolicies::GetPolicyWarning(cmPolicies::CMP0139)
          << "\n"
             "PATH_EQUAL will be interpreted as an operator "
             "when the policy is set to NEW.  "
             "Since the policy is not set the OLD behavior will be used.";

        this->Makefile.IssueMessage(MessageType::AUTHOR_WARNING, e.str());
      }
    }
  }
  return true;
}

//=========================================================================
// level 3 handles NOT
bool cmConditionEvaluator::HandleLevel3(cmArgumentList& newArgs, std::string&,
                                        MessageType&)
{
  for (auto args = newArgs.make2ArgsIterator(); args.next != newArgs.end();
       args.advance(newArgs)) {
    if (this->IsKeyword(keyNOT, *args.current)) {
      auto const rhs = this->GetBooleanValue(*args.next);
      newArgs.ReduceOneArg(!rhs, args);
    }
  }
  return true;
}

//=========================================================================
// level 4 handles AND OR
bool cmConditionEvaluator::HandleLevel4(cmArgumentList& newArgs, std::string&,
                                        MessageType&)
{
  for (auto args = newArgs.make3ArgsIterator(); args.nextnext != newArgs.end();
       args.advance(newArgs)) {

    int matchNo;

    if ((matchNo = this->matchKeys(*args.next, keyAND, keyOR))) {
      auto const lhs = this->GetBooleanValue(*args.current);
      auto const rhs = this->GetBooleanValue(*args.nextnext);
      // clang-format off
      const auto result =
        cmRt2CtSelector<
            std::logical_and, std::logical_or
          >::eval(matchNo, lhs, rhs);
      // clang-format on
      newArgs.ReduceTwoArgs(result, args);
    }
  }
  return true;
}
