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
#include "cmWhileCommand.h"
#include "cmIfCommand.h"

bool cmWhileFunctionBlocker::
IsFunctionBlocked(const cmListFileFunction& lff, cmMakefile &mf) 
{
  // Prevent recusion and don't let this blocker block its own
  // commands.
  if (m_Executing)
    {
    return false;
    }
  
  // at end of for each execute recorded commands
  if (lff.m_Name == "ENDWHILE")
    {
    char* errorString = 0;
    
    std::vector<std::string> expandedArguments;
    mf.ExpandArguments(m_Args, expandedArguments);
    bool isTrue = 
      cmIfCommand::IsTrue(expandedArguments,&errorString,&mf);

    m_Executing = true;
    while (isTrue)
      {      
      // Invoke all the functions that were collected in the block.
      for(unsigned int c = 0; c < m_Functions.size(); ++c)
        {
        mf.ExecuteCommand(m_Functions[c]);
        }
      expandedArguments.clear();
      mf.ExpandArguments(m_Args, expandedArguments);
      isTrue = 
        cmIfCommand::IsTrue(expandedArguments,&errorString,&mf);
      }
    mf.RemoveFunctionBlocker(lff);
    return true;
    }

  // record the command
  m_Functions.push_back(lff);
  
  // always return true
  return true;
}

bool cmWhileFunctionBlocker::
ShouldRemove(const cmListFileFunction& lff, cmMakefile& mf)
{
  if(lff.m_Name == "ENDWHILE")
    {
    if (lff.m_Arguments == m_Args)
      {
      return true;
      }
    }
  return false;
}

void cmWhileFunctionBlocker::
ScopeEnded(cmMakefile &mf) 
{
  cmSystemTools::Error("The end of a CMakeLists file was reached with a WHILE statement that was not closed properly. Within the directory: ", 
                       mf.GetCurrentDirectory());
}

bool cmWhileCommand::InvokeInitialPass(
  const std::vector<cmListFileArgument>& args)
{
  if(args.size() < 1)
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }
  
  // create a function blocker
  cmWhileFunctionBlocker *f = new cmWhileFunctionBlocker();
  f->m_Args = args;
  m_Makefile->AddFunctionBlocker(f);
  
  return true;
}

