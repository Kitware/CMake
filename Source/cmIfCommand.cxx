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
#include <stdlib.h> // required for atof
#include <deque>
#include <cmsys/RegularExpression.hxx>

bool cmIfFunctionBlocker::
IsFunctionBlocked(const cmListFileFunction& lff, cmMakefile &mf)
{
  const char* name = lff.m_Name.c_str();
  const std::vector<cmListFileArgument>& args = lff.m_Arguments;
  // always let if statements through
  if (!strcmp(name,"IF"))
    {
    return false;
    }
  
  // watch for our ELSE or ENDIF
  if (!strcmp(name,"ELSE") || !strcmp(name,"ENDIF"))
    {
    if (args == m_Args)
      {
      // if it was an else statement then we should change state
      // and block this Else Command
      if (!strcmp(name,"ELSE"))
        {
        m_IsBlocking = !m_IsBlocking;
        return true;
        }
      // otherwise it must be an ENDIF statement, in that case remove the
      // function blocker
      mf.RemoveFunctionBlocker(lff);
      return true;
      }
    else if(args.empty())
      {
      std::string err = "Empty arguments for ";
      err += name;
      err += ".  Did you mean ";
      err += name;
      err += "( ";
      for(std::vector<cmListFileArgument>::const_iterator a = m_Args.begin();
          a != m_Args.end();++a)
        {
        err += (a->Quoted?"\"":"");
        err += a->Value;
        err += (a->Quoted?"\"":"");
        err += " ";
        }
      err += ")?";
      cmSystemTools::Error(err.c_str());
      }
    }
  return m_IsBlocking;
}

bool cmIfFunctionBlocker::ShouldRemove(const cmListFileFunction& lff,
                                       cmMakefile&)
{
  if (lff.m_Name == "ENDIF")
    {
    if (lff.m_Arguments == m_Args)
      {
      return true;
      }
    }
  return false;
}

void cmIfFunctionBlocker::
ScopeEnded(cmMakefile &mf)
{
  std::string errmsg = "The end of a CMakeLists file was reached with an IF statement that was not closed properly.\nWithin the directory: ";
  errmsg += mf.GetCurrentDirectory();
  errmsg += "\nThe arguments are: ";
  for(std::vector<cmListFileArgument>::const_iterator j = m_Args.begin();
      j != m_Args.end(); ++j)
    {   
    errmsg += (j->Quoted?"\"":"");
    errmsg += j->Value;
    errmsg += (j->Quoted?"\"":"");
    errmsg += " ";
    }
  cmSystemTools::Message(errmsg.c_str(), "Warning");
}

bool cmIfCommand::InvokeInitialPass(const std::vector<cmListFileArgument>& args)
{
  bool isValid;
  
  std::vector<std::string> expandedArguments;
  m_Makefile->ExpandArguments(args, expandedArguments);
  bool isTrue = cmIfCommand::IsTrue(expandedArguments,isValid,m_Makefile);
  
  if (!isValid)
    {
    std::string err = "An IF command had incorrect arguments: ";
    unsigned int i;
    for(i =0; i < args.size(); ++i)
      {
      err += (args[i].Quoted?"\"":"");
      err += args[i].Value;
      err += (args[i].Quoted?"\"":"");
      err += " ";
      }
    this->SetError(err.c_str());
    return false;
    }
  
  cmIfFunctionBlocker *f = new cmIfFunctionBlocker();
  // if is isn't true block the commands
  f->m_IsBlocking = !isTrue;
  f->m_Args = args;
  m_Makefile->AddFunctionBlocker(f);
  
  return true;
}

// order of operations, 
// EXISTS COMMAND DEFINED 
// MATCHES LESS GREATER EQUAL STRLESS STRGREATER
// AND OR
//
// There is an issue on whether the arguments should be values of references,
// for example IF (FOO AND BAR) should that compare the strings FOO and BAR
// or should it really do IF (${FOO} AND ${BAR}) Currently EXISTS COMMAND and
// DEFINED all take values. EQUAL, LESS and GREATER can take numeric values or
// variable names. STRLESS and STRGREATER take variable names but if the
// variable name is not found it will use the name directly. AND OR take
// variables or the values 0 or 1.


bool cmIfCommand::IsTrue(const std::vector<std::string> &args,
                         bool &isValid, const cmMakefile *makefile)
{
  // check for the different signatures
  isValid = false;
  const char *def;
  const char *def2;

  // handle empty invocation
  if (args.size() < 1)
    {
    isValid = true;
    return false;
    }
  
  // store the reduced args in this vector
  std::deque<std::string> newArgs;
  int reducible;
  unsigned int i;
  
  // copy to the list structure
  for(i = 0; i < args.size(); ++i)
    {   
    newArgs.push_back(args[i]);
    }
  
  // now loop through the arguments and see if we can reduce any of them
  // we do this multiple times. Once for each level of precedence
  do
    {
    reducible = 0;
    std::deque<std::string>::iterator arg = newArgs.begin();
    while (arg != newArgs.end())
      {
      // does a file exist
      if (*arg == "EXISTS" && arg + 1  != newArgs.end())
        {
        if(cmSystemTools::FileExists((arg + 1)->c_str()))
          {
          *arg = "1";
          }
        else 
          {
          *arg = "0";
          }
        newArgs.erase(arg+1);
        reducible = 1;
        }
      // does a command exist
      if (*arg == "COMMAND" && arg + 1  != newArgs.end())
        {
        if(makefile->CommandExists((arg + 1)->c_str()))
          {
          *arg = "1";
          }
        else 
          {
          *arg = "0";
          }
        newArgs.erase(arg+1);
        reducible = 1;
        }
      // is a variable defined
      if (*arg == "DEFINED" && arg + 1  != newArgs.end())
        {
        def = makefile->GetDefinition((arg + 1)->c_str());
        if(def)
          {
          *arg = "1";
          }
        else 
          {
          *arg = "0";
          }
        newArgs.erase(arg+1);
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
    std::deque<std::string>::iterator arg = newArgs.begin();
    while (arg != newArgs.end())
      {
      if (arg + 1 != newArgs.end() && arg + 2 != newArgs.end() &&
          *(arg + 1) == "MATCHES") 
        {
        def = cmIfCommand::GetVariableOrString(arg->c_str(), makefile);
        cmsys::RegularExpression regEntry((arg+2)->c_str());
        if (regEntry.find(def))
          {
          *arg = "1";
          }
        else
          {
          *arg = "0";
          }
        newArgs.erase(arg+2);
        newArgs.erase(arg+1);
        reducible = 1;
        }

      if (arg + 1 != newArgs.end() && *arg == "MATCHES") 
        {
        *arg = "0";
        newArgs.erase(arg+1);
        reducible = 1;
        }

      if (arg + 1 != newArgs.end() && arg + 2 != newArgs.end() &&
          (*(arg + 1) == "LESS" || *(arg + 1) == "GREATER" || 
           *(arg + 1) == "EQUAL")) 
        {
        def = cmIfCommand::GetVariableOrString(arg->c_str(), makefile);
        def2 = cmIfCommand::GetVariableOrString((arg + 2)->c_str(), makefile);
        if (*(arg + 1) == "LESS")
          {
          if(atof(def) < atof(def2))
            {
            *arg = "1";
            }
          else
            {
            *arg = "0";
            }
          }
        else if (*(arg + 1) == "GREATER")
          {
          if(atof(def) > atof(def2))
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
          if(atof(def) == atof(def2))
            {
            *arg = "1";
            }
          else
            {
            *arg = "0";
            }
          }          
        newArgs.erase(arg+2);
        newArgs.erase(arg+1);
        reducible = 1;
        }

      if (arg + 1 != newArgs.end() && arg + 2 != newArgs.end() &&
          (*(arg + 1) == "STRLESS" || *(arg + 1) == "STRGREATER")) 
        {
        def = cmIfCommand::GetVariableOrString(arg->c_str(), makefile);
        def2 = cmIfCommand::GetVariableOrString((arg + 2)->c_str(), makefile);
        if (*(arg + 1) == "STRLESS")
          {
          if(strcmp(def,def2) < 0)
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
          if(strcmp(def,def2) > 0)
            {
            *arg = "1";
            }
          else
            {
            *arg = "0";
            }
          }          
        newArgs.erase(arg+2);
        newArgs.erase(arg+1);
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
    std::deque<std::string>::iterator arg = newArgs.begin();
    while (arg != newArgs.end())
      {
      if (arg + 1 != newArgs.end() && *arg == "NOT")
        {
        def = cmIfCommand::GetVariableOrNumber((arg + 1)->c_str(), makefile);
        if(!cmSystemTools::IsOff(def))
          {
          *arg = "0";
          }
        else
          {
          *arg = "1";
          }
        newArgs.erase(arg+1);
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
    std::deque<std::string>::iterator arg = newArgs.begin();
    while (arg != newArgs.end())
      {
      if (arg + 1 != newArgs.end() && *(arg + 1) == "AND" && 
          arg + 2 != newArgs.end())
        {
        def = cmIfCommand::GetVariableOrNumber(arg->c_str(), makefile);
        def2 = cmIfCommand::GetVariableOrNumber((arg + 2)->c_str(), makefile);
        if(cmSystemTools::IsOff(def) || cmSystemTools::IsOff(def2))
          {
          *arg = "0";
          }
        else
          {
          *arg = "1";
          }
        newArgs.erase(arg+2);
        newArgs.erase(arg+1);
        reducible = 1;
        }

      if (arg + 1 != newArgs.end() && *(arg + 1) == "OR" && 
          arg + 2 != newArgs.end())
        {
        def = cmIfCommand::GetVariableOrNumber(arg->c_str(), makefile);
        def2 = cmIfCommand::GetVariableOrNumber((arg + 2)->c_str(), makefile);
        if(cmSystemTools::IsOff(def) && cmSystemTools::IsOff(def2))
          {
          *arg = "0";
          }
        else
          {
          *arg = "1";
          }
        newArgs.erase(arg+2);
        newArgs.erase(arg+1);
        reducible = 1;
        }
  
      ++arg;
      }
    }
  while (reducible);

  // now at the end there should only be one argument left
  if (newArgs.size() == 1)
    {
    isValid = true;
    if (newArgs[0] == "0")
      {
      return false;
      }
    if (newArgs[0] == "1")
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
