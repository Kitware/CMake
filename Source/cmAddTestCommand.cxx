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
#include "cmAddTestCommand.h"

#include "cmTest.h"


// cmExecutableCommand
bool cmAddTestCommand
::InitialPass(std::vector<std::string> const& args, cmExecutionStatus &)
{
  // First argument is the name of the test Second argument is the name of
  // the executable to run (a target or external program) Remaining arguments
  // are the arguments to pass to the executable
  if(args.size() < 2 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }
  
  // store the arguments for the final pass
  // also expand any CMake variables

  std::vector<cmStdString> arguments;
  std::vector<std::string>::const_iterator it;
  for ( it = args.begin() + 2; it != args.end(); ++ it )
    {
    arguments.push_back(*it);
    }

  cmTest* test = this->Makefile->CreateTest(args[0].c_str());
  test->SetCommand(args[1].c_str());
  test->SetArguments(arguments);

  return true;
}
