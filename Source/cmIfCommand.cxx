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
      mf.GetStateSnapshot().UnmarkNotExecuted(
        functionBlocks[c].ConditionBegin);
      mf.GetStateSnapshot().UnmarkNotExecuted(functionBlocks[c].BlockBegin);
      Execute(functionBlocks[c].Functions, mf, inStatus);
      return;
    } else {
      // if trace is enabled, print the evaluated "elseif" statement
      if (mf.GetCMakeInstance()->GetTrace()) {
        //        mf.PrintCommandTrace(functions[c]);
      }
      mf.GetStateSnapshot().UnmarkNotExecuted(
        functionBlocks[c].ConditionBegin);

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
        mf.GetStateSnapshot().UnmarkNotExecuted(functionBlocks[c].BlockBegin);
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
      mf.GetStateSnapshot().MarkNotExecuted(
        this->FunctionBlocks.back().BlockBegin, lff.Line);
      this->FunctionBlocks.back().BlockEnd = lff.Line;

      // Remove the function blocker for this scope or bail.
      cmsys::auto_ptr<cmFunctionBlocker> fb(
        mf.RemoveFunctionBlocker(this, lff));
      if (!fb.get()) {
        return false;
      }

      if (!this->IsBlocking) {
        mf.GetStateSnapshot().UnmarkNotExecuted(
          this->FunctionBlocks.front().BlockBegin);
        Execute(this->FunctionBlocks.front().Functions, mf, inStatus);
        return true;
      }

      ScopedBit(this, this->FunctionBlocks, mf, inStatus);
      return true;
    }
  } else if (this->ScopeDepth == 1 &&
             !cmSystemTools::Strucmp(lff.Name.c_str(), "elseif")) {
    mf.GetStateSnapshot().MarkNotExecuted(
      this->FunctionBlocks.back().BlockBegin, lff.Line);
    this->FunctionBlocks.back().BlockEnd = lff.Line;
    cmListFileFunctionBlock block;
    block.Condition = lff.Arguments;
    block.ConditionBegin = lff.Line;
    block.BlockBegin = lff.CloseParenLine + 1;
    block.Backtrace = mf.GetBacktrace(lff);

    mf.GetStateSnapshot().MarkNotExecuted(block.ConditionBegin,
                                          block.BlockBegin);

    block.CommCon = lff;

    this->FunctionBlocks.push_back(block);
    addIt = false;
  } else if (this->ScopeDepth == 1 &&
             !cmSystemTools::Strucmp(lff.Name.c_str(), "else")) {
    mf.GetStateSnapshot().MarkNotExecuted(
      this->FunctionBlocks.back().BlockBegin, lff.Line);
    this->FunctionBlocks.back().BlockEnd = lff.Line;
    cmListFileFunctionBlock block;
    block.ConditionBegin = lff.Line;
    block.BlockBegin = lff.CloseParenLine + 1;

    mf.GetStateSnapshot().MarkNotExecuted(block.ConditionBegin,
                                          block.BlockBegin);

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

cmCommand::ParameterContext cmIfCommand::GetContextForParameter(
  std::vector<std::string> const& args, size_t index)
{
  if (index == 0) {
    if (args.empty() || (args.size() > 0 && (args[index] == "TARGET" ||
                                             args[index] == "POLICY"))) {
      return KeywordParameter;
    }
  }
  if (index == 1 && args[0] == "TARGET") {
    return SingleBinaryTargetParameter;
  }
  if (index == 1 && args[0] == "POLICY") {
    return PolicyParameter;
  }

  //  if (args.size() <= index)
  return NoContext;

  //  // Need context for <constant> ?
  //  size_t exprIndex = 0;
  //  size_t exprSize = 1; // Compute this

  //  if (args[index] == "(" || args[index] == ")")
  //    {
  //    return NoContext; // TODO: Might be
  //    KeywordOrVariableOrConstantParameter
  //    // set exprSize = 0;
  //    }

  //  for ( ; index != 0 && args[index - 1] != "("; --index)
  //    {
  //    ++exprIndex;
  //    }

  //  // In the end, exprIndex is 0, 1, or 2

  //  switch(exprSize)
  //    {
  //    case 0:
  //    case 1:
  //      return KeywordOrVariableOrConstantParameter;
  //    case 2: {
  //      switch(exprIndex)
  //      {
  //      case 0:
  //        return KeywordParameter;
  //      case 1:
  //        {
  //        if (args[0] == "NOT")
  //          return KeywordOrVariableOrConstantParameter;
  //        if (args[0] == "COMMAND")
  //          return CommandNameParameter;
  //        if (args[0] == "POLICY")
  //          return PolicyParameter;
  //        if (args[0] == "TARGET")
  //          return SingleTargetParameter;
  //        if (args[0] == "EXISTS")
  //          return FilePathParameter;
  //        if (args[0] == "IS_DIRECTORY")
  //          return FilePathParameter;
  //        if (args[0] == "IS_SYMLINK")
  //          return FilePathParameter;
  //        if (args[0] == "IS_ABSOLUTE")
  //          return FilePathParameter;
  //        if (args[0] == "DEFINED")
  //          return VariableIdentifierParameter;
  //        return NoContext; // Unreachable?
  //        }
  //      }
  //    }
  //    case 3:
  //      {
  //      switch(exprIndex)
  //        {
  //        case 0:
  //          // Could be NOT or a variable or constant
  //          return KeywordOrVariableOrConstantParameter;
  //          // Bah, humbug - do completion ltr only! Or?
  //        case 1:
  //          {
  //          if (args[0] == "NOT")
  //            return KeywordParameter;
  //          return KeywordParameter; // Binary keyword
  //          }
  //        case 2: {
  //          if (args[0] == "NOT")
  //            {
  //            if (args[1] == "COMMAND")
  //              return CommandNameParameter;
  //            if (args[1] == "POLICY")
  //              return PolicyParameter;
  //            if (args[1] == "TARGET")
  //              return SingleTargetParameter;
  //            if (args[1] == "EXISTS")
  //              return FilePathParameter;
  //            if (args[1] == "IS_DIRECTORY")
  //              return FilePathParameter;
  //            if (args[1] == "IS_SYMLINK")
  //              return FilePathParameter;
  //            if (args[1] == "IS_ABSOLUTE")
  //              return FilePathParameter;
  //            if (args[1] == "DEFINED")
  //              return VariableIdentifierParameter;
  //            return NoContext; // Unreachable?
  //            }
  //          if (args[1] == "AND" || args[1] == "OR")
  //            return VariableIdentifierParameter;
  //          if (args[1] == "IS_NEWER_THAN")
  //              return FilePathParameter;
  //          if (args[1] == "MATCHES")
  //              return VariableIdentifierParameter; // ConstantParameter;
  //          if (args[1] == "LESS")
  //              return NumberParameter;
  //          if (args[1] == "GREATER")
  //              return NumberParameter;
  //          if (args[1] == "EQUAL")
  //              return NumberParameter;
  //          if (args[1] == "STRLESS")
  //              return NumberParameter; // ?
  //          if (args[1] == "STRGREATER")
  //              return NumberParameter; // ?
  //          if (args[1] == "STREQUAL")
  //              return NumberParameter; // ?
  //          if (args[1] == "VERSION_LESS")
  //              return VersionParameter;
  //          if (args[1] == "VERSION_GREATER")
  //              return VersionParameter;
  //          if (args[1] == "VERSION_EQUAL")
  //              return VersionParameter;
  //          return VariableIdentifierParameter;
  //          }
  //        }
  //      }
  //    }
  return NoContext;
}

std::vector<std::string> cmIfCommand::GetKeywords(
  std::vector<std::string> const& args, size_t index)
{
  size_t exprIndex = 0;
  size_t exprSize = 1; // Compute this
  for (; index != 0 && args[index - 1] != "("; --index) {
    ++exprIndex;
  }
  std::vector<std::string> keywords;
  if (exprSize <= 1) {
    keywords.push_back("NOT");
    keywords.push_back("COMMAND");
    keywords.push_back("POLICY");
    keywords.push_back("TARGET");
    keywords.push_back("EXISTS");
    keywords.push_back("IS_DIRECTORY");
    keywords.push_back("IS_SYMLINK");
    keywords.push_back("IS_ABSOLUTE");
    keywords.push_back("DEFINED");
  } else if (exprSize == 2) {
    switch (exprIndex) {
      case 0:
        keywords.push_back("NOT");
        break;
      case 1:
        keywords.push_back("COMMAND");
        keywords.push_back("POLICY");
        keywords.push_back("TARGET");
        keywords.push_back("EXISTS");
        keywords.push_back("IS_DIRECTORY");
        keywords.push_back("IS_SYMLINK");
        keywords.push_back("IS_ABSOLUTE");
        keywords.push_back("DEFINED");
    }
  } else if (exprSize == 3) {
    switch (exprIndex) {
      case 0:
        keywords.push_back("NOT");
        break;
      case 1: {
        if (args[index - 1] == "NOT") {
          keywords.push_back("COMMAND");
          keywords.push_back("POLICY");
          keywords.push_back("TARGET");
          keywords.push_back("EXISTS");
          keywords.push_back("IS_DIRECTORY");
          keywords.push_back("IS_SYMLINK");
          keywords.push_back("IS_ABSOLUTE");
          keywords.push_back("DEFINED");
        }
        break;
      }
      case 2: {
        assert(args[index - 2] != "NOT"); // no keywords here.
                                          //           {
                                          //
                                          //           }
        //         return VariableIdentifierParameter; // ?
      }
    }
  }
  return keywords;
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
  block.ConditionBegin = lfc.Line;
  block.BlockBegin = lfc.CloseParenLine + 1;
  block.CommCon.Line = lfc.Line;
  block.CommCon.Name = lfc.Name;

  f->FunctionBlocks.push_back(block);

  this->Makefile->AddFunctionBlocker(f);

  return true;
}
