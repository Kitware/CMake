/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "cmAddCustomTargetCommand.h"

// cmAddCustomTargetCommand
bool cmAddCustomTargetCommand::InitialPass(std::vector<std::string> const& argsIn)
{
  std::vector<std::string> args = argsIn;
  bool all = false;
  
  if(args.size() < 2 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }
  m_Makefile->ExpandVariablesInString(args[0]);

  // all target option
  std::string arguments;
  std::vector<std::string>::iterator s = args.begin();
  ++s; // move past args[0] as it is already to be used
  if (args.size() >= 3)
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
    command = m_Makefile->ExpandVariablesInString(*s);
    ++s;
    }
  for (;s != args.end(); ++s)
    {
    m_Makefile->ExpandVariablesInString(*s);
    arguments += cmSystemTools::EscapeSpaces(s->c_str());
    arguments += " ";
    }
  m_Makefile->AddUtilityCommand(args[0].c_str(), 
                                command.c_str(),
                                arguments.c_str(), all);

  return true;
}

