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

bool cmCTestRunScriptCommand
::InitialPass(std::vector<std::string> const& args, cmExecutionStatus &)
{
  if(args.size() < 1 )
    {
    this->CTestScriptHandler->RunCurrentScript();
    return true;
    }

  bool np = false;
  unsigned int i = 0;
  if (args[i] == "NEW_PROCESS")
    {
    np = true;
    i++;
    }
  int start = i;
  // run each script
  std::string returnVariable;
  for (i = start; i < args.size(); ++i)
    {
    if(args[i] == "RETURN_VALUE")
      {
      ++i;
      if(i < args.size())
        {
        returnVariable = args[i];
        }
      }
    }
  for (i = start; i < args.size(); ++i)
    {
    if(args[i] == "RETURN_VALUE")
      {
      ++i;
      }
    else
      {
      int ret;
      cmCTestScriptHandler::RunScript(this->CTest, args[i].c_str(), !np,
        &ret);
      cmOStringStream str;
      str << ret;
      this->Makefile->AddDefinition(returnVariable.c_str(), str.str().c_str());
      }
    }
  return true;
}


