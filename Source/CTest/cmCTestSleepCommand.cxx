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
#include "cmCTestSleepCommand.h"

#include "cmCTestScriptHandler.h"
#include <stdlib.h> // required for atoi

bool cmCTestSleepCommand::InitialPass(
  std::vector<std::string> const& args)
{
  if (args.size() < 1)
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }

  // sleep for specified seconds
  unsigned int time1 = atoi(args[0].c_str());
  if(args.size() == 1 )
    {
    cmCTestScriptHandler::SleepInSeconds(time1);
    // update the elapsed time since it could have slept for a while
    m_CTestScriptHandler->UpdateElapsedTime();
    return true;
    }

  // sleep up to a duration
  if(args.size() == 3 )
    {
    unsigned int duration = atoi(args[1].c_str());
    unsigned int time2 = atoi(args[2].c_str());
    if (time1 + duration > time2)
      {
      duration = (time1 + duration - time2);
      cmCTestScriptHandler::SleepInSeconds(duration);
      // update the elapsed time since it could have slept for a while
      m_CTestScriptHandler->UpdateElapsedTime();
      }
    return true;
    }
  
  this->SetError("called with incorrect number of arguments");
  return false;
}


