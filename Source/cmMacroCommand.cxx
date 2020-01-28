/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmMacroCommand.h"

#include <cstdio>
#include <utility>

#include <cm/memory>
#include <cm/string_view>
#include <cmext/algorithm>

#include "cm_static_string_view.hxx"

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

// define the class for macro commands
class cmMacroHelperCommand
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

bool cmMacroHelperCommand::operator()(
  std::vector<cmListFileArgument> const& args,
  cmExecutionStatus& inStatus) const
{
  cmMakefile& makefile = inStatus.GetMakefile();

  // Expand the argument list to the macro.
  std::vector<std::string> expandedArgs;
  makefile.ExpandArguments(args, expandedArgs);

  // make sure the number of arguments passed is at least the number
  // required by the signature
  if (expandedArgs.size() < this->Args.size() - 1) {
    std::string errorMsg =
      cmStrCat("Macro invoked with incorrect arguments for macro named: ",
               this->Args[0]);
    inStatus.SetError(errorMsg);
    return false;
  }

  cmMakefile::MacroPushPop macroScope(&makefile, this->FilePath,
                                      this->Policies);

  // set the value of argc
  std::string argcDef = std::to_string(expandedArgs.size());

  auto eit = expandedArgs.begin() + (this->Args.size() - 1);
  std::string expandedArgn = cmJoin(cmMakeRange(eit, expandedArgs.end()), ";");
  std::string expandedArgv = cmJoin(expandedArgs, ";");
  std::vector<std::string> variables;
  variables.reserve(this->Args.size() - 1);
  for (unsigned int j = 1; j < this->Args.size(); ++j) {
    variables.push_back("${" + this->Args[j] + "}");
  }
  std::vector<std::string> argVs;
  argVs.reserve(expandedArgs.size());
  char argvName[60];
  for (unsigned int j = 0; j < expandedArgs.size(); ++j) {
    sprintf(argvName, "${ARGV%u}", j);
    argVs.emplace_back(argvName);
  }
  // Invoke all the functions that were collected in the block.
  cmListFileFunction newLFF;
  // for each function
  for (cmListFileFunction const& func : this->Functions) {
    // Replace the formal arguments and then invoke the command.
    newLFF.Arguments.clear();
    newLFF.Arguments.reserve(func.Arguments.size());
    newLFF.Name = func.Name;
    newLFF.Line = func.Line;

    // for each argument of the current function
    for (cmListFileArgument const& k : func.Arguments) {
      cmListFileArgument arg;
      arg.Value = k.Value;
      if (k.Delim != cmListFileArgument::Bracket) {
        // replace formal arguments
        for (unsigned int j = 0; j < variables.size(); ++j) {
          cmSystemTools::ReplaceString(arg.Value, variables[j],
                                       expandedArgs[j]);
        }
        // replace argc
        cmSystemTools::ReplaceString(arg.Value, "${ARGC}", argcDef);

        cmSystemTools::ReplaceString(arg.Value, "${ARGN}", expandedArgn);
        cmSystemTools::ReplaceString(arg.Value, "${ARGV}", expandedArgv);

        // if the current argument of the current function has ${ARGV in it
        // then try replacing ARGV values
        if (arg.Value.find("${ARGV") != std::string::npos) {
          for (unsigned int t = 0; t < expandedArgs.size(); ++t) {
            cmSystemTools::ReplaceString(arg.Value, argVs[t], expandedArgs[t]);
          }
        }
      }
      arg.Delim = k.Delim;
      arg.Line = k.Line;
      newLFF.Arguments.push_back(std::move(arg));
    }
    cmExecutionStatus status(makefile);
    if (!makefile.ExecuteCommand(newLFF, status) || status.GetNestedError()) {
      // The error message should have already included the call stack
      // so we do not need to report an error here.
      macroScope.Quiet();
      inStatus.SetNestedError();
      return false;
    }
    if (status.GetReturnInvoked()) {
      inStatus.SetReturnInvoked();
      return true;
    }
    if (status.GetBreakInvoked()) {
      inStatus.SetBreakInvoked();
      return true;
    }
  }
  return true;
}

class cmMacroFunctionBlocker : public cmFunctionBlocker
{
public:
  cm::string_view StartCommandName() const override { return "macro"_s; }
  cm::string_view EndCommandName() const override { return "endmacro"_s; }

  bool ArgumentsMatch(cmListFileFunction const&,
                      cmMakefile& mf) const override;

  bool Replay(std::vector<cmListFileFunction> functions,
              cmExecutionStatus& status) override;

  std::vector<std::string> Args;
};

bool cmMacroFunctionBlocker::ArgumentsMatch(cmListFileFunction const& lff,
                                            cmMakefile& mf) const
{
  std::vector<std::string> expandedArguments;
  mf.ExpandArguments(lff.Arguments, expandedArguments,
                     this->GetStartingContext().FilePath.c_str());
  return expandedArguments.empty() || expandedArguments[0] == this->Args[0];
}

bool cmMacroFunctionBlocker::Replay(std::vector<cmListFileFunction> functions,
                                    cmExecutionStatus& status)
{
  cmMakefile& mf = status.GetMakefile();
  mf.AppendProperty("MACROS", this->Args[0]);
  // create a new command and add it to cmake
  cmMacroHelperCommand f;
  f.Args = this->Args;
  f.Functions = std::move(functions);
  f.FilePath = this->GetStartingContext().FilePath;
  mf.RecordPolicies(f.Policies);
  mf.GetState()->AddScriptedCommand(this->Args[0], std::move(f));
  return true;
}
}

bool cmMacroCommand(std::vector<std::string> const& args,
                    cmExecutionStatus& status)
{
  if (args.empty()) {
    status.SetError("called with incorrect number of arguments");
    return false;
  }

  // create a function blocker
  {
    auto fb = cm::make_unique<cmMacroFunctionBlocker>();
    cm::append(fb->Args, args);
    status.GetMakefile().AddFunctionBlocker(std::move(fb));
  }
  return true;
}
