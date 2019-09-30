/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmIfCommand.h"

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
  return lff.Arguments.empty() || lff.Arguments == this->Args;
}

bool cmIfFunctionBlocker::Replay(std::vector<cmListFileFunction> functions,
                                 cmExecutionStatus& inStatus)
{
  cmMakefile& mf = inStatus.GetMakefile();
  // execute the functions for the true parts of the if statement
  int scopeDepth = 0;
  for (cmListFileFunction const& func : functions) {
    // keep track of scope depth
    if (func.Name.Lower == "if") {
      scopeDepth++;
    }
    if (func.Name.Lower == "endif") {
      scopeDepth--;
    }
    // watch for our state change
    if (scopeDepth == 0 && func.Name.Lower == "else") {

      if (this->ElseSeen) {
        cmListFileBacktrace bt = mf.GetBacktrace(func);
        mf.GetCMakeInstance()->IssueMessage(
          MessageType::FATAL_ERROR,
          "A duplicate ELSE command was found inside an IF block.", bt);
        cmSystemTools::SetFatalErrorOccured();
        return true;
      }

      this->IsBlocking = this->HasRun;
      this->HasRun = true;
      this->ElseSeen = true;

      // if trace is enabled, print a (trivially) evaluated "else"
      // statement
      if (!this->IsBlocking && mf.GetCMakeInstance()->GetTrace()) {
        mf.PrintCommandTrace(func);
      }
    } else if (scopeDepth == 0 && func.Name.Lower == "elseif") {
      if (this->ElseSeen) {
        cmListFileBacktrace bt = mf.GetBacktrace(func);
        mf.GetCMakeInstance()->IssueMessage(
          MessageType::FATAL_ERROR,
          "An ELSEIF command was found after an ELSE command.", bt);
        cmSystemTools::SetFatalErrorOccured();
        return true;
      }

      if (this->HasRun) {
        this->IsBlocking = true;
      } else {
        // if trace is enabled, print the evaluated "elseif" statement
        if (mf.GetCMakeInstance()->GetTrace()) {
          mf.PrintCommandTrace(func);
        }

        std::string errorString;

        std::vector<cmExpandedCommandArgument> expandedArguments;
        mf.ExpandArguments(func.Arguments, expandedArguments);

        MessageType messType;

        cmListFileContext conditionContext =
          cmListFileContext::FromCommandContext(
            func, this->GetStartingContext().FilePath);

        cmConditionEvaluator conditionEvaluator(mf, conditionContext,
                                                mf.GetBacktrace(func));

        bool isTrue =
          conditionEvaluator.IsTrue(expandedArguments, errorString, messType);

        if (!errorString.empty()) {
          std::string err =
            cmStrCat(cmIfCommandError(expandedArguments), errorString);
          cmListFileBacktrace bt = mf.GetBacktrace(func);
          mf.GetCMakeInstance()->IssueMessage(messType, err, bt);
          if (messType == MessageType::FATAL_ERROR) {
            cmSystemTools::SetFatalErrorOccured();
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
        inStatus.SetReturnInvoked();
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

  cmConditionEvaluator conditionEvaluator(
    makefile, makefile.GetExecutionContext(), makefile.GetBacktrace());

  bool isTrue =
    conditionEvaluator.IsTrue(expandedArguments, errorString, status);

  if (!errorString.empty()) {
    std::string err =
      cmStrCat("if ", cmIfCommandError(expandedArguments), errorString);
    if (status == MessageType::FATAL_ERROR) {
      makefile.IssueMessage(MessageType::FATAL_ERROR, err);
      cmSystemTools::SetFatalErrorOccured();
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
