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
#include "cmForEachCommand.h"

bool cmForEachFunctionBlocker::
IsFunctionBlocked(const cmListFileFunction& lff, cmMakefile &mf) 
{
  // Prevent recusion and don't let this blobker block its own
  // commands.
  if (m_Executing)
    {
    return false;
    }
  
  // at end of for each execute recorded commands
  if (lff.m_Name == "ENDFOREACH")
    {
    std::vector<std::string> expandedArguments;
    mf.ExpandArguments(lff.m_Arguments, expandedArguments);
    if(!expandedArguments.empty() && (expandedArguments[0] == m_Args[0]))
      {
      m_Executing = true;
      std::string variable = "${";
      variable += m_Args[0];
      variable += "}"; 
      std::vector<std::string>::const_iterator j = m_Args.begin();
      ++j;
      
      for( ; j != m_Args.end(); ++j)
        {   
        // Invoke all the functions that were collected in the block.
        for(unsigned int c = 0; c < m_Functions.size(); ++c)
          {
          // Replace the loop variable and then invoke the command.
          cmListFileFunction newLFF;
          newLFF.m_Name = m_Functions[c].m_Name;
          for (std::vector<cmListFileArgument>::const_iterator k = 
                 m_Functions[c].m_Arguments.begin();
               k != m_Functions[c].m_Arguments.end(); ++k)
            {
            std::string tmps = k->Value;
            cmSystemTools::ReplaceString(tmps, variable.c_str(), j->c_str());
            cmListFileArgument arg(tmps, k->Quoted);
            newLFF.m_Arguments.push_back(arg);
            }
          mf.ExecuteCommand(newLFF);
          }
        }
      return false;
      }
    }

  // record the command
  m_Functions.push_back(lff);
  
  // always return true
  return true;
}

bool cmForEachFunctionBlocker::
ShouldRemove(const cmListFileFunction& lff, cmMakefile& mf)
{
  if(lff.m_Name == "ENDFOREACH")
    {
    std::vector<std::string> expandedArguments;
    mf.ExpandArguments(lff.m_Arguments, expandedArguments);
    if(!expandedArguments.empty() && (expandedArguments[0] == m_Args[0]))
      {
      return true;
      }
    }
  return false;
}

void cmForEachFunctionBlocker::
ScopeEnded(cmMakefile &mf) 
{
  cmSystemTools::Error("The end of a CMakeLists file was reached with a FOREACH statement that was not closed properly. Within the directory: ", 
                       mf.GetCurrentDirectory());
}

bool cmForEachCommand::InitialPass(std::vector<std::string> const& args)
{
  if(args.size() < 1)
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }
  
  // create a function blocker
  cmForEachFunctionBlocker *f = new cmForEachFunctionBlocker();
  f->m_Args = args;
  m_Makefile->AddFunctionBlocker(f);
  
  return true;
}

