/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "cmMacroCommand.h"
#include "cmCacheManager.h"

bool cmMacroFunctionBlocker::
IsFunctionBlocked(const char *name, const std::vector<std::string> &args, 
                  cmMakefile &mf) 
{
  // record commands until we hit the ENDMACRO
  // at the ENDMACRO call we shift gears and start looking for invocations
  if (!strcmp(name,"ENDMACRO") && args[0] == m_Args[0])
    {
    m_Executing = true;
    return true;
    }
  
  if (!m_Executing)
    {
    // if it wasn't an endmacro and we are not executing then we must be
    // recording
    m_Commands.push_back(name);
    std::vector<std::string> newArgs;
    for(std::vector<std::string>::const_iterator j = args.begin();
        j != args.end(); ++j)
      {   
      newArgs.push_back(*j);
      }
    m_CommandArguments.push_back(newArgs);
    return true;
    }
  
  // otherwise the macro has been recorded and we are executing
  // so we look for macro invocations
  if (!strcmp(name,m_Args[0].c_str()))
    {
    // make sure the number of arguments matches
    if (args.size() != m_Args.size() - 1)
      {
      cmSystemTools::Error("A macro was invoked without the correct number of arguments. The macro name was: ", m_Args[0].c_str());
      }
    // for each recorded command
    for(unsigned int c = 0; c < m_Commands.size(); ++c)
      {
      // perform argument replacement
      std::vector<std::string> newArgs;
      // for each argument of this command
      for (std::vector<std::string>::const_iterator k = 
             m_CommandArguments[c].begin();
           k != m_CommandArguments[c].end(); ++k)
        {
        // replace any matches with the formal arguments
        std::string tmps = *k;
        // for each formal macro argument
        for (unsigned int j = 1; j < m_Args.size(); ++j)
          {
          std::string variable = "${";
          variable += m_Args[j];
          variable += "}"; 
          cmSystemTools::ReplaceString(tmps, variable.c_str(),
                                       args[j-1].c_str());
          }
        newArgs.push_back(tmps);
        }
      // execute command
      mf.ExecuteCommand(m_Commands[c],newArgs);
      }
    return true;
    }

  // if not an invocation then it is just an ordinary line
  return false;
}

bool cmMacroFunctionBlocker::
ShouldRemove(const char *, const std::vector<std::string> &, 
             cmMakefile &) 
{
  return false;
}

void cmMacroFunctionBlocker::
ScopeEnded(cmMakefile &) 
{
  // macros never leave scope
}

bool cmMacroCommand::InitialPass(std::vector<std::string> const& argsIn)
{
  std::vector<std::string> args;
  cmSystemTools::ExpandListArguments(argsIn, args);

  if(args.size() < 1)
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }
  
  // create a function blocker
  cmMacroFunctionBlocker *f = new cmMacroFunctionBlocker();
  for(std::vector<std::string>::const_iterator j = args.begin();
      j != args.end(); ++j)
    {   
    f->m_Args.push_back(*j);
    }
  m_Makefile->AddFunctionBlocker(f);
  
  return true;
}

