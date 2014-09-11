/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2014 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#include "cmConditionEvaluator.h"

cmConditionEvaluator::cmConditionEvaluator(cmMakefile& makefile):
  Makefile(makefile),
  Policy12Status(makefile.GetPolicyStatus(cmPolicies::CMP0012))
{

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

bool cmConditionEvaluator::IsTrue(
  const std::vector<std::string> &args,
  std::string &errorString,
  cmake::MessageType &status)
{
  errorString = "";

  // handle empty invocation
  if (args.size() < 1)
    {
    return false;
    }

  // store the reduced args in this vector
  cmArgumentList newArgs;

  // copy to the list structure
  for(unsigned int i = 0; i < args.size(); ++i)
    {
    newArgs.push_back(args[i]);
    }

  // now loop through the arguments and see if we can reduce any of them
  // we do this multiple times. Once for each level of precedence
  // parens
  if (!this->HandleLevel0(newArgs, errorString, status))
    {
    return false;
    }
  //predicates
  if (!this->HandleLevel1(newArgs, errorString, status))
    {
    return false;
    }
  // binary ops
  if (!this->HandleLevel2(newArgs, errorString, status))
    {
    return false;
    }

  // NOT
  if (!this->HandleLevel3(newArgs, errorString, status))
    {
    return false;
    }
  // AND OR
  if (!this->HandleLevel4(newArgs, errorString, status))
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

  return this->GetBooleanValueWithAutoDereference(*(newArgs.begin()),
    errorString, status, true);
}

//=========================================================================
const char* cmConditionEvaluator::GetVariableOrString(
    const std::string& str) const
{
  const char* def = this->Makefile.GetDefinition(str);

  if(!def)
    {
    def = str.c_str();
    }

  return def;
}

//=========================================================================
bool cmConditionEvaluator::GetBooleanValue(
  std::string& arg) const
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
  const char* def = this->Makefile.GetDefinition(arg);
  return !cmSystemTools::IsOff(def);
}

//=========================================================================
// Boolean value behavior from CMake 2.6.4 and below.
bool cmConditionEvaluator::GetBooleanValueOld(
  std::string const& arg, bool one) const
{
  if(one)
    {
    // Old IsTrue behavior for single argument.
    if(arg == "0")
      { return false; }
    else if(arg == "1")
      { return true; }
    else
      { return !cmSystemTools::IsOff(this->Makefile.GetDefinition(arg)); }
    }
  else
    {
    // Old GetVariableOrNumber behavior.
    const char* def = this->Makefile.GetDefinition(arg);
    if(!def && atoi(arg.c_str()))
      {
      def = arg.c_str();
      }
    return !cmSystemTools::IsOff(def);
    }
}

//=========================================================================
// returns the resulting boolean value
bool cmConditionEvaluator::GetBooleanValueWithAutoDereference(
  std::string &newArg,
  std::string &errorString,
  cmake::MessageType &status,
  bool oneArg) const
{
  // Use the policy if it is set.
  if (this->Policy12Status == cmPolicies::NEW)
    {
    return GetBooleanValue(newArg);
    }
  else if (this->Policy12Status == cmPolicies::OLD)
    {
    return GetBooleanValueOld(newArg, oneArg);
    }

  // Check policy only if old and new results differ.
  bool newResult = this->GetBooleanValue(newArg);
  bool oldResult = this->GetBooleanValueOld(newArg, oneArg);
  if(newResult != oldResult)
    {
    switch(this->Policy12Status)
      {
      case cmPolicies::WARN:
        {
        cmPolicies* policies = this->Makefile.GetPolicies();
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
        cmPolicies* policies = this->Makefile.GetPolicies();
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
void cmConditionEvaluator::IncrementArguments(cmArgumentList &newArgs,
                        cmArgumentList::iterator &argP1,
                        cmArgumentList::iterator &argP2) const
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
void cmConditionEvaluator::HandlePredicate(bool value, int &reducible,
                     cmArgumentList::iterator &arg,
                     cmArgumentList &newArgs,
                     cmArgumentList::iterator &argP1,
                     cmArgumentList::iterator &argP2) const
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
  this->IncrementArguments(newArgs,argP1,argP2);
  reducible = 1;
}

//=========================================================================
// helper function to reduce code duplication
void cmConditionEvaluator::HandleBinaryOp(bool value, int &reducible,
                     cmArgumentList::iterator &arg,
                     cmArgumentList &newArgs,
                     cmArgumentList::iterator &argP1,
                     cmArgumentList::iterator &argP2)
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
  this->IncrementArguments(newArgs,argP1,argP2);
  reducible = 1;
}

//=========================================================================
// level 0 processes parenthetical expressions
bool cmConditionEvaluator::HandleLevel0(cmArgumentList &newArgs,
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
          this->IsTrue(newArgs2, errorString, status);
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
bool cmConditionEvaluator::HandleLevel1(cmArgumentList &newArgs,
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
      this->IncrementArguments(newArgs,argP1,argP2);
      // does a file exist
      if (*arg == "EXISTS" && argP1  != newArgs.end())
        {
        this->HandlePredicate(
          cmSystemTools::FileExists((argP1)->c_str()),
          reducible, arg, newArgs, argP1, argP2);
        }
      // does a directory with this name exist
      if (*arg == "IS_DIRECTORY" && argP1  != newArgs.end())
        {
        this->HandlePredicate(
          cmSystemTools::FileIsDirectory((argP1)->c_str()),
          reducible, arg, newArgs, argP1, argP2);
        }
      // does a symlink with this name exist
      if (*arg == "IS_SYMLINK" && argP1  != newArgs.end())
        {
        this->HandlePredicate(
          cmSystemTools::FileIsSymlink((argP1)->c_str()),
          reducible, arg, newArgs, argP1, argP2);
        }
      // is the given path an absolute path ?
      if (*arg == "IS_ABSOLUTE" && argP1  != newArgs.end())
        {
        this->HandlePredicate(
          cmSystemTools::FileIsFullPath((argP1)->c_str()),
          reducible, arg, newArgs, argP1, argP2);
        }
      // does a command exist
      if (*arg == "COMMAND" && argP1  != newArgs.end())
        {
        this->HandlePredicate(
          this->Makefile.CommandExists((argP1)->c_str()),
          reducible, arg, newArgs, argP1, argP2);
        }
      // does a policy exist
      if (*arg == "POLICY" && argP1 != newArgs.end())
        {
        cmPolicies::PolicyID pid;
        this->HandlePredicate(
          this->Makefile.GetPolicies()->GetPolicyID((argP1)->c_str(), pid),
          reducible, arg, newArgs, argP1, argP2);
        }
      // does a target exist
      if (*arg == "TARGET" && argP1 != newArgs.end())
        {
        this->HandlePredicate(
          this->Makefile.FindTargetToUse(*argP1)?true:false,
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
          bdef = this->Makefile.IsDefinitionSet(*(argP1));
          }
        this->HandlePredicate(bdef, reducible, arg, newArgs, argP1, argP2);
        }
      ++arg;
      }
    }
  while (reducible);
  return true;
}

//=========================================================================
// level two handles most binary operations except for AND  OR
bool cmConditionEvaluator::HandleLevel2(cmArgumentList &newArgs,
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
      this->IncrementArguments(newArgs,argP1,argP2);
      if (argP1 != newArgs.end() && argP2 != newArgs.end() &&
        *(argP1) == "MATCHES")
        {
        def = this->GetVariableOrString(*arg);
        const char* rex = (argP2)->c_str();
        this->Makefile.ClearMatches();
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
          this->Makefile.StoreMatches(regEntry);
          *arg = "1";
          }
        else
          {
          *arg = "0";
          }
        newArgs.erase(argP2);
        newArgs.erase(argP1);
        argP1 = arg;
        this->IncrementArguments(newArgs,argP1,argP2);
        reducible = 1;
        }

      if (argP1 != newArgs.end() && *arg == "MATCHES")
        {
        *arg = "0";
        newArgs.erase(argP1);
        argP1 = arg;
        this->IncrementArguments(newArgs,argP1,argP2);
        reducible = 1;
        }

      if (argP1 != newArgs.end() && argP2 != newArgs.end() &&
        (*(argP1) == "LESS" || *(argP1) == "GREATER" ||
         *(argP1) == "EQUAL"))
        {
        def = this->GetVariableOrString(*arg);
        def2 = this->GetVariableOrString(*argP2);
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
        def = this->GetVariableOrString(*arg);
        def2 = this->GetVariableOrString(*argP2);
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
        this->HandleBinaryOp(result,
          reducible, arg, newArgs, argP1, argP2);
        }

      if (argP1 != newArgs.end() && argP2 != newArgs.end() &&
        (*(argP1) == "VERSION_LESS" || *(argP1) == "VERSION_GREATER" ||
         *(argP1) == "VERSION_EQUAL"))
        {
        def = this->GetVariableOrString(*arg);
        def2 = this->GetVariableOrString(*argP2);
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
        this->HandleBinaryOp(result,
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
        this->HandleBinaryOp(
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
bool cmConditionEvaluator::HandleLevel3(cmArgumentList &newArgs,
                  std::string &errorString,
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
        bool rhs = this->GetBooleanValueWithAutoDereference(*argP1,
                                                      errorString,
                                                      status);
        this->HandlePredicate(!rhs, reducible, arg, newArgs, argP1, argP2);
        }
      ++arg;
      }
    }
  while (reducible);
  return true;
}

//=========================================================================
// level 4 handles AND OR
bool cmConditionEvaluator::HandleLevel4(cmArgumentList &newArgs,
                  std::string &errorString,
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
        lhs = this->GetBooleanValueWithAutoDereference(*arg,
                                                 errorString,
                                                 status);
        rhs = this->GetBooleanValueWithAutoDereference(*argP2,
                                                 errorString,
                                                 status);
        this->HandleBinaryOp((lhs && rhs),
          reducible, arg, newArgs, argP1, argP2);
        }

      if (argP1 != newArgs.end() && *(argP1) == "OR" &&
        argP2 != newArgs.end())
        {
        lhs = this->GetBooleanValueWithAutoDereference(*arg,
                                                 errorString,
                                                 status);
        rhs = this->GetBooleanValueWithAutoDereference(*argP2,
                                                 errorString,
                                                 status);
        this->HandleBinaryOp((lhs || rhs),
          reducible, arg, newArgs, argP1, argP2);
        }
      ++arg;
      }
    }
  while (reducible);
  return true;
}
