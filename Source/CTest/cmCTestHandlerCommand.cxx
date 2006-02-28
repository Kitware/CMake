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
#include "cmCTestHandlerCommand.h"

#include "cmCTest.h"
#include "cmCTestGenericHandler.h"

cmCTestHandlerCommand::cmCTestHandlerCommand()
{
  const size_t INIT_SIZE = 100;
  size_t cc;
  m_Arguments.reserve(INIT_SIZE);
  for ( cc = 0; cc < INIT_SIZE; ++ cc )
    {
    m_Arguments.push_back(0);
    }
  m_Arguments[ct_RETURN_VALUE] = "RETURN_VALUE";
  m_Arguments[ct_SOURCE] = "SOURCE";
  m_Arguments[ct_BUILD] = "BUILD";
  m_Arguments[ct_SUBMIT_INDEX] = "SUBMIT_INDEX";
  m_Last = ct_LAST;
}

bool cmCTestHandlerCommand::InitialPass(
  std::vector<std::string> const& args)
{
  if ( !this->ProcessArguments(args, m_Last, &*m_Arguments.begin(), m_Values) )
    {
    return false;
    }

  cmCTestGenericHandler* handler = this->InitializeHandler();
  if ( !handler )
    {
    this->SetError("internal CTest error. Cannot instantiate test handler");
    return false;
    }

  if ( m_Values[ct_BUILD] )
    {
    m_CTest->SetCTestConfiguration("BuildDirectory", m_Values[ct_BUILD]);
    }
  if ( m_Values[ct_SUBMIT_INDEX] )
    {
    if ( m_CTest->GetDartVersion() <= 1 )
      {
      cmCTestLog(m_CTest, ERROR_MESSAGE, "Dart before version 2.0 does not support collecting submissions." << std::endl
        << "Please upgrade the server to Dart 2 or higher, or do not use SUBMIT_INDEX." << std::endl);
      }
    else
      {
      handler->SetSubmitIndex(atoi(m_Values[ct_SUBMIT_INDEX]));
      }
    }


  std::string current_dir = cmSystemTools::GetCurrentWorkingDirectory();
  cmSystemTools::ChangeDirectory(m_CTest->GetCTestConfiguration("BuildDirectory").c_str());
  int res = handler->ProcessHandler();
  if ( m_Values[ct_RETURN_VALUE] && *m_Values[ct_RETURN_VALUE])
    {
    cmOStringStream str;
    str << res;
    m_Makefile->AddDefinition(m_Values[ct_RETURN_VALUE], str.str().c_str());
    }
  cmSystemTools::ChangeDirectory(current_dir.c_str());
  return true;
}

bool cmCTestHandlerCommand::ProcessArguments(std::vector<std::string> const& args,
  int last, const char** strings, std::vector<const char*>& values)
{
  int state = 0;
  int cc;
  values.resize(last);
  for ( cc = 0; cc < last; ++ cc )
    {
    values[cc] = 0;
    }

  for(size_t i=0; i < args.size(); ++i)
    {
    if ( state > 0 && state < last )
      {
      values[state] = args[i].c_str();
#undef cerr
      cmCTestLog(m_CTest, DEBUG, "Set " << strings[state] << " to " << args[i].c_str() << std::endl);
      state = 0;
      }
    else
      {
      bool found = false;
      for ( cc = 0; cc < last; ++ cc )
        {
        if ( strings[cc] && args[i] == strings[cc] )
          {
          state = cc;
          if ( values[state] )
            {
            cmOStringStream ostr;
            ostr << "called with incorrect number of arguments. " << strings[state] << " specified twice.";
            this->SetError(ostr.str().c_str());
            return false;
            }
          found = true;
          break;
          }
        }
      if ( !found )
        {
        cmOStringStream str;
        str << "called with incorrect number of arguments. Extra argument is: " << args[i].c_str() << ".";
        this->SetError(str.str().c_str());
        return false;
        }
      }
    }
  return true;
}
