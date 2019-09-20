/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmFunctionCommand.h"

#include <sstream>
#include <utility>

#include <cm/memory>
#include <cm/string_view>

#include "cm_static_string_view.hxx"

#include "cmAlgorithms.h"
#include "cmExecutionStatus.h"
#include "cmFunctionBlocker.h"
#include "cmListFileCache.h"
#include "cmMakefile.h"
#include "cmPolicies.h"
#include "cmRange.h"
#include "cmState.h"
#include "cmStringAlgorithms.h"

namespace {
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
};
}

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
    std::string errorMsg = cmStrCat(
      "Function invoked with incorrect arguments for function named: ",
      this->Args[0]);
    inStatus.SetError(errorMsg);
    return false;
  }

  cmMakefile::FunctionPushPop functionScope(&makefile, this->FilePath,
                                            this->Policies);

  // set the value of argc
  makefile.AddDefinition("ARGC", std::to_string(expandedArgs.size()));
  makefile.MarkVariableAsUsed("ARGC");

  // set the values for ARGV0 ARGV1 ...
  for (unsigned int t = 0; t < expandedArgs.size(); ++t) {
    std::ostringstream tmpStream;
    tmpStream << "ARGV" << t;
    makefile.AddDefinition(tmpStream.str(), expandedArgs[t]);
    makefile.MarkVariableAsUsed(tmpStream.str());
  }

  // define the formal arguments
  for (unsigned int j = 1; j < this->Args.size(); ++j) {
    makefile.AddDefinition(this->Args[j], expandedArgs[j - 1]);
  }

  // define ARGV and ARGN
  std::string argvDef = cmJoin(expandedArgs, ";");
  auto eit = expandedArgs.begin() + (this->Args.size() - 1);
  std::string argnDef = cmJoin(cmMakeRange(eit, expandedArgs.end()), ";");
  makefile.AddDefinition("ARGV", argvDef);
  makefile.MarkVariableAsUsed("ARGV");
  makefile.AddDefinition("ARGN", argnDef);
  makefile.MarkVariableAsUsed("ARGN");

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
      return true;
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
  mf.ExpandArguments(lff.Arguments, expandedArguments,
                     this->GetStartingContext().FilePath.c_str());
  return expandedArguments.empty() || expandedArguments[0] == this->Args[0];
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
  mf.RecordPolicies(f.Policies);
  mf.GetState()->AddScriptedCommand(this->Args[0], std::move(f));
  return true;
}

bool cmFunctionCommand(std::vector<std::string> const& args,
                       cmExecutionStatus& status)
{
  if (args.empty()) {
    status.SetError("called with incorrect number of arguments");
    return false;
  }

  // create a function blocker
  {
    auto fb = cm::make_unique<cmFunctionFunctionBlocker>();
    cmAppend(fb->Args, args);
    status.GetMakefile().AddFunctionBlocker(std::move(fb));
  }
  return true;
}
