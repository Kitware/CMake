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

bool cmIfFunctionBlocker::
IsFunctionBlocked(const char *name, const std::vector<std::string> &args, 
                  cmMakefile &mf)
{
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
      mf.RemoveFunctionBlocker("ENDIF",args);
      return true;
      }
    else if(args.empty())
      {
      std::string err = "Empty arguments for ";
      err += name;
      err += ".  Did you mean ";
      err += name;
      err += "( ";
      for(std::vector<std::string>::const_iterator a = m_Args.begin();
          a != m_Args.end();++a)
        {
        err += *a;
        err += " ";
        }
      err += ")?";
      cmSystemTools::Error(err.c_str());
      }
    }
  return m_IsBlocking;
}

bool cmIfFunctionBlocker::
ShouldRemove(const char *name, const std::vector<std::string> &args, 
             cmMakefile &) 
{
  if (!strcmp(name,"ELSE") || !strcmp(name,"ENDIF"))
    {
    if (args == m_Args)
      {
      return true;
      }
    }
  return false;
}

void cmIfFunctionBlocker::
ScopeEnded(cmMakefile &mf)
{
  const char* versionValue
    = mf.GetDefinition("CMAKE_MINIMUM_REQUIRED_VERSION");
  if (!versionValue || (atof(versionValue) <= 1.4))
    {
    return;
    }
  
  std::string errmsg = "The end of a CMakeLists file was reached with an IF statement that was not closed properly.\nWithin the directory: ";
  errmsg += mf.GetCurrentDirectory();
  errmsg += "\nThe arguments are: ";
  for(std::vector<std::string>::const_iterator j = m_Args.begin();
      j != m_Args.end(); ++j)
    {   
    errmsg += *j;
    errmsg += " ";
    }
  cmSystemTools::Error(errmsg.c_str());
}

bool cmIfCommand::InitialPass(std::vector<std::string> const& args)
{
  bool isValid;
  bool isTrue = cmIfCommand::IsTrue(args,isValid,m_Makefile);
  
  if (!isValid)
    {
    std::string err = "An IF command had incorrect arguments: ";
    unsigned int i;
    for(i =0; i < args.size(); ++i)
      {
      err += args[i];
      err += " ";
      }
    this->SetError(err.c_str());
    return false;
    }
  
  cmIfFunctionBlocker *f = new cmIfFunctionBlocker();
  // if is isn't true block the commands
  f->m_IsBlocking = !isTrue;
  for(std::vector<std::string>::const_iterator j = args.begin();
      j != args.end(); ++j)
    {   
    f->m_Args.push_back(*j);
    }
  m_Makefile->AddFunctionBlocker(f);
  
  return true;
}

bool cmIfCommand::IsTrue(const std::vector<std::string> &args, bool &isValid,
                         const cmMakefile *makefile)
{
  // check for the different signatures
  bool isTrue = true;
  isValid = false;
  const char *def;
  const char *def2;

  if(args.size() < 1 )
    {
    isValid = true;
    return false;
    }

  if (args.size() == 1)
    {
    def = makefile->GetDefinition(args[0].c_str());
    if(cmSystemTools::IsOff(def))
      {
      isTrue = false;
      }
    isValid = true;
    }

  if (args.size() == 2 && (args[0] == "NOT"))
    {
    def = makefile->GetDefinition(args[1].c_str());
    if(!cmSystemTools::IsOff(def))
      {
      isTrue = false;
      }
    isValid = true;

    }

  if (args.size() == 2 && (args[0] == "COMMAND"))
    {
    if(!makefile->CommandExists(args[1].c_str()))
      {
      isTrue = false;
      }
    isValid = true;
    }

  if (args.size() == 2 && (args[0] == "EXISTS"))
    {
    if(!cmSystemTools::FileExists(args[1].c_str()))
      {
      isTrue = false;
      }
    isValid = true;
    }

  if (args.size() == 2 && (args[0] == "MATCHES"))
    {
    if(!cmSystemTools::FileExists(args[1].c_str()))
      {
      isTrue = false;
      }
    isValid = true;
    }

  if (args.size() == 3 && (args[1] == "AND"))
    {
    def = makefile->GetDefinition(args[0].c_str());
    def2 = makefile->GetDefinition(args[2].c_str());
    if(cmSystemTools::IsOff(def) || cmSystemTools::IsOff(def2))
      {
      isTrue = false;
      }
    isValid = true;
    }
  
  if (args.size() == 3 && (args[1] == "OR"))
    {
    def = makefile->GetDefinition(args[0].c_str());
    def2 = makefile->GetDefinition(args[2].c_str());
    if(cmSystemTools::IsOff(def) && cmSystemTools::IsOff(def2))
      {
      isTrue = false;
      }
    isValid = true;
    }

  if (args.size() == 3 && (args[1] == "MATCHES"))
    {
    def = cmIfCommand::GetVariableOrString(args[0].c_str(), makefile);
    cmRegularExpression regEntry(args[2].c_str());
    
    // check for black line or comment
    if (!regEntry.find(def))
      {
      isTrue = false;
      }
    isValid = true;
    }
  
  if (args.size() == 3 && (args[1] == "LESS"))
    {
    def = cmIfCommand::GetVariableOrString(args[0].c_str(), makefile);
    def2 = cmIfCommand::GetVariableOrString(args[2].c_str(), makefile);
    if (!def)
      {
      def = args[0].c_str();
      }
    if (!def2)
      {
      def2 = args[2].c_str();
      }    
    if(atof(def) >= atof(def2))
      {
      isTrue = false;
      }
    isValid = true;
    }

  if (args.size() == 3 && (args[1] == "GREATER"))
    {
    def = cmIfCommand::GetVariableOrString(args[0].c_str(), makefile);
    def2 = cmIfCommand::GetVariableOrString(args[2].c_str(), makefile);
    if(atof(def) <= atof(def2))
      {
      isTrue = false;
      }
    isValid = true;
    }

  if (args.size() == 3 && (args[1] == "STRLESS"))
    {
    def = cmIfCommand::GetVariableOrString(args[0].c_str(), makefile);
    def2 = cmIfCommand::GetVariableOrString(args[2].c_str(), makefile);
    if(strcmp(def,def2) >= 0)
      {
      isTrue = false;
      }
    isValid = true;
    }

  if (args.size() == 3 && (args[1] == "STRGREATER"))
    {
    def = cmIfCommand::GetVariableOrString(args[0].c_str(), makefile);
    def2 = cmIfCommand::GetVariableOrString(args[2].c_str(), makefile);
    if(strcmp(def,def2) <= 0)
      {
      isTrue = false;
      }
    isValid = true;
    }
  return isTrue;
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
