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
#include "cmAddCustomTargetCommand.h"

// cmAddCustomTargetCommand
bool cmAddCustomTargetCommand::InitialPass(std::vector<std::string> const& args)
{
  bool all = false;
  
  if(args.size() < 2 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }

  // all target option
  std::string arguments;
  std::vector<std::string>::const_iterator s = args.begin();
  ++s; // move past args[0] as it is already to be used
  if (args.size() >= 2)
    {
    if (args[1] == "ALL")
      {
      all = true;
      ++s; // skip all 
      }
    }
  std::string command;
  if(s != args.end())
    {
    command = *s;
    ++s;
    }
  for (;s != args.end(); ++s)
    {
    arguments += cmSystemTools::EscapeSpaces(s->c_str());
    arguments += " ";
    }
  m_Makefile->AddUtilityCommand(args[0].c_str(), 
                                command.c_str(),
                                arguments.c_str(), all);

  return true;
}

