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

#include "cmCTestGenericHandler.h"

cmCTestGenericHandler::cmCTestGenericHandler()
{
  m_Verbose = false;
  m_CTest = 0;
}

cmCTestGenericHandler::~cmCTestGenericHandler()
{
}

void cmCTestGenericHandler::SetOption(const char* op, const char* value)
{
  if ( !op )
    {
    return;
    }
  if ( !value )
    {
    cmCTestGenericHandler::t_StringToString::iterator remit 
      = m_Options.find(op);
    if ( remit != m_Options.end() )
      {
      m_Options.erase(remit);
      }
    return;
    }

  m_Options[op] = value;
}

const char* cmCTestGenericHandler::GetOption(const char* op)
{
  cmCTestGenericHandler::t_StringToString::iterator remit 
    = m_Options.find(op);
  if ( remit == m_Options.end() )
    {
    return 0;
    }
  return remit->second.c_str();
}

