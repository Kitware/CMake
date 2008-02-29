/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "cmIfCommand.h"
#include "cmStringCommand.h"

#include <stdlib.h> // required for atof
#include <list>
#include <cmsys/RegularExpression.hxx>

bool cmIfFunctionBlocker::
IsFunctionBlocked(const cmListFileFunction& lff, cmMakefile &mf,
                  cmExecutionStatus &inStatus)
{
  // Prevent recusion and don't let this blocker block its own
  // commands.
  if (this->Executing)
    {
    return false;
    }

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
      // execute the functions for the true parts of the if statement
      this->Executing = true;
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
            char* errorString = 0;
            
            std::vector<std::string> expandedArguments;
            mf.ExpandArguments(this->Functions[c].Arguments, 
                               expandedArguments);
            bool isTrue = 
              cmIfCommand::IsTrue(expandedArguments,&errorString,&mf);
            
            if (errorString)
              {
              std::string err = "had incorrect arguments: ";
              unsigned int i;
              for(i =0; i < this->Functions[c].Arguments.size(); ++i)
                {
                err += (this->Functions[c].Arguments[i].Quoted?"\"":"");
                err += this->Functions[c].Arguments[i].Value;
                err += (this->Functions[c].Arguments[i].Quoted?"\"":"");
                err += " ";
                }
              err += "(";
              err += errorString;
              err += ").";
              cmSystemTools::Error(err.c_str());
              delete [] errorString;
              return false;
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
            mf.RemoveFunctionBlocker(lff);
            return true;
            }
          if (status.GetBreakInvoked())
            {
            inStatus.SetBreakInvoked(true);
            mf.RemoveFunctionBlocker(lff);
            return true;
            }
          }
        }
      mf.RemoveFunctionBlocker(lff);
      return true;
      }
    }
  
  // record the command
  this->Functions.push_back(lff);
  
  // always return true
  return true;
}

bool cmIfFunctionBlocker::ShouldRemove(const cmListFileFunction& lff,
                                       cmMakefile& mf)
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

void cmIfFunctionBlocker::
ScopeEnded(cmMakefile &mf)
{
  std::string errmsg = "The end of a CMakeLists file was reached with an "
    "IF statement that was not closed properly.\nWithin the directory: ";
  errmsg += mf.GetCurrentDirectory();
  errmsg += "\nThe arguments are: ";
  for(std::vector<cmListFileArgument>::const_iterator j = this->Args.begin();
      j != this->Args.end(); ++j)
    {   
    errmsg += (j->Quoted?"\"":"");
    errmsg += j->Value;
    errmsg += (j->Quoted?"\"":"");
    errmsg += " ";
    }
  cmSystemTools::Message(errmsg.c_str(), "Warning");
}

bool cmIfCommand
::InvokeInitialPass(const std::vector<cmListFileArgument>& args, 
                    cmExecutionStatus &)
{
  char* errorString = 0;
  
  std::vector<std::string> expandedArguments;
  this->Makefile->ExpandArguments(args, expandedArguments);
  bool isTrue = 
    cmIfCommand::IsTrue(expandedArguments,&errorString,this->Makefile);
  
  if (errorString)
    {
    std::string err = "had incorrect arguments: ";
    unsigned int i;
    for(i =0; i < args.size(); ++i)
      {
      err += (args[i].Quoted?"\"":"");
      err += args[i].Value;
      err += (args[i].Quoted?"\"":"");
      err += " ";
      }
    err += "(";
    err += errorString;
    err += ").";
    this->SetError(err.c_str());
    delete [] errorString;
    return false;
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
}


// order of operations, 
// IS_DIRECTORY EXISTS COMMAND DEFINED 
// MATCHES LESS GREATER EQUAL STRLESS STRGREATER STREQUAL 
// AND OR
//
// There is an issue on whether the arguments should be values of references,
// for example IF (FOO AND BAR) should that compare the strings FOO and BAR
// or should it really do IF (${FOO} AND ${BAR}) Currently IS_DIRECTORY
// EXISTS COMMAND and DEFINED all take values. EQUAL, LESS and GREATER can
// take numeric values or variable names. STRLESS and STRGREATER take
// variable names but if the variable name is not found it will use the name
// directly. AND OR take variables or the values 0 or 1.


bool cmIfCommand::IsTrue(const std::vector<std::string> &args,
                         char **errorString, cmMakefile *makefile)
{
  // check for the different signatures
  const char *def;
  const char *def2;
  const char* msg = "Unknown arguments specified";
  *errorString = new char[strlen(msg) + 1];
  strcpy(*errorString, msg);

  // handle empty invocation
  if (args.size() < 1)
    {
    delete [] *errorString;
    *errorString = 0;
    return false;
    }

  // store the reduced args in this vector
  std::list<std::string> newArgs;
  int reducible;
  unsigned int i;

  // copy to the list structure
  for(i = 0; i < args.size(); ++i)
    {   
    newArgs.push_back(args[i]);
    }
  std::list<std::string>::iterator argP1;
  std::list<std::string>::iterator argP2;

  // now loop through the arguments and see if we can reduce any of them
  // we do this multiple times. Once for each level of precedence
  do
    {
    reducible = 0;
    std::list<std::string>::iterator arg = newArgs.begin();
    while (arg != newArgs.end())
      {
      argP1 = arg;
      IncrementArguments(newArgs,argP1,argP2);
      // does a file exist
      if (*arg == "EXISTS" && argP1  != newArgs.end())
        {
        if(cmSystemTools::FileExists((argP1)->c_str()))
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
      // does a directory with this name exist
      if (*arg == "IS_DIRECTORY" && argP1  != newArgs.end())
        {
        if(cmSystemTools::FileIsDirectory((argP1)->c_str()))
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
      // is the given path an absolute path ?
      if (*arg == "IS_ABSOLUTE" && argP1  != newArgs.end())
        {
        if(cmSystemTools::FileIsFullPath((argP1)->c_str()))
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
      // does a command exist
      if (*arg == "COMMAND" && argP1  != newArgs.end())
        {
        if(makefile->CommandExists((argP1)->c_str()))
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
        if(bdef)
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
      ++arg;
      }
    }
  while (reducible);


  // now loop through the arguments and see if we can reduce any of them
  // we do this multiple times. Once for each level of precedence
  do
    {
    reducible = 0;
    std::list<std::string>::iterator arg = newArgs.begin();

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
          delete [] *errorString;
          *errorString = new char[error.str().size() + 1];
          strcpy(*errorString, error.str().c_str());
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
        if(sscanf(def, "%lg", &lhs) != 1 ||
           sscanf(def2, "%lg", &rhs) != 1)
          {
          *arg = "0";
          }
        else if (*(argP1) == "LESS")
          {
          if(lhs < rhs)
            {
            *arg = "1";
            }
          else
            {
            *arg = "0";
            }
          }
        else if (*(argP1) == "GREATER")
          {
          if(lhs > rhs)
            {
            *arg = "1";
            }
          else
            {
            *arg = "0";
            }
          }          
        else
          {
          if(lhs == rhs)
            {
            *arg = "1";
            }
          else
            {
            *arg = "0";
            }
          }          
        newArgs.erase(argP2);
        newArgs.erase(argP1);
        argP1 = arg;
        IncrementArguments(newArgs,argP1,argP2);
        reducible = 1;
        }

      if (argP1 != newArgs.end() && argP2 != newArgs.end() &&
        (*(argP1) == "STRLESS" || 
         *(argP1) == "STREQUAL" || 
         *(argP1) == "STRGREATER")) 
        {
        def = cmIfCommand::GetVariableOrString(arg->c_str(), makefile);
        def2 = cmIfCommand::GetVariableOrString((argP2)->c_str(), makefile);
        int val = strcmp(def,def2);
        int result;
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
        if(result)
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

      // is file A newer than file B
      if (argP1 != newArgs.end() && argP2 != newArgs.end() &&
          *(argP1) == "IS_NEWER_THAN")
        {
        int fileIsNewer=0;
        bool success=cmSystemTools::FileTimeCompare(arg->c_str(),
            (argP2)->c_str(),
            &fileIsNewer);
        if(success==false || fileIsNewer==1 || fileIsNewer==0)
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

      ++arg;
      }
    }
  while (reducible);


  // now loop through the arguments and see if we can reduce any of them
  // we do this multiple times. Once for each level of precedence
  do
    {
    reducible = 0;
    std::list<std::string>::iterator arg = newArgs.begin();
    while (arg != newArgs.end())
      {
      argP1 = arg;
      IncrementArguments(newArgs,argP1,argP2);
      if (argP1 != newArgs.end() && *arg == "NOT")
        {
        def = cmIfCommand::GetVariableOrNumber((argP1)->c_str(), makefile);
        if(!cmSystemTools::IsOff(def))
          {
          *arg = "0";
          }
        else
          {
          *arg = "1";
          }
        newArgs.erase(argP1);
        argP1 = arg;
        IncrementArguments(newArgs,argP1,argP2);
        reducible = 1;
        }
      ++arg;
      }
    }
  while (reducible);

  // now loop through the arguments and see if we can reduce any of them
  // we do this multiple times. Once for each level of precedence
  do
    {
    reducible = 0;
    std::list<std::string>::iterator arg = newArgs.begin();
    while (arg != newArgs.end())
      {
      argP1 = arg;
      IncrementArguments(newArgs,argP1,argP2);
      if (argP1 != newArgs.end() && *(argP1) == "AND" && 
        argP2 != newArgs.end())
        {
        def = cmIfCommand::GetVariableOrNumber(arg->c_str(), makefile);
        def2 = cmIfCommand::GetVariableOrNumber((argP2)->c_str(), makefile);
        if(cmSystemTools::IsOff(def) || cmSystemTools::IsOff(def2))
          {
          *arg = "0";
          }
        else
          {
          *arg = "1";
          }
        newArgs.erase(argP2);
        newArgs.erase(argP1);
        argP1 = arg;
        IncrementArguments(newArgs,argP1,argP2);
        reducible = 1;
        }

      if (argP1 != newArgs.end() && *(argP1) == "OR" && 
        argP2 != newArgs.end())
        {
        def = cmIfCommand::GetVariableOrNumber(arg->c_str(), makefile);
        def2 = cmIfCommand::GetVariableOrNumber((argP2)->c_str(), makefile);
        if(cmSystemTools::IsOff(def) && cmSystemTools::IsOff(def2))
          {
          *arg = "0";
          }
        else
          {
          *arg = "1";
          }
        newArgs.erase(argP2);
        newArgs.erase(argP1);
        argP1 = arg;
        IncrementArguments(newArgs,argP1,argP2);
        reducible = 1;
        }

      ++arg;
      }
    }
  while (reducible);

  // now at the end there should only be one argument left
  if (newArgs.size() == 1)
    {
    delete [] *errorString;
    *errorString = 0;
    if (*newArgs.begin() == "0")
      {
      return false;
      }
    if (*newArgs.begin() == "1")
      {
      return true;
      }
    def = makefile->GetDefinition(args[0].c_str());
    if(cmSystemTools::IsOff(def))
      {
      return false;
      }
    }

  return true;
}

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

const char* cmIfCommand::GetVariableOrNumber(const char* str,
                                             const cmMakefile* mf)
{
  const char* def = mf->GetDefinition(str);
  if(!def)
    {
    if (atoi(str))
      {
      def = str;
      }
    }
  return def;
}
