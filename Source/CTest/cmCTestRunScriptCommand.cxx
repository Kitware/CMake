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
#include "cmCTestRunScriptCommand.h"

#include "cmCTestScriptHandler.h"

bool cmCTestRunScriptCommand::InitialPass(std::vector<std::string> const& args)
{
  if(args.size() < 1 )
    {
    m_CTestScriptHandler->RunCurrentScript();
    return true;
    }

  // run each script
  unsigned int i;
  for (i = 0; i < args.size(); ++i)
    {
    cmCTestScriptHandler::RunScript(m_CTest, args[i].c_str());
    }
  return true;
}


