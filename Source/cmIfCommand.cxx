/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmIfCommand.h"

#include "cmOutputConverter.h"
#include "cmStringCommand.h"

#include "cmConditionEvaluator.h"

#include <cmsys/RegularExpression.hxx>
#include <list>
#include <stdlib.h> // required for atof

static std::string cmIfCommandError(
  std::vector<cmExpandedCommandArgument> const& args)
{
  std::string err = "given arguments:\n ";
  for (std::vector<cmExpandedCommandArgument>::const_iterator i = args.begin();
       i != args.end(); ++i) {
    err += " ";
    err += cmOutputConverter::EscapeForCMake(i->GetValue());
  }
  err += "\n";
  return err;
}

void Execute(std::vector<cmListFileFunction> functions, cmMakefile& mf,
             cmExecutionStatus& inStatus)
{
  cmExecutionStatus status;
  for (unsigned int c = 0; c < functions.size(); ++c) {
    status.Clear();
    mf.ExecuteCommand(functions[c], status);
    if (status.GetReturnInvoked()) {
      inStatus.SetReturnInvoked(true);
      return;
    }
    if (status.GetBreakInvoked()) {
      inStatus.SetBreakInvoked(true);
      return;
    }
    if (status.GetContinueInvoked()) {
      inStatus.SetContinueInvoked(true);
      return;
    }
  }
}

void ScopedBit(cmIfFunctionBlocker* that,
               std::vector<cmListFileFunctionBlock> functionBlocks,
               cmMakefile& mf, cmExecutionStatus& inStatus)
{
  for (unsigned int c = 1; c < functionBlocks.size(); ++c) {
    if (functionBlocks[c].Condition.empty()) {
      if (mf.GetCMakeInstance()->GetTrace()) {
        // if trace is enabled, print a (trivially) evaluated "else"
        // statement
        //        mf.PrintCommandTrace(functions[c]);
      }
      Execute(functionBlocks[c].Functions, mf, inStatus);
      return;
    } else {
      // if trace is enabled, print the evaluated "elseif" statement
      if (mf.GetCMakeInstance()->GetTrace()) {
        //        mf.PrintCommandTrace(functions[c]);
      }

      std::string errorString;

      std::vector<cmExpandedCommandArgument> expandedArguments2;
      mf.ExpandArguments(functionBlocks[c].Condition, expandedArguments2);

      cmake::MessageType messType2;

      cmListFileContext conditionContext2 =
        cmListFileContext::FromCommandContext(
          functionBlocks[c].CommCon, that->GetStartingContext().FilePath);

      cmConditionEvaluator conditionEvaluator2(mf, conditionContext2,
                                               functionBlocks[c].Backtrace);

      bool isTrue2 =
        conditionEvaluator2.IsTrue(expandedArguments2, errorString, messType2);

      bool isTrue = isTrue2;

      if (!errorString.empty()) {
        std::string err = cmIfCommandError(expandedArguments2);
        err += errorString;
        cmListFileBacktrace bt = functionBlocks[c].Backtrace;
        mf.GetCMakeInstance()->IssueMessage(messType2, err, bt);
        if (messType2 == cmake::FATAL_ERROR) {
          cmSystemTools::SetFatalErrorOccured();
          return;
        }
      }

      if (isTrue) {
        Execute(functionBlocks[c].Functions, mf, inStatus);
        return;
      }
    }
  }
}

//=========================================================================
bool cmIfFunctionBlocker::IsFunctionBlocked(const cmListFileFunction& lff,
                                            cmMakefile& mf,
                                            cmExecutionStatus& inStatus)
{
  // we start by recording all the functions
  bool addIt = true;
  if (!cmSystemTools::Strucmp(lff.Name.c_str(), "if")) {
    this->ScopeDepth++;
  } else if (!cmSystemTools::Strucmp(lff.Name.c_str(), "endif")) {
    this->ScopeDepth--;
    // if this is the endif for this if statement, then start executing
    if (!this->ScopeDepth) {
      // Remove the function blocker for this scope or bail.
      cmsys::auto_ptr<cmFunctionBlocker> fb(
        mf.RemoveFunctionBlocker(this, lff));
      if (!fb.get()) {
        return false;
      }

      if (!this->IsBlocking) {
        Execute(this->FunctionBlocks.front().Functions, mf, inStatus);
        return true;
      }

      ScopedBit(this, this->FunctionBlocks, mf, inStatus);
      return true;
    }
  } else if (this->ScopeDepth == 1 &&
             !cmSystemTools::Strucmp(lff.Name.c_str(), "elseif")) {
    cmListFileFunctionBlock block;
    block.Condition = lff.Arguments;
    block.Backtrace = mf.GetBacktrace(lff);

    block.CommCon = lff;

    this->FunctionBlocks.push_back(block);
    addIt = false;
  } else if (this->ScopeDepth == 1 &&
             !cmSystemTools::Strucmp(lff.Name.c_str(), "else")) {
    cmListFileFunctionBlock block;
    this->FunctionBlocks.push_back(block);
    addIt = false;
  }

  // record the command
  if (addIt) {
    this->FunctionBlocks.back().Functions.push_back(lff);
  }

  // always return true
  return true;
}

//=========================================================================
bool cmIfFunctionBlocker::ShouldRemove(const cmListFileFunction& lff,
                                       cmMakefile&)
{
  if (!cmSystemTools::Strucmp(lff.Name.c_str(), "endif")) {
    // if the endif has arguments, then make sure
    // they match the arguments of the matching if
    if (lff.Arguments.empty() ||
        lff.Arguments == this->FunctionBlocks.front().Condition) {
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

  cmake::MessageType status;

  cmConditionEvaluator conditionEvaluator(
    *(this->Makefile), this->Makefile->GetExecutionContext(),
    this->Makefile->GetBacktrace());

  bool isTrue =
    conditionEvaluator.IsTrue(expandedArguments, errorString, status);

  if (!errorString.empty()) {
    std::string err = cmIfCommandError(expandedArguments);
    err += errorString;
    if (status == cmake::FATAL_ERROR) {
      this->SetError(err);
      cmSystemTools::SetFatalErrorOccured();
      return false;
    } else {
      this->Makefile->IssueMessage(status, err);
    }
  }

  cmIfFunctionBlocker* f = new cmIfFunctionBlocker();
  // if is isn't true block the commands
  f->ScopeDepth = 1;
  f->IsBlocking = !isTrue;

  cmListFileFunctionBlock block;
  block.Condition = args;
  auto lfc = this->Makefile->GetExecutionContext();
  block.CommCon.Line = lfc.Line;
  block.CommCon.Name = lfc.Name;

  f->FunctionBlocks.push_back(block);

  this->Makefile->AddFunctionBlocker(f);

  return true;
}
