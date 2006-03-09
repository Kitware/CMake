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
#include "cmCTestTestCommand.h"

#include "cmCTest.h"
#include "cmCTestGenericHandler.h"

cmCTestTestCommand::cmCTestTestCommand()
{
  m_Arguments[ctt_START] = "START";
  m_Arguments[ctt_END] = "END";
  m_Arguments[ctt_STRIDE] = "STRIDE";
  m_Arguments[ctt_LAST] = 0;
  m_Last = ctt_LAST;
}

cmCTestGenericHandler* cmCTestTestCommand::InitializeHandler()
{
  const char* ctestTimeout = m_Makefile->GetDefinition("CTEST_TEST_TIMEOUT");
  double timeout = m_CTest->GetTimeOut();
  if ( ctestTimeout )
    {
    timeout = atof(ctestTimeout);
    }
  else
    {
    if ( timeout <= 0 )
      {
      // By default use timeout of 10 minutes
      timeout = 600;
      }
    }
  m_CTest->SetTimeOut(timeout);
  cmCTestGenericHandler* handler = this->InitializeActualHandler();
  if ( m_Values[ctt_START] || m_Values[ctt_END] || m_Values[ctt_STRIDE] )
    {
    cmOStringStream testsToRunString;
    if ( m_Values[ctt_START] )
      {
      testsToRunString << m_Values[ctt_START];
      }
    testsToRunString << ",";
    if ( m_Values[ctt_END] )
      {
      testsToRunString << m_Values[ctt_END];
      }
    testsToRunString << ",";
    if ( m_Values[ctt_STRIDE] )
      {
      testsToRunString << m_Values[ctt_STRIDE];
      }
    handler->SetOption("TestsToRunInformation",
      testsToRunString.str().c_str());
    }
  return handler;
}

cmCTestGenericHandler* cmCTestTestCommand::InitializeActualHandler()
{
  return m_CTest->GetInitializedHandler("test");
}

