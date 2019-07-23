/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmWhileCommand.h"

#include "cm_memory.hxx"

#include "cmConditionEvaluator.h"
#include "cmExecutionStatus.h"
#include "cmExpandedCommandArgument.h"
#include "cmFunctionBlocker.h"
#include "cmListFileCache.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmSystemTools.h"

#include <string>
#include <utility>

class cmWhileFunctionBlocker : public cmFunctionBlocker
{
public:
  cmWhileFunctionBlocker(cmMakefile* mf);
  ~cmWhileFunctionBlocker() override;
  bool IsFunctionBlocked(const cmListFileFunction& lff, cmMakefile& mf,
                         cmExecutionStatus&) override;
  bool ShouldRemove(const cmListFileFunction& lff, cmMakefile& mf) override;
  bool Replay(std::vector<cmListFileFunction> const& functions,
              cmExecutionStatus& inStatus);

  std::vector<cmListFileArgument> Args;
  std::vector<cmListFileFunction> Functions;

private:
  cmMakefile* Makefile;
  int Depth;
};

cmWhileFunctionBlocker::cmWhileFunctionBlocker(cmMakefile* mf)
  : Makefile(mf)
  , Depth(0)
{
  this->Makefile->PushLoopBlock();
}

cmWhileFunctionBlocker::~cmWhileFunctionBlocker()
{
  this->Makefile->PopLoopBlock();
}

bool cmWhileFunctionBlocker::IsFunctionBlocked(const cmListFileFunction& lff,
                                               cmMakefile& mf,
                                               cmExecutionStatus& inStatus)
{
  // at end of for each execute recorded commands
  if (lff.Name.Lower == "while") {
    // record the number of while commands past this one
    this->Depth++;
  } else if (lff.Name.Lower == "endwhile") {
    // if this is the endwhile for this while loop then execute
    if (!this->Depth) {
      // Remove the function blocker for this scope or bail.
      std::unique_ptr<cmFunctionBlocker> fb(
        mf.RemoveFunctionBlocker(this, lff));
      if (!fb) {
        return false;
      }
      return this->Replay(this->Functions, inStatus);
    }
    // decrement for each nested while that ends
    this->Depth--;
  }

  // record the command
  this->Functions.push_back(lff);

  // always return true
  return true;
}

bool cmWhileFunctionBlocker::ShouldRemove(const cmListFileFunction& lff,
                                          cmMakefile&)
{
  if (lff.Name.Lower == "endwhile") {
    // if the endwhile has arguments, then make sure
    // they match the arguments of the matching while
    if (lff.Arguments.empty() || lff.Arguments == this->Args) {
      return true;
    }
  }
  return false;
}

bool cmWhileFunctionBlocker::Replay(
  std::vector<cmListFileFunction> const& functions,
  cmExecutionStatus& inStatus)
{
  cmMakefile& mf = inStatus.GetMakefile();
  std::string errorString;

  std::vector<cmExpandedCommandArgument> expandedArguments;
  mf.ExpandArguments(this->Args, expandedArguments);
  MessageType messageType;

  cmListFileContext execContext = this->GetStartingContext();

  cmCommandContext commandContext;
  commandContext.Line = execContext.Line;
  commandContext.Name = execContext.Name;

  cmConditionEvaluator conditionEvaluator(mf, this->GetStartingContext(),
                                          mf.GetBacktrace(commandContext));

  bool isTrue =
    conditionEvaluator.IsTrue(expandedArguments, errorString, messageType);

  while (isTrue) {
    if (!errorString.empty()) {
      std::string err = "had incorrect arguments: ";
      for (cmListFileArgument const& arg : this->Args) {
        err += (arg.Delim ? "\"" : "");
        err += arg.Value;
        err += (arg.Delim ? "\"" : "");
        err += " ";
      }
      err += "(";
      err += errorString;
      err += ").";
      mf.IssueMessage(messageType, err);
      if (messageType == MessageType::FATAL_ERROR) {
        cmSystemTools::SetFatalErrorOccured();
        return true;
      }
    }

    // Invoke all the functions that were collected in the block.
    for (cmListFileFunction const& fn : functions) {
      cmExecutionStatus status(mf);
      mf.ExecuteCommand(fn, status);
      if (status.GetReturnInvoked()) {
        inStatus.SetReturnInvoked();
        return true;
      }
      if (status.GetBreakInvoked()) {
        return true;
      }
      if (status.GetContinueInvoked()) {
        break;
      }
      if (cmSystemTools::GetFatalErrorOccured()) {
        return true;
      }
    }
    expandedArguments.clear();
    mf.ExpandArguments(this->Args, expandedArguments);
    isTrue =
      conditionEvaluator.IsTrue(expandedArguments, errorString, messageType);
  }
  return true;
}

bool cmWhileCommand(std::vector<cmListFileArgument> const& args,
                    cmExecutionStatus& status)
{
  if (args.empty()) {
    status.SetError("called with incorrect number of arguments");
    return false;
  }

  // create a function blocker
  {
    cmMakefile& makefile = status.GetMakefile();
    auto fb = cm::make_unique<cmWhileFunctionBlocker>(&makefile);
    fb->Args = args;
    makefile.AddFunctionBlocker(std::move(fb));
  }
  return true;
}
