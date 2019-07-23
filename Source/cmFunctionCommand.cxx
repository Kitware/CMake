/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmFunctionCommand.h"

#include <sstream>
#include <utility>

#include "cmAlgorithms.h"
#include "cmExecutionStatus.h"
#include "cmFunctionBlocker.h"
#include "cmListFileCache.h"
#include "cmMakefile.h"
#include "cmPolicies.h"
#include "cmRange.h"
#include "cmState.h"
#include "cmStringAlgorithms.h"

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
    std::string errorMsg =
      "Function invoked with incorrect arguments for function named: ";
    errorMsg += this->Args[0];
    inStatus.SetError(errorMsg);
    return false;
  }

  cmMakefile::FunctionPushPop functionScope(&makefile, this->FilePath,
                                            this->Policies);

  // set the value of argc
  std::ostringstream strStream;
  strStream << expandedArgs.size();
  makefile.AddDefinition("ARGC", strStream.str());
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
  std::vector<std::string>::const_iterator eit =
    expandedArgs.begin() + (this->Args.size() - 1);
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
  bool IsFunctionBlocked(const cmListFileFunction&, cmMakefile& mf,
                         cmExecutionStatus&) override;
  bool ShouldRemove(const cmListFileFunction&, cmMakefile& mf) override;
  bool Replay(std::vector<cmListFileFunction> const& functions,
              cmExecutionStatus& status);

  std::vector<std::string> Args;
  std::vector<cmListFileFunction> Functions;
  int Depth = 0;
};

bool cmFunctionFunctionBlocker::IsFunctionBlocked(
  const cmListFileFunction& lff, cmMakefile& mf, cmExecutionStatus& status)
{
  // record commands until we hit the ENDFUNCTION
  // at the ENDFUNCTION call we shift gears and start looking for invocations
  if (lff.Name.Lower == "function") {
    this->Depth++;
  } else if (lff.Name.Lower == "endfunction") {
    // if this is the endfunction for this function then execute
    if (!this->Depth) {
      auto self = mf.RemoveFunctionBlocker(this, lff);
      return this->Replay(this->Functions, status);
    }
    // decrement for each nested function that ends
    this->Depth--;
  }

  // if it wasn't an endfunction and we are not executing then we must be
  // recording
  this->Functions.push_back(lff);
  return true;
}

bool cmFunctionFunctionBlocker::ShouldRemove(const cmListFileFunction& lff,
                                             cmMakefile& mf)
{
  if (lff.Name.Lower == "endfunction") {
    std::vector<std::string> expandedArguments;
    mf.ExpandArguments(lff.Arguments, expandedArguments,
                       this->GetStartingContext().FilePath.c_str());
    // if the endfunction has arguments then make sure
    // they match the ones in the opening function command
    if ((expandedArguments.empty() ||
         (expandedArguments[0] == this->Args[0]))) {
      return true;
    }
  }

  return false;
}

bool cmFunctionFunctionBlocker::Replay(
  std::vector<cmListFileFunction> const& functions, cmExecutionStatus& status)
{
  cmMakefile& mf = status.GetMakefile();
  // create a new command and add it to cmake
  cmFunctionHelperCommand f;
  f.Args = this->Args;
  f.Functions = functions;
  f.FilePath = this->GetStartingContext().FilePath;
  mf.RecordPolicies(f.Policies);
  mf.GetState()->AddScriptedCommand(this->Args[0], std::move(f));
  return true;
}

bool cmFunctionCommand::InitialPass(std::vector<std::string> const& args,
                                    cmExecutionStatus&)
{
  if (args.empty()) {
    this->SetError("called with incorrect number of arguments");
    return false;
  }

  // create a function blocker
  {
    auto fb = cm::make_unique<cmFunctionFunctionBlocker>();
    cmAppend(fb->Args, args);
    this->Makefile->AddFunctionBlocker(std::move(fb));
  }
  return true;
}
