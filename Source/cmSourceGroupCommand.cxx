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
  
  if ( args[1] == "REGULAR_EXPRESSION" && args.size() == 3 )
    {
    m_Makefile->AddSourceGroup(args[0].c_str(), args[2].c_str());
    return true;
    }

  if ( args[1] == "FILES"  )
    {
    cmSourceGroup* sg =  m_Makefile->GetSourceGroup(args[0].c_str());
    if ( !sg )
      {
      m_Makefile->AddSourceGroup(args[0].c_str(), 0);
      sg =  m_Makefile->GetSourceGroup(args[0].c_str());
      }
    unsigned int cc;
    for ( cc = 3; cc < args.size(); cc ++ )
      {
      sg->AddSource(args[cc].c_str(), 0);
      }
    
    return true;
    }
  
  if ( args.size() == 2 )
    {
    m_Makefile->AddSourceGroup(args[0].c_str(), args[1].c_str());
    return true;
    }

  this->SetError("called with incorrect number of arguments");
  return false;
}

