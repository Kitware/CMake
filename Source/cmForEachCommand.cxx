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
      
      std::string tmps;
      cmListFileArgument arg;
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
            tmps = k->Value;
            cmSystemTools::ReplaceString(tmps, variable.c_str(), j->c_str());
            arg.Value = tmps;
            arg.Quoted = k->Quoted;
            arg.FilePath = k->FilePath;
            arg.Line = k->Line;
            newLFF.m_Arguments.push_back(arg);
            }
          mf.ExecuteCommand(newLFF);
          }
        }
      mf.RemoveFunctionBlocker(lff);
      return true;
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
  if ( args.size() > 1 )
    {
    if ( args[1] == "RANGE" )
      {
      int start = 0;
      int stop = 0;
      int step = 0;
      if ( args.size() == 3 )
        {
        stop = atoi(args[2].c_str());
        }
      if ( args.size() == 4 )
        {
        start = atoi(args[2].c_str());
        stop = atoi(args[3].c_str());
        }
      if ( args.size() == 5 )
        {
        start = atoi(args[2].c_str());
        stop = atoi(args[3].c_str());
        step = atoi(args[4].c_str());
        }
      if ( step == 0 )
        {
        if ( start > stop )
          {
          step = -1;
          }
        else
          {
          step = 1;
          }
        }
      if ( 
        (start > stop && step > 0) ||
        (start < stop && step < 0) ||
        step == 0
        )
        {
        cmOStringStream str;
        str << "called with incorrect range specification: start ";
        str << start << ", stop " << stop << ", step " << step;
        this->SetError(str.str().c_str());
        return false;
        }
      std::vector<std::string> range;
      char buffer[100];
      range.push_back(args[0]);
      int cc;
      for ( cc = start; ; cc += step )
        {
        if ( (step > 0 && cc > stop) || (step < 0 && cc < stop) )
          {
          break;
          }
        sprintf(buffer, "%d", cc);
        range.push_back(buffer);
        if ( cc == stop )
          {
          break;
          }
        }
      f->m_Args = range;
      }
    else
      {
      f->m_Args = args;
      }
    }
  else
    {
    f->m_Args = args;
    }
  m_Makefile->AddFunctionBlocker(f);
  
  return true;
}

