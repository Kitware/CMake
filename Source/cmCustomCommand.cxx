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
#include "cmCustomCommand.h"
#include "cmMakefile.h"

/**
 * The constructor
 */
cmCustomCommand::cmCustomCommand(const char *command,
                                 const char* arguments,
                                 std::vector<std::string> dep,
                                 const char *out):
  m_Command(command),
  m_Arguments(arguments),
  m_Depends(dep)
{
  if (out)
    {
    m_Output = out;
    }
}

cmCustomCommand::cmCustomCommand(const char *command,
                                 const char* arguments):
  m_Command(command),
  m_Arguments(arguments)
{
}

/**
 * Copy constructor.
 */
cmCustomCommand::cmCustomCommand(const cmCustomCommand& r):
  m_Command(r.m_Command),
  m_Arguments(r.m_Arguments),
  m_Comment(r.m_Comment),
  m_Output(r.m_Output),
  m_Depends(r.m_Depends)
{
}

void cmCustomCommand::ExpandVariables(const cmMakefile &mf)
{
  mf.ExpandVariablesInString(m_Command);
  mf.ExpandVariablesInString(m_Arguments);
  mf.ExpandVariablesInString(m_Output);

  for (std::vector<std::string>::iterator i = m_Depends.begin();
       i != m_Depends.end(); ++i)
    {
    mf.ExpandVariablesInString(*i);
    }
}


bool cmCustomCommand::IsEquivalent(const char* command,
                                   const char* args)
{
  if(m_Command != command)
    {
    return false;
    }
  if(m_Arguments != args)
    {
    return false;
    }
  return true;
}
