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
#include "cmSourceGroupCommand.h"

// cmSourceGroupCommand
bool cmSourceGroupCommand::InitialPass(std::vector<std::string> const& args)
{
  if(args.size() < 1)
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }  

  // Get the source group with the given name.
  cmSourceGroup* sg = m_Makefile->GetSourceGroup(args[0].c_str());
  if(!sg)
    {
    m_Makefile->AddSourceGroup(args[0].c_str(), 0);
    sg = m_Makefile->GetSourceGroup(args[0].c_str());
    }
  
  // Process arguments.
  bool doingFiles = false;
  for(unsigned int i=1; i < args.size(); ++i)
    {
    if(args[i] == "REGULAR_EXPRESSION")
      {
      // Next argument must specify the regex.
      if(i+1 < args.size())
        {
        ++i;
        sg->SetGroupRegex(args[i].c_str());
        }
      else
        {
        this->SetError("REGULAR_EXPRESSION argument given without a regex.");
        return false;
        }
      doingFiles = false;
      }
    else if(args[i] == "FILES")
      {
      // Next arguments will specify files.
      doingFiles = true;
      }
    else if(doingFiles)
      {
      // Convert name to full path and add to the group's list.
      std::string src = args[i].c_str();
      if(!cmSystemTools::FileIsFullPath(src.c_str()))
        {
        src = m_Makefile->GetCurrentDirectory();
        src += "/";
        src += args[i];
        }
      sg->AddGroupFile(src.c_str());
      }
    else
      {
      cmOStringStream err;
      err << "Unknown argument \"" << args[i].c_str() << "\".  "
          << "Perhaps the FILES keyword is missing.\n";
      this->SetError(err.str().c_str());
      return false;
      }
    }
  
  return true;
}
