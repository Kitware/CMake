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
#include "cmMessageCommand.h"
#include "cmCacheManager.h"

// cmLibraryCommand
bool cmMessageCommand::InitialPass(std::vector<std::string> const& args)
{
  if(args.size() < 1 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }

  std::string message;
  std::vector<std::string>::const_iterator i = args.begin();

  bool send_error = false;
  if (*i == "SEND_ERROR")
    {
    send_error = true;
    ++i;
    }

  for(;i != args.end(); ++i)
    {
    message += *i;
    }

  if (send_error)
    {
    cmSystemTools::Error(message.c_str());
    }
  else
    {
    cmSystemTools::Message(message.c_str());
    }

  return true;
}

