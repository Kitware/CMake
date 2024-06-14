/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmIfCommand.h"

#include <string>
#include <utility>

#include <cm/memory>
#include <cm/string_view>
#include <cmext/string_view>

#include "cmConditionEvaluator.h"
#include "cmExecutionStatus.h"
#include "cmExpandedCommandArgument.h"
#include "cmFunctionBlocker.h"
#include "cmListFileCache.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmOutputConverter.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmake.h"

static std::string cmIfCommandError(
  std::vector<cmExpandedCommandArgument> const& args)
{
  std::string err = "given arguments:\n ";
  for (cmExpandedCommandArgument const& i : args) {
    err += " ";
    err += cmOutputConverter::EscapeForCMake(i.GetValue());
  }
  err += "\n";
  return err;
}

class cmIfFunctionBlocker : public cmFunctionBlocker
{
public:
  cm::string_view StartCommandName() const override { return "if"_s; }
  cm::string_view EndCommandName() const override { return "endif"_s; }

  bool ArgumentsMatch(cmListFileFunction const& lff,
                      cmMakefile&) const override;

  bool Replay(std::vector<cmListFileFunction> functions,
              cmExecutionStatus& inStatus) override;

  std::vector<cmListFileArgument> Args;
  bool IsBlocking;
  bool HasRun = false;
  bool ElseSeen = false;
};

bool cmIfFunctionBlocker::ArgumentsMatch(cmListFileFunction const& lff,
                                         cmMakefile&) const
{
  return lff.Arguments().empty() || lff.Arguments() == this->Args;
}

bool cmIfFunctionBlocker::Replay(std::vector<cmListFileFunction> functions,
                                 cmExecutionStatus& inStatus)
{
  cmMakefile& mf = inStatus.GetMakefile();
  // execute the functions for the true parts of the if statement
  int scopeDepth = 0;
  for (cmListFileFunction const& func : functions) {
    // keep track of scope depth
    if (func.LowerCaseName() == "if") {
      scopeDepth++;
    }
    if (func.LowerCaseName() == "endif") {
      scopeDepth--;
    }
    // watch for our state change
    if (scopeDepth == 0 && func.LowerCaseName() == "else") {
      cmListFileBacktrace elseBT = mf.GetBacktrace().Push(
        cmListFileContext{ func.OriginalName(),
                           this->GetStartingContext().FilePath, func.Line() });

      if (this->ElseSeen) {
        mf.GetCMakeInstance()->IssueMessage(
          MessageType::FATAL_ERROR,
          "A duplicate ELSE command was found inside an IF block.", elseBT);
        cmSystemTools::SetFatalErrorOccurred();
        return true;
      }

      this->IsBlocking = this->HasRun;
      this->HasRun = true;
      this->ElseSeen = true;

      // if trace is enabled, print a (trivially) evaluated "else"
      // statement
      if (!this->IsBlocking && mf.GetCMakeInstance()->GetTrace()) {
        mf.PrintCommandTrace(func, elseBT,
                             cmMakefile::CommandMissingFromStack::Yes);
      }
    } else if (scopeDepth == 0 && func.LowerCaseName() == "elseif") {
      cmListFileBacktrace elseifBT = mf.GetBacktrace().Push(
        cmListFileContext{ func.OriginalName(),
                           this->GetStartingContext().FilePath, func.Line() });
      if (this->ElseSeen) {
        mf.GetCMakeInstance()->IssueMessage(
          MessageType::FATAL_ERROR,
          "An ELSEIF command was found after an ELSE command.", elseifBT);
        cmSystemTools::SetFatalErrorOccurred();
        return true;
      }

      if (this->HasRun) {
        this->IsBlocking = true;
      } else {
        // if trace is enabled, print the evaluated "elseif" statement
        if (mf.GetCMakeInstance()->GetTrace()) {
          mf.PrintCommandTrace(func, elseifBT,
                               cmMakefile::CommandMissingFromStack::Yes);
        }

        std::string errorString;

        std::vector<cmExpandedCommandArgument> expandedArguments;
        mf.ExpandArguments(func.Arguments(), expandedArguments);

        MessageType messType;

        cmConditionEvaluator conditionEvaluator(mf, elseifBT);

        bool isTrue =
          conditionEvaluator.IsTrue(expandedArguments, errorString, messType);

        if (!errorString.empty()) {
          std::string err =
            cmStrCat(cmIfCommandError(expandedArguments), errorString);
          mf.GetCMakeInstance()->IssueMessage(messType, err, elseifBT);
          if (messType == MessageType::FATAL_ERROR) {
            cmSystemTools::SetFatalErrorOccurred();
            return true;
          }
        }

        if (isTrue) {
          this->IsBlocking = false;
          this->HasRun = true;
        }
      }
    }

    // should we execute?
    else if (!this->IsBlocking) {
      cmExecutionStatus status(mf);
      mf.ExecuteCommand(func, status);
      if (status.GetReturnInvoked()) {
        inStatus.SetReturnInvoked(status.GetReturnVariables());
        return true;
      }
      if (status.GetBreakInvoked()) {
        inStatus.SetBreakInvoked();
        return true;
      }
      if (status.GetContinueInvoked()) {
        inStatus.SetContinueInvoked();
        return true;
      }
      if (status.HasExitCode()) {
        inStatus.SetExitCode(status.GetExitCode());
        return true;
      }
    }
  }
  return true;
}

//=========================================================================
bool cmIfCommand(std::vector<cmListFileArgument> const& args,
                 cmExecutionStatus& inStatus)
{
  cmMakefile& makefile = inStatus.GetMakefile();
  std::string errorString;

  std::vector<cmExpandedCommandArgument> expandedArguments;
  makefile.ExpandArguments(args, expandedArguments);

  MessageType status;

  cmConditionEvaluator conditionEvaluator(makefile, makefile.GetBacktrace());

  bool isTrue =
    conditionEvaluator.IsTrue(expandedArguments, errorString, status);

  if (!errorString.empty()) {
    std::string err =
      cmStrCat("if ", cmIfCommandError(expandedArguments), errorString);
    if (status == MessageType::FATAL_ERROR) {
      makefile.IssueMessage(MessageType::FATAL_ERROR, err);
      cmSystemTools::SetFatalErrorOccurred();
      return true;
    }
    makefile.IssueMessage(status, err);
  }

  {
    auto fb = cm::make_unique<cmIfFunctionBlocker>();
    // if is isn't true block the commands
    fb->IsBlocking = !isTrue;
    if (isTrue) {
      fb->HasRun = true;
    }
    fb->Args = args;
    makefile.AddFunctionBlocker(std::move(fb));
  }

  return true;
}
