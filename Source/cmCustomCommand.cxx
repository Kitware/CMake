/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


  Copyright (c) 2000 National Library of Medicine
  All rights reserved.

  See COPYRIGHT.txt for copyright details.

=========================================================================*/
#include "cmCustomCommand.h"
#include "cmMakefile.h"

/**
 * The constructor
 */
cmCustomCommand::cmCustomCommand(const char *src, const char *command,
                                 std::vector<std::string> dep,
                                 std::vector<std::string> out):
  m_Source(src),
  m_Command(command),
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
  m_Depends(r.m_Depends),
  m_Outputs(r.m_Outputs)
{
}

void cmCustomCommand::ExpandVariables(const cmMakefile &mf)
{
  mf.ExpandVariablesInString(m_Source);
  mf.ExpandVariablesInString(m_Command);

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

