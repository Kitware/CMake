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
#include "cmForEachCommand.h"
#include "cmCacheManager.h"

bool cmForEachFunctionBlocker::
IsFunctionBlocked(const char *name, const std::vector<std::string> &args, 
                  cmMakefile &mf) 
{
  // prevent recusion and don't let this blobker blobk its own commands
  if (m_Executing)
    {
    return false;
    }
  
  // at end of for each execute recorded commands
  if (!strcmp(name,"ENDFOREACH") && args[0] == m_Args[0])
    {
    m_Executing = true;
    std::string variable = "${";
    variable += m_Args[0];
    variable += "}"; 
    std::vector<std::string>::const_iterator j = m_Args.begin();
    ++j;
    
    for( ; j != m_Args.end(); ++j)
      {   
      // perform string replace
	for(unsigned int c = 0; c < m_Commands.size(); ++c)
        {
        std::vector<std::string> newArgs;
        for (std::vector<std::string>::const_iterator k = 
               m_CommandArguments[c].begin();
             k != m_CommandArguments[c].end(); ++k)
          {
          std::string tmps = *k;
          cmSystemTools::ReplaceString(tmps, variable.c_str(),
                                       j->c_str());
          newArgs.push_back(tmps);
          }
        // execute command
        mf.ExecuteCommand(m_Commands[c],newArgs);
        }
      }
    return false;
    }

  // record the command
  m_Commands.push_back(name);
  std::vector<std::string> newArgs;
  for(std::vector<std::string>::const_iterator j = args.begin();
      j != args.end(); ++j)
    {   
    newArgs.push_back(*j);
    }
  m_CommandArguments.push_back(newArgs);
  
  // always return true
  return true;
}

bool cmForEachFunctionBlocker::
ShouldRemove(const char *name, const std::vector<std::string> &args, 
             cmMakefile &) 
{
  if (!strcmp(name,"ENDFOREACH") && args[0] == m_Args[0])
    {
    return true;
    }
  return false;
}

void cmForEachFunctionBlocker::
ScopeEnded(cmMakefile &mf) 
{
  cmSystemTools::Error("The end of a CMakeLists file was reached with a FOREACH statement that was not closed properly. Within the directory: ", 
                       mf.GetCurrentDirectory());
}

bool cmForEachCommand::InitialPass(std::vector<std::string> const& argsIn)
{
  std::vector<std::string> args;
  cmSystemTools::ExpandListArguments(argsIn, args);

  if(args.size() < 1)
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }
  
  // create a function blocker
  cmForEachFunctionBlocker *f = new cmForEachFunctionBlocker();
  for(std::vector<std::string>::const_iterator j = args.begin();
      j != args.end(); ++j)
    {   
    f->m_Args.push_back(*j);
    }
  m_Makefile->AddFunctionBlocker(f);
  
  return true;
}

