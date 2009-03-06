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
#include "cmMessageCommand.h"

// cmLibraryCommand
bool cmMessageCommand
::InitialPass(std::vector<std::string> const& args, cmExecutionStatus &)
{
  if(args.size() < 1 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }
  std::string message;
  std::vector<std::string>::const_iterator i = args.begin();

  cmake::MessageType type = cmake::MESSAGE;
  bool status = false;
  if (*i == "SEND_ERROR" || *i == "FATAL_ERROR")
    {
    type = cmake::FATAL_ERROR;
    ++i;
    }
  else if (*i == "WARNING")
    {
    type = cmake::WARNING;
    ++i;
    }
  else if (*i == "AUTHOR_WARNING")
    {
    type = cmake::AUTHOR_WARNING;
    ++i;
    }
  else if (*i == "STATUS")
    {
    status = true;
    ++i;
    }

  for(;i != args.end(); ++i)
    {
    message += *i;
    }

  if (type != cmake::MESSAGE)
    {
    this->Makefile->IssueMessage(type, message.c_str());
    }
  else
    {
    if (status)
      {
      this->Makefile->DisplayStatus(message.c_str(), -1);
      }
    else
      {
      cmSystemTools::Message(message.c_str());
      }
    }
  return true;
}

