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

//----------------------------------------------------------------------------
cmCustomCommand::cmCustomCommand()
{
  m_Used = false;
}

//----------------------------------------------------------------------------
cmCustomCommand::cmCustomCommand(const cmCustomCommand& r):
  m_Output(r.m_Output),
  m_Depends(r.m_Depends),
  m_CommandLines(r.m_CommandLines),
  m_Comment(r.m_Comment)
{
  m_Used = false;
}

//----------------------------------------------------------------------------
cmCustomCommand::cmCustomCommand(const char* output,
                                 const std::vector<std::string>& depends,
                                 const cmCustomCommandLines& commandLines,
                                 const char* comment):
  m_Output(output?output:""),
  m_Depends(depends),
  m_CommandLines(commandLines),
  m_Comment(comment?comment:"")
{
  m_Used = false;
}

//----------------------------------------------------------------------------
const char* cmCustomCommand::GetOutput() const
{
  return m_Output.c_str();
}

//----------------------------------------------------------------------------
const std::vector<std::string>& cmCustomCommand::GetDepends() const
{
  return m_Depends;
}

//----------------------------------------------------------------------------
const cmCustomCommandLines& cmCustomCommand::GetCommandLines() const
{
  return m_CommandLines;
}

//----------------------------------------------------------------------------
const char* cmCustomCommand::GetComment() const
{
  return m_Comment.c_str();
}
