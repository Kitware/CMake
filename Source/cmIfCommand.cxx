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
#include <list>
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
  char* errorString = 0;
  
  std::vector<std::string> expandedArguments;
  m_Makefile->ExpandArguments(args, expandedArguments);
  bool isTrue = cmIfCommand::IsTrue(expandedArguments,&errorString,m_Makefile);
  
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
  f->m_IsBlocking = !isTrue;
  f->m_Args = args;
  m_Makefile->AddFunctionBlocker(f);
  
  return true;
}

// order of operations, 
// EXISTS COMMAND DEFINED 
// MATCHES LESS GREATER EQUAL STRLESS STRGREATER STREQUAL 
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
                         char **errorString, const cmMakefile *makefile)
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

  // this is a super ugly hack. Basically old versiosn of VTK and ITK have a
  // bad test to check for more recent versions of CMake in the
  // CMakeLists.txt file for libtiff. So when we reved CMake up to 2.0 the
  // test started failing because the minor version went to zero this causes
  // the test to pass
  if (args.size() == 3 &&
    (makefile->GetDefinition("VTKTIFF_SOURCE_DIR") ||
     makefile->GetDefinition("ITKTIFF_SOURCE_DIR")) &&
    args[0] == "CMAKE_MINOR_VERSION" &&
    args[1] == "MATCHES")
    {
    delete [] *errorString;
    *errorString = 0;
    return true;
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
      argP1++;
      argP2 = argP1;
      argP2++;
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
        argP1++;
        argP2 = argP1;
        argP2++;
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
        argP1++;
        argP2 = argP1;
        argP2++;
        reducible = 1;
        }
      // is a variable defined
      if (*arg == "DEFINED" && argP1  != newArgs.end())
        {
        def = makefile->GetDefinition((argP1)->c_str());
        if(def)
          {
          *arg = "1";
          }
        else 
          {
          *arg = "0";
          }
        newArgs.erase(argP1);
        argP1 = arg;
        argP1++;
        argP2 = argP1;
        argP2++;
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
      argP1++;
      argP2 = argP1;
      argP2++;
      if (argP1 != newArgs.end() && argP2 != newArgs.end() &&
        *(argP1) == "MATCHES") 
        {
        def = cmIfCommand::GetVariableOrString(arg->c_str(), makefile);
        const char* rex = (argP2)->c_str();
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
          *arg = "1";
          }
        else
          {
          *arg = "0";
          }
        newArgs.erase(argP2);
        newArgs.erase(argP1);
        argP1 = arg;
        argP1++;
        argP2 = argP1;
        argP2++;
        reducible = 1;
        }

      if (argP1 != newArgs.end() && *arg == "MATCHES") 
        {
        *arg = "0";
        newArgs.erase(argP1);
        argP1 = arg;
        argP1++;
        argP2 = argP1;
        argP2++;
        reducible = 1;
        }

      if (argP1 != newArgs.end() && argP2 != newArgs.end() &&
        (*(argP1) == "LESS" || *(argP1) == "GREATER" || 
         *(argP1) == "EQUAL")) 
        {
        def = cmIfCommand::GetVariableOrString(arg->c_str(), makefile);
        def2 = cmIfCommand::GetVariableOrString((argP2)->c_str(), makefile);
        if (*(argP1) == "LESS")
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
        else if (*(argP1) == "GREATER")
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
        newArgs.erase(argP2);
        newArgs.erase(argP1);
        argP1 = arg;
        argP1++;
        argP2 = argP1;
        argP2++;
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
        argP1++;
        argP2 = argP1;
        argP2++;
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
      argP1++;
      argP2 = argP1;
      argP2++;
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
        argP1++;
        argP2 = argP1;
        argP2++;
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
      argP1++;
      argP2 = argP1;
      argP2++;
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
        argP1++;
        argP2 = argP1;
        argP2++;
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
        argP1++;
        argP2 = argP1;
        argP2++;
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
