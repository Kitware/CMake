/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmFunctionCommand.h"

#include <utility>

#include <cm/memory>
#include <cm/string_view>
#include <cmext/algorithm>
#include <cmext/string_view>

#include "cmExecutionStatus.h"
#include "cmFunctionBlocker.h"
#include "cmListFileCache.h"
#include "cmMakefile.h"
#include "cmPolicies.h"
#include "cmRange.h"
#include "cmState.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"

namespace {
std::string const ARGC = "ARGC";
std::string const ARGN = "ARGN";
std::string const ARGV = "ARGV";
std::string const CMAKE_CURRENT_FUNCTION = "CMAKE_CURRENT_FUNCTION";
std::string const CMAKE_CURRENT_FUNCTION_LIST_FILE =
  "CMAKE_CURRENT_FUNCTION_LIST_FILE";
std::string const CMAKE_CURRENT_FUNCTION_LIST_DIR =
  "CMAKE_CURRENT_FUNCTION_LIST_DIR";
std::string const CMAKE_CURRENT_FUNCTION_LIST_LINE =
  "CMAKE_CURRENT_FUNCTION_LIST_LINE";

// define the class for function commands
class cmFunctionHelperCommand
{
public:
  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  bool operator()(std::vector<cmListFileArgument> const& args,
                  cmExecutionStatus& inStatus) const;

  std::vector<std::string> Args;
  std::vector<cmListFileFunction> Functions;
  cmPolicies::PolicyMap Policies;
  std::string FilePath;
  long Line;
};

bool cmFunctionHelperCommand::operator()(
  std::vector<cmListFileArgument> const& args,
  cmExecutionStatus& inStatus) const
{
  cmMakefile& makefile = inStatus.GetMakefile();

  // Expand the argument list to the function.
  std::vector<std::string> expandedArgs;
  makefile.ExpandArguments(args, expandedArgs);

  // make sure the number of arguments passed is at least the number
  // required by the signature
  if (expandedArgs.size() < this->Args.size() - 1) {
    auto const errorMsg = cmStrCat(
      "Function invoked with incorrect arguments for function named: ",
      this->Args.front());
    inStatus.SetError(errorMsg);
    return false;
  }

  cmMakefile::FunctionPushPop functionScope(&makefile, this->FilePath,
                                            this->Policies);

  // set the value of argc
  makefile.AddDefinition(ARGC, std::to_string(expandedArgs.size()));
  makefile.MarkVariableAsUsed(ARGC);

  // set the values for ARGV0 ARGV1 ...
  for (auto t = 0u; t < expandedArgs.size(); ++t) {
    auto const value = cmStrCat(ARGV, std::to_string(t));
    makefile.AddDefinition(value, expandedArgs[t]);
    makefile.MarkVariableAsUsed(value);
  }

  // define the formal arguments
  for (auto j = 1u; j < this->Args.size(); ++j) {
    makefile.AddDefinition(this->Args[j], expandedArgs[j - 1]);
  }

  // define ARGV and ARGN
  auto const argvDef = cmJoin(expandedArgs, ";");
  auto const eit = expandedArgs.begin() + (this->Args.size() - 1);
  auto const argnDef = cmJoin(cmMakeRange(eit, expandedArgs.end()), ";");
  makefile.AddDefinition(ARGV, argvDef);
  makefile.MarkVariableAsUsed(ARGV);
  makefile.AddDefinition(ARGN, argnDef);
  makefile.MarkVariableAsUsed(ARGN);

  makefile.AddDefinition(CMAKE_CURRENT_FUNCTION, this->Args.front());
  makefile.MarkVariableAsUsed(CMAKE_CURRENT_FUNCTION);
  makefile.AddDefinition(CMAKE_CURRENT_FUNCTION_LIST_FILE, this->FilePath);
  makefile.MarkVariableAsUsed(CMAKE_CURRENT_FUNCTION_LIST_FILE);
  makefile.AddDefinition(CMAKE_CURRENT_FUNCTION_LIST_DIR,
                         cmSystemTools::GetFilenamePath(this->FilePath));
  makefile.MarkVariableAsUsed(CMAKE_CURRENT_FUNCTION_LIST_DIR);
  makefile.AddDefinition(CMAKE_CURRENT_FUNCTION_LIST_LINE,
                         std::to_string(this->Line));
  makefile.MarkVariableAsUsed(CMAKE_CURRENT_FUNCTION_LIST_LINE);

  // Invoke all the functions that were collected in the block.
  // for each function
  for (cmListFileFunction const& func : this->Functions) {
    cmExecutionStatus status(makefile);
    if (!makefile.ExecuteCommand(func, status) || status.GetNestedError()) {
      // The error message should have already included the call stack
      // so we do not need to report an error here.
      functionScope.Quiet();
      inStatus.SetNestedError();
      return false;
    }
    if (status.GetReturnInvoked()) {
      makefile.RaiseScope(status.GetReturnVariables());
      break;
    }
  }

  // pop scope on the makefile
  return true;
}

class cmFunctionFunctionBlocker : public cmFunctionBlocker
{
public:
  cm::string_view StartCommandName() const override { return "function"_s; }
  cm::string_view EndCommandName() const override { return "endfunction"_s; }

  bool ArgumentsMatch(cmListFileFunction const&,
                      cmMakefile& mf) const override;

  bool Replay(std::vector<cmListFileFunction> functions,
              cmExecutionStatus& status) override;

  std::vector<std::string> Args;
};

bool cmFunctionFunctionBlocker::ArgumentsMatch(cmListFileFunction const& lff,
                                               cmMakefile& mf) const
{
  std::vector<std::string> expandedArguments;
  mf.ExpandArguments(lff.Arguments(), expandedArguments);
  return expandedArguments.empty() ||
    expandedArguments.front() == this->Args.front();
}

bool cmFunctionFunctionBlocker::Replay(
  std::vector<cmListFileFunction> functions, cmExecutionStatus& status)
{
  cmMakefile& mf = status.GetMakefile();
  // create a new command and add it to cmake
  cmFunctionHelperCommand f;
  f.Args = this->Args;
  f.Functions = std::move(functions);
  f.FilePath = this->GetStartingContext().FilePath;
  f.Line = this->GetStartingContext().Line;
  mf.RecordPolicies(f.Policies);
  return mf.GetState()->AddScriptedCommand(
    this->Args.front(),
    BT<cmState::Command>(std::move(f),
                         mf.GetBacktrace().Push(this->GetStartingContext())),
    mf);
}

} // anonymous namespace

bool cmFunctionCommand(std::vector<std::string> const& args,
                       cmExecutionStatus& status)
{
  if (args.empty()) {
    status.SetError("called with incorrect number of arguments");
    return false;
  }

  // create a function blocker
  auto fb = cm::make_unique<cmFunctionFunctionBlocker>();
  cm::append(fb->Args, args);
  status.GetMakefile().AddFunctionBlocker(std::move(fb));

  return true;
}
