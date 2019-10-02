/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmWhileCommand.h"

#include <string>
#include <utility>

#include <cm/memory>
#include <cm/string_view>

#include "cm_static_string_view.hxx"

#include "cmConditionEvaluator.h"
#include "cmExecutionStatus.h"
#include "cmExpandedCommandArgument.h"
#include "cmFunctionBlocker.h"
#include "cmListFileCache.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmSystemTools.h"

class cmWhileFunctionBlocker : public cmFunctionBlocker
{
public:
  cmWhileFunctionBlocker(cmMakefile* mf);
  ~cmWhileFunctionBlocker() override;

  cm::string_view StartCommandName() const override { return "while"_s; }
  cm::string_view EndCommandName() const override { return "endwhile"_s; }

  bool ArgumentsMatch(cmListFileFunction const& lff,
                      cmMakefile& mf) const override;

  bool Replay(std::vector<cmListFileFunction> functions,
              cmExecutionStatus& inStatus) override;

  std::vector<cmListFileArgument> Args;

private:
  cmMakefile* Makefile;
};

cmWhileFunctionBlocker::cmWhileFunctionBlocker(cmMakefile* mf)
  : Makefile(mf)
{
  this->Makefile->PushLoopBlock();
}

cmWhileFunctionBlocker::~cmWhileFunctionBlocker()
{
  this->Makefile->PopLoopBlock();
}

bool cmWhileFunctionBlocker::ArgumentsMatch(cmListFileFunction const& lff,
                                            cmMakefile&) const
{
  return lff.Arguments.empty() || lff.Arguments == this->Args;
}

bool cmWhileFunctionBlocker::Replay(std::vector<cmListFileFunction> functions,
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
