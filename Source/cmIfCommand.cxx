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
#include "cmStringCommand.h"

#include <stdlib.h> // required for atof
#include <list>
#include <cmsys/RegularExpression.hxx>


static std::string cmIfCommandError(
  cmMakefile* mf, std::vector<std::string> const& args)
{
  cmLocalGenerator* lg = mf->GetLocalGenerator();
  std::string err = "given arguments:\n ";
  for(std::vector<std::string>::const_iterator i = args.begin();
      i != args.end(); ++i)
    {
    err += " ";
    err += lg->EscapeForCMake(i->c_str());
    }
  err += "\n";
  return err;
}

//=========================================================================
bool cmIfFunctionBlocker::
IsFunctionBlocked(const cmListFileFunction& lff,
                  cmMakefile &mf,
                  cmExecutionStatus &inStatus)
{
  // we start by recording all the functions
  if (!cmSystemTools::Strucmp(lff.Name.c_str(),"if"))
    {
    this->ScopeDepth++;
    }
  if (!cmSystemTools::Strucmp(lff.Name.c_str(),"endif"))
    {
    this->ScopeDepth--;
    // if this is the endif for this if statement, then start executing
    if (!this->ScopeDepth)
      {
      // Remove the function blocker for this scope or bail.
      cmsys::auto_ptr<cmFunctionBlocker>
        fb(mf.RemoveFunctionBlocker(this, lff));
      if(!fb.get()) { return false; }

      // execute the functions for the true parts of the if statement
      cmExecutionStatus status;
      int scopeDepth = 0;
      for(unsigned int c = 0; c < this->Functions.size(); ++c)
        {
        // keep track of scope depth
        if (!cmSystemTools::Strucmp(this->Functions[c].Name.c_str(),"if"))
          {
          scopeDepth++;
          }
        if (!cmSystemTools::Strucmp(this->Functions[c].Name.c_str(),"endif"))
          {
          scopeDepth--;
          }
        // watch for our state change
        if (scopeDepth == 0 &&
            !cmSystemTools::Strucmp(this->Functions[c].Name.c_str(),"else"))
          {
          this->IsBlocking = this->HasRun;
          this->HasRun = true;

          // if trace is enabled, print a (trivially) evaluated "else"
          // statement
          if(!this->IsBlocking && mf.GetCMakeInstance()->GetTrace())
            {
            mf.PrintCommandTrace(this->Functions[c]);
            }
          }
        else if (scopeDepth == 0 && !cmSystemTools::Strucmp
                 (this->Functions[c].Name.c_str(),"elseif"))
          {
          if (this->HasRun)
            {
            this->IsBlocking = true;
            }
          else
            {
            // Place this call on the call stack.
            cmMakefileCall stack_manager(&mf, this->Functions[c], status);
            static_cast<void>(stack_manager);

            // if trace is enabled, print the evaluated "elseif" statement
            if(mf.GetCMakeInstance()->GetTrace())
              {
              mf.PrintCommandTrace(this->Functions[c]);
              }

            std::string errorString;

            std::vector<std::string> expandedArguments;
            mf.ExpandArguments(this->Functions[c].Arguments,
                               expandedArguments);

            cmake::MessageType messType;
            bool isTrue =
              cmIfCommand::IsTrue(expandedArguments, errorString,
                                  &mf, messType);

            if (errorString.size())
              {
              std::string err = cmIfCommandError(&mf, expandedArguments);
              err += errorString;
              mf.IssueMessage(messType, err);
              if (messType == cmake::FATAL_ERROR)
                {
                cmSystemTools::SetFatalErrorOccured();
                return true;
                }
              }

            if (isTrue)
              {
              this->IsBlocking = false;
              this->HasRun = true;
              }
            }
          }

        // should we execute?
        else if (!this->IsBlocking)
          {
          status.Clear();
          mf.ExecuteCommand(this->Functions[c],status);
          if (status.GetReturnInvoked())
            {
            inStatus.SetReturnInvoked(true);
            return true;
            }
          if (status.GetBreakInvoked())
            {
            inStatus.SetBreakInvoked(true);
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
  if (!cmSystemTools::Strucmp(lff.Name.c_str(),"endif"))
    {
    // if the endif has arguments, then make sure
    // they match the arguments of the matching if
    if (lff.Arguments.size() == 0 ||
        lff.Arguments == this->Args)
      {
      return true;
      }
    }

  return false;
}

//=========================================================================
bool cmIfCommand
::InvokeInitialPass(const std::vector<cmListFileArgument>& args,
                    cmExecutionStatus &)
{
  std::string errorString;

  std::vector<std::string> expandedArguments;
  this->Makefile->ExpandArguments(args, expandedArguments);

  cmake::MessageType status;
  bool isTrue =
    cmIfCommand::IsTrue(expandedArguments,errorString,
                        this->Makefile, status);

  if (errorString.size())
    {
    std::string err = cmIfCommandError(this->Makefile, expandedArguments);
    err += errorString;
    if (status == cmake::FATAL_ERROR)
      {
      this->SetError(err.c_str());
      cmSystemTools::SetFatalErrorOccured();
      return false;
      }
    else
      {
      this->Makefile->IssueMessage(status, err);
      }
    }

  cmIfFunctionBlocker *f = new cmIfFunctionBlocker();
  // if is isn't true block the commands
  f->ScopeDepth = 1;
  f->IsBlocking = !isTrue;
  if (isTrue)
    {
    f->HasRun = true;
    }
  f->Args = args;
  this->Makefile->AddFunctionBlocker(f);

  return true;
}

namespace
{
  //=========================================================================
  bool GetBooleanValue(std::string& arg, cmMakefile* mf)
  {
  // Check basic constants.
  if (arg == "0")
    {
    return false;
    }
  if (arg == "1")
    {
    return true;
    }

  // Check named constants.
  if (cmSystemTools::IsOn(arg.c_str()))
    {
    return true;
    }
  if (cmSystemTools::IsOff(arg.c_str()))
    {
    return false;
    }

  // Check for numbers.
  if(!arg.empty())
    {
    char* end;
    double d = strtod(arg.c_str(), &end);
    if(*end == '\0')
      {
      // The whole string is a number.  Use C conversion to bool.
      return d? true:false;
      }
    }

  // Check definition.
  const char* def = mf->GetDefinition(arg.c_str());
  return !cmSystemTools::IsOff(def);
  }

  //=========================================================================
  // Boolean value behavior from CMake 2.6.4 and below.
  bool GetBooleanValueOld(std::string const& arg, cmMakefile* mf, bool one)
  {
  if(one)
    {
    // Old IsTrue behavior for single argument.
    if(arg == "0")
      { return false; }
    else if(arg == "1")
      { return true; }
    else
      { return !cmSystemTools::IsOff(mf->GetDefinition(arg.c_str())); }
    }
  else
    {
    // Old GetVariableOrNumber behavior.
    const char* def = mf->GetDefinition(arg.c_str());
    if(!def && atoi(arg.c_str()))
      {
      def = arg.c_str();
      }
    return !cmSystemTools::IsOff(def);
    }
  }

  //=========================================================================
  // returns the resulting boolean value
  bool GetBooleanValueWithAutoDereference(
    std::string &newArg,
    cmMakefile *makefile,
    std::string &errorString,
    cmPolicies::PolicyStatus Policy12Status,
    cmake::MessageType &status,
    bool oneArg = false)
  {
  // Use the policy if it is set.
  if (Policy12Status == cmPolicies::NEW)
    {
    return GetBooleanValue(newArg, makefile);
    }
  else if (Policy12Status == cmPolicies::OLD)
    {
    return GetBooleanValueOld(newArg, makefile, oneArg);
    }

  // Check policy only if old and new results differ.
  bool newResult = GetBooleanValue(newArg, makefile);
  bool oldResult = GetBooleanValueOld(newArg, makefile, oneArg);
  if(newResult != oldResult)
    {
    switch(Policy12Status)
      {
      case cmPolicies::WARN:
        {
        cmPolicies* policies = makefile->GetPolicies();
        errorString = "An argument named \"" + newArg
          + "\" appears in a conditional statement.  "
          + policies->GetPolicyWarning(cmPolicies::CMP0012);
        status = cmake::AUTHOR_WARNING;
        }
      case cmPolicies::OLD:
        return oldResult;
      case cmPolicies::REQUIRED_IF_USED:
      case cmPolicies::REQUIRED_ALWAYS:
        {
        cmPolicies* policies = makefile->GetPolicies();
        errorString = "An argument named \"" + newArg
          + "\" appears in a conditional statement.  "
          + policies->GetRequiredPolicyError(cmPolicies::CMP0012);
        status = cmake::FATAL_ERROR;
        }
      case cmPolicies::NEW:
        break;
      }
    }
  return newResult;
  }

  //=========================================================================
  void IncrementArguments(std::list<std::string> &newArgs,
                          std::list<std::string>::iterator &argP1,
                          std::list<std::string>::iterator &argP2)
  {
    if (argP1  != newArgs.end())
      {
      argP1++;
      argP2 = argP1;
      if (argP1  != newArgs.end())
        {
        argP2++;
        }
      }
  }

  //=========================================================================
  // helper function to reduce code duplication
  void HandlePredicate(bool value, int &reducible,
                       std::list<std::string>::iterator &arg,
                       std::list<std::string> &newArgs,
                       std::list<std::string>::iterator &argP1,
                       std::list<std::string>::iterator &argP2)
  {
    if(value)
      {
      *arg = "1";
      }
    else
      {
      *arg = "0";
      }
    newArgs.erase(argP1);
    argP1 = arg;
    IncrementArguments(newArgs,argP1,argP2);
    reducible = 1;
  }

  //=========================================================================
  // helper function to reduce code duplication
  void HandleBinaryOp(bool value, int &reducible,
                       std::list<std::string>::iterator &arg,
                       std::list<std::string> &newArgs,
                       std::list<std::string>::iterator &argP1,
                       std::list<std::string>::iterator &argP2)
  {
    if(value)
      {
      *arg = "1";
      }
    else
      {
      *arg = "0";
      }
    newArgs.erase(argP2);
    newArgs.erase(argP1);
    argP1 = arg;
    IncrementArguments(newArgs,argP1,argP2);
    reducible = 1;
  }

  //=========================================================================
  // level 0 processes parenthetical expressions
  bool HandleLevel0(std::list<std::string> &newArgs,
                    cmMakefile *makefile,
                    std::string &errorString,
                    cmake::MessageType &status)
  {
  int reducible;
  do
    {
    reducible = 0;
    std::list<std::string>::iterator arg = newArgs.begin();
    while (arg != newArgs.end())
      {
      if (*arg == "(")
        {
        // search for the closing paren for this opening one
        std::list<std::string>::iterator argClose;
        argClose = arg;
        argClose++;
        unsigned int depth = 1;
        while (argClose != newArgs.end() && depth)
          {
          if (*argClose == "(")
            {
              depth++;
            }
          if (*argClose == ")")
            {
              depth--;
            }
          argClose++;
          }
        if (depth)
          {
          errorString = "mismatched parenthesis in condition";
          status = cmake::FATAL_ERROR;
          return false;
          }
        // store the reduced args in this vector
        std::vector<std::string> newArgs2;

        // copy to the list structure
        std::list<std::string>::iterator argP1 = arg;
        argP1++;
        for(; argP1 != argClose; argP1++)
          {
          newArgs2.push_back(*argP1);
          }
        newArgs2.pop_back();
        // now recursively invoke IsTrue to handle the values inside the
        // parenthetical expression
        bool value =
          cmIfCommand::IsTrue(newArgs2, errorString, makefile, status);
        if(value)
          {
          *arg = "1";
          }
        else
          {
          *arg = "0";
          }
        argP1 = arg;
        argP1++;
        // remove the now evaluated parenthetical expression
        newArgs.erase(argP1,argClose);
        }
      ++arg;
      }
    }
  while (reducible);
  return true;
  }

  //=========================================================================
  // level one handles most predicates except for NOT
  bool HandleLevel1(std::list<std::string> &newArgs,
                    cmMakefile *makefile,
                    std::string &, cmake::MessageType &)
  {
  int reducible;
  do
    {
    reducible = 0;
    std::list<std::string>::iterator arg = newArgs.begin();
    std::list<std::string>::iterator argP1;
    std::list<std::string>::iterator argP2;
    while (arg != newArgs.end())
      {
      argP1 = arg;
      IncrementArguments(newArgs,argP1,argP2);
      // does a file exist
      if (*arg == "EXISTS" && argP1  != newArgs.end())
        {
        HandlePredicate(
          cmSystemTools::FileExists((argP1)->c_str()),
          reducible, arg, newArgs, argP1, argP2);
        }
      // does a directory with this name exist
      if (*arg == "IS_DIRECTORY" && argP1  != newArgs.end())
        {
        HandlePredicate(
          cmSystemTools::FileIsDirectory((argP1)->c_str()),
          reducible, arg, newArgs, argP1, argP2);
        }
      // does a symlink with this name exist
      if (*arg == "IS_SYMLINK" && argP1  != newArgs.end())
        {
        HandlePredicate(
          cmSystemTools::FileIsSymlink((argP1)->c_str()),
          reducible, arg, newArgs, argP1, argP2);
        }
      // is the given path an absolute path ?
      if (*arg == "IS_ABSOLUTE" && argP1  != newArgs.end())
        {
        HandlePredicate(
          cmSystemTools::FileIsFullPath((argP1)->c_str()),
          reducible, arg, newArgs, argP1, argP2);
        }
      // does a command exist
      if (*arg == "COMMAND" && argP1  != newArgs.end())
        {
        HandlePredicate(
          makefile->CommandExists((argP1)->c_str()),
          reducible, arg, newArgs, argP1, argP2);
        }
      // does a policy exist
      if (*arg == "POLICY" && argP1 != newArgs.end())
        {
        cmPolicies::PolicyID pid;
        HandlePredicate(
          makefile->GetPolicies()->GetPolicyID((argP1)->c_str(), pid),
          reducible, arg, newArgs, argP1, argP2);
        }
      // does a target exist
      if (*arg == "TARGET" && argP1 != newArgs.end())
        {
        HandlePredicate(
          makefile->FindTargetToUse(*argP1)?true:false,
          reducible, arg, newArgs, argP1, argP2);
        }
      // is a variable defined
      if (*arg == "DEFINED" && argP1  != newArgs.end())
        {
        size_t argP1len = argP1->size();
        bool bdef = false;
        if(argP1len > 4 && argP1->substr(0, 4) == "ENV{" &&
           argP1->operator[](argP1len-1) == '}')
          {
          std::string env = argP1->substr(4, argP1len-5);
          bdef = cmSystemTools::GetEnv(env.c_str())?true:false;
          }
        else
          {
          bdef = makefile->IsDefinitionSet((argP1)->c_str());
          }
        HandlePredicate(bdef, reducible, arg, newArgs, argP1, argP2);
        }
      ++arg;
      }
    }
  while (reducible);
  return true;
  }

  //=========================================================================
  // level two handles most binary operations except for AND  OR
  bool HandleLevel2(std::list<std::string> &newArgs,
                    cmMakefile *makefile,
                    std::string &errorString,
                    cmake::MessageType &status)
  {
  int reducible;
  const char *def;
  const char *def2;
  do
    {
    reducible = 0;
    std::list<std::string>::iterator arg = newArgs.begin();
    std::list<std::string>::iterator argP1;
    std::list<std::string>::iterator argP2;
    while (arg != newArgs.end())
      {
      argP1 = arg;
      IncrementArguments(newArgs,argP1,argP2);
      if (argP1 != newArgs.end() && argP2 != newArgs.end() &&
        *(argP1) == "MATCHES")
        {
        def = cmIfCommand::GetVariableOrString(arg->c_str(), makefile);
        const char* rex = (argP2)->c_str();
        cmStringCommand::ClearMatches(makefile);
        cmsys::RegularExpression regEntry;
        if ( !regEntry.compile(rex) )
          {
          cmOStringStream error;
          error << "Regular expression \"" << rex << "\" cannot compile";
          errorString = error.str();
          status = cmake::FATAL_ERROR;
          return false;
          }
        if (regEntry.find(def))
          {
          cmStringCommand::StoreMatches(makefile, regEntry);
          *arg = "1";
          }
        else
          {
          *arg = "0";
          }
        newArgs.erase(argP2);
        newArgs.erase(argP1);
        argP1 = arg;
        IncrementArguments(newArgs,argP1,argP2);
        reducible = 1;
        }

      if (argP1 != newArgs.end() && *arg == "MATCHES")
        {
        *arg = "0";
        newArgs.erase(argP1);
        argP1 = arg;
        IncrementArguments(newArgs,argP1,argP2);
        reducible = 1;
        }

      if (argP1 != newArgs.end() && argP2 != newArgs.end() &&
        (*(argP1) == "LESS" || *(argP1) == "GREATER" ||
         *(argP1) == "EQUAL"))
        {
        def = cmIfCommand::GetVariableOrString(arg->c_str(), makefile);
        def2 = cmIfCommand::GetVariableOrString((argP2)->c_str(), makefile);
        double lhs;
        double rhs;
        bool result;
        if(sscanf(def, "%lg", &lhs) != 1 ||
           sscanf(def2, "%lg", &rhs) != 1)
          {
          result = false;
          }
        else if (*(argP1) == "LESS")
          {
          result = (lhs < rhs);
          }
        else if (*(argP1) == "GREATER")
          {
          result = (lhs > rhs);
          }
        else
          {
          result = (lhs == rhs);
          }
        HandleBinaryOp(result,
          reducible, arg, newArgs, argP1, argP2);
        }

      if (argP1 != newArgs.end() && argP2 != newArgs.end() &&
        (*(argP1) == "STRLESS" ||
         *(argP1) == "STREQUAL" ||
         *(argP1) == "STRGREATER"))
        {
        def = cmIfCommand::GetVariableOrString(arg->c_str(), makefile);
        def2 = cmIfCommand::GetVariableOrString((argP2)->c_str(), makefile);
        int val = strcmp(def,def2);
        bool result;
        if (*(argP1) == "STRLESS")
          {
          result = (val < 0);
          }
        else if (*(argP1) == "STRGREATER")
          {
          result = (val > 0);
          }
        else // strequal
          {
          result = (val == 0);
          }
        HandleBinaryOp(result,
          reducible, arg, newArgs, argP1, argP2);
        }

      if (argP1 != newArgs.end() && argP2 != newArgs.end() &&
        (*(argP1) == "VERSION_LESS" || *(argP1) == "VERSION_GREATER" ||
         *(argP1) == "VERSION_EQUAL"))
        {
        def = cmIfCommand::GetVariableOrString(arg->c_str(), makefile);
        def2 = cmIfCommand::GetVariableOrString((argP2)->c_str(), makefile);
        cmSystemTools::CompareOp op = cmSystemTools::OP_EQUAL;
        if(*argP1 == "VERSION_LESS")
          {
          op = cmSystemTools::OP_LESS;
          }
        else if(*argP1 == "VERSION_GREATER")
          {
          op = cmSystemTools::OP_GREATER;
          }
        bool result = cmSystemTools::VersionCompare(op, def, def2);
        HandleBinaryOp(result,
          reducible, arg, newArgs, argP1, argP2);
        }

      // is file A newer than file B
      if (argP1 != newArgs.end() && argP2 != newArgs.end() &&
          *(argP1) == "IS_NEWER_THAN")
        {
        int fileIsNewer=0;
        bool success=cmSystemTools::FileTimeCompare(arg->c_str(),
            (argP2)->c_str(),
            &fileIsNewer);
        HandleBinaryOp(
          (success==false || fileIsNewer==1 || fileIsNewer==0),
          reducible, arg, newArgs, argP1, argP2);
        }

      ++arg;
      }
    }
  while (reducible);
  return true;
  }

  //=========================================================================
  // level 3 handles NOT
  bool HandleLevel3(std::list<std::string> &newArgs,
                    cmMakefile *makefile,
                    std::string &errorString,
                    cmPolicies::PolicyStatus Policy12Status,
                    cmake::MessageType &status)
  {
  int reducible;
  do
    {
    reducible = 0;
    std::list<std::string>::iterator arg = newArgs.begin();
    std::list<std::string>::iterator argP1;
    std::list<std::string>::iterator argP2;
    while (arg != newArgs.end())
      {
      argP1 = arg;
      IncrementArguments(newArgs,argP1,argP2);
      if (argP1 != newArgs.end() && *arg == "NOT")
        {
        bool rhs = GetBooleanValueWithAutoDereference(*argP1, makefile,
                                                      errorString,
                                                      Policy12Status,
                                                      status);
        HandlePredicate(!rhs, reducible, arg, newArgs, argP1, argP2);
        }
      ++arg;
      }
    }
  while (reducible);
  return true;
  }

  //=========================================================================
  // level 4 handles AND OR
  bool HandleLevel4(std::list<std::string> &newArgs,
                    cmMakefile *makefile,
                    std::string &errorString,
                    cmPolicies::PolicyStatus Policy12Status,
                    cmake::MessageType &status)
  {
  int reducible;
  bool lhs;
  bool rhs;
  do
    {
    reducible = 0;
    std::list<std::string>::iterator arg = newArgs.begin();
    std::list<std::string>::iterator argP1;
    std::list<std::string>::iterator argP2;
    while (arg != newArgs.end())
      {
      argP1 = arg;
      IncrementArguments(newArgs,argP1,argP2);
      if (argP1 != newArgs.end() && *(argP1) == "AND" &&
        argP2 != newArgs.end())
        {
        lhs = GetBooleanValueWithAutoDereference(*arg, makefile,
                                                 errorString,
                                                 Policy12Status,
                                                 status);
        rhs = GetBooleanValueWithAutoDereference(*argP2, makefile,
                                                 errorString,
                                                 Policy12Status,
                                                 status);
        HandleBinaryOp((lhs && rhs),
          reducible, arg, newArgs, argP1, argP2);
        }

      if (argP1 != newArgs.end() && *(argP1) == "OR" &&
        argP2 != newArgs.end())
        {
        lhs = GetBooleanValueWithAutoDereference(*arg, makefile,
                                                 errorString,
                                                 Policy12Status,
                                                 status);
        rhs = GetBooleanValueWithAutoDereference(*argP2, makefile,
                                                 errorString,
                                                 Policy12Status,
                                                 status);
        HandleBinaryOp((lhs || rhs),
          reducible, arg, newArgs, argP1, argP2);
        }
      ++arg;
      }
    }
  while (reducible);
  return true;
  }
}


//=========================================================================
// order of operations,
// 1.   ( )   -- parenthetical groups
// 2.  IS_DIRECTORY EXISTS COMMAND DEFINED etc predicates
// 3. MATCHES LESS GREATER EQUAL STRLESS STRGREATER STREQUAL etc binary ops
// 4. NOT
// 5. AND OR
//
// There is an issue on whether the arguments should be values of references,
// for example IF (FOO AND BAR) should that compare the strings FOO and BAR
// or should it really do IF (${FOO} AND ${BAR}) Currently IS_DIRECTORY
// EXISTS COMMAND and DEFINED all take values. EQUAL, LESS and GREATER can
// take numeric values or variable names. STRLESS and STRGREATER take
// variable names but if the variable name is not found it will use the name
// directly. AND OR take variables or the values 0 or 1.


bool cmIfCommand::IsTrue(const std::vector<std::string> &args,
                         std::string &errorString, cmMakefile *makefile,
                         cmake::MessageType &status)
{
  errorString = "";

  // handle empty invocation
  if (args.size() < 1)
    {
    return false;
    }

  // store the reduced args in this vector
  std::list<std::string> newArgs;

  // copy to the list structure
  for(unsigned int i = 0; i < args.size(); ++i)
    {
    newArgs.push_back(args[i]);
    }

  // now loop through the arguments and see if we can reduce any of them
  // we do this multiple times. Once for each level of precedence
  // parens
  if (!HandleLevel0(newArgs, makefile, errorString, status))
    {
    return false;
    }
  //predicates
  if (!HandleLevel1(newArgs, makefile, errorString, status))
    {
    return false;
    }
  // binary ops
  if (!HandleLevel2(newArgs, makefile, errorString, status))
    {
    return false;
    }

  // used to store the value of policy CMP0012 for performance
  cmPolicies::PolicyStatus Policy12Status =
    makefile->GetPolicyStatus(cmPolicies::CMP0012);

  // NOT
  if (!HandleLevel3(newArgs, makefile, errorString,
                    Policy12Status, status))
    {
    return false;
    }
  // AND OR
  if (!HandleLevel4(newArgs, makefile, errorString,
                    Policy12Status, status))
    {
    return false;
    }

  // now at the end there should only be one argument left
  if (newArgs.size() != 1)
    {
    errorString = "Unknown arguments specified";
    status = cmake::FATAL_ERROR;
    return false;
    }

  return GetBooleanValueWithAutoDereference(*(newArgs.begin()),
                                            makefile,
                                            errorString,
                                            Policy12Status,
                                            status, true);
}

//=========================================================================
const char* cmIfCommand::GetVariableOrString(const char* str,
                                             const cmMakefile* mf)
{
  const char* def = mf->GetDefinition(str);
  if(!def)
    {
    def = str;
    }
  return def;
}
