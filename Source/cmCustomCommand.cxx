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
cmCustomCommand::cmCustomCommand(const char *src, const char *command,
                                 const char* arguments,
                                 std::vector<std::string> dep,
                                 std::vector<std::string> out):
  m_Source(src),
  m_Command(command),
  m_Arguments(arguments),
  m_Depends(dep),
  m_Outputs(out)
{
}


/**
 * Copy constructor.
 */
cmCustomCommand::cmCustomCommand(const cmCustomCommand& r):
  m_Source(r.m_Source),
  m_Command(r.m_Command),
  m_Arguments(r.m_Arguments),
  m_Depends(r.m_Depends),
  m_Outputs(r.m_Outputs)
{
}

void cmCustomCommand::ExpandVariables(const cmMakefile &mf)
{
  mf.ExpandVariablesInString(m_Source);
  mf.ExpandVariablesInString(m_Command);
  mf.ExpandVariablesInString(m_Arguments);

  for (std::vector<std::string>::iterator i = m_Depends.begin();
       i != m_Depends.end(); ++i)
    {
    mf.ExpandVariablesInString(*i);
    }
  for (std::vector<std::string>::iterator i = m_Outputs.begin();
       i != m_Outputs.end(); ++i)
    {
    mf.ExpandVariablesInString(*i);
    }  
}

