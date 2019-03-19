/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmIfCommand.h"

#include "cmConditionEvaluator.h"
#include "cmExecutionStatus.h"
#include "cmExpandedCommandArgument.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmOutputConverter.h"
#include "cmSystemTools.h"
#include "cmake.h"

#include <memory> // IWYU pragma: keep

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

//=========================================================================
bool cmIfFunctionBlocker::IsFunctionBlocked(const cmListFileFunction& lff,
                                            cmMakefile& mf,
                                            cmExecutionStatus& inStatus)
{
  // we start by recording all the functions
  if (lff.Name.Lower == "if") {
    this->ScopeDepth++;
  } else if (lff.Name.Lower == "endif") {
    this->ScopeDepth--;
    // if this is the endif for this if statement, then start executing
    if (!this->ScopeDepth) {
      // Remove the function blocker for this scope or bail.
      std::unique_ptr<cmFunctionBlocker> fb(
        mf.RemoveFunctionBlocker(this, lff));
      if (!fb) {
        return false;
      }

      // execute the functions for the true parts of the if statement
      cmExecutionStatus status;
      int scopeDepth = 0;
      for (cmListFileFunction const& func : this->Functions) {
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

            bool isTrue = conditionEvaluator.IsTrue(expandedArguments,
                                                    errorString, messType);

            if (!errorString.empty()) {
              std::string err = cmIfCommandError(expandedArguments);
              err += errorString;
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
          status.Clear();
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
  }

  // record the command
  this->Functions.push_back(lff);

  // always return true
  return true;
}

//=========================================================================
bool cmIfFunctionBlocker::ShouldRemove(const cmListFileFunction& lff,
                                       cmMakefile&)
{
  if (lff.Name.Lower == "endif") {
    // if the endif has arguments, then make sure
    // they match the arguments of the matching if
    if (lff.Arguments.empty() || lff.Arguments == this->Args) {
      return true;
    }
  }

  return false;
}

//=========================================================================
bool cmIfCommand::InvokeInitialPass(
  const std::vector<cmListFileArgument>& args, cmExecutionStatus&)
{
  std::string errorString;

  std::vector<cmExpandedCommandArgument> expandedArguments;
  this->Makefile->ExpandArguments(args, expandedArguments);

  MessageType status;

  cmConditionEvaluator conditionEvaluator(
    *(this->Makefile), this->Makefile->GetExecutionContext(),
    this->Makefile->GetBacktrace());

  bool isTrue =
    conditionEvaluator.IsTrue(expandedArguments, errorString, status);

  if (!errorString.empty()) {
    std::string err = "if " + cmIfCommandError(expandedArguments);
    err += errorString;
    if (status == MessageType::FATAL_ERROR) {
      this->Makefile->IssueMessage(MessageType::FATAL_ERROR, err);
      cmSystemTools::SetFatalErrorOccured();
      return true;
    }
    this->Makefile->IssueMessage(status, err);
  }

  cmIfFunctionBlocker* f = new cmIfFunctionBlocker();
  // if is isn't true block the commands
  f->ScopeDepth = 1;
  f->IsBlocking = !isTrue;
  if (isTrue) {
    f->HasRun = true;
  }
  f->Args = args;
  this->Makefile->AddFunctionBlocker(f);

  return true;
}
