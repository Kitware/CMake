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
#include "cmBuildCommand.h"

// cmBuildCommand
bool cmBuildCommand::InitialPass(std::vector<std::string> const& args)
{
  if(args.size() < 2 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }
  const char* define = args[0].c_str();
  const char* cacheValue
    = m_Makefile->GetDefinition(define);
  if(cacheValue)
    {
    return true;
    }
  std::string makecommand;
  std::string makeprogram = args[1];
  if(makeprogram.find("msdev") != std::string::npos ||
     makeprogram.find("MSDEV") != std::string::npos )
    {
    makecommand = "\"";
    makecommand += makeprogram;
    makecommand += "\"";
    makecommand += " ";
    makecommand += m_Makefile->GetProjectName();
    makecommand += ".dsw /MAKE \"ALL_BUILD - Release\" ";
    }
  else if (makeprogram.find("devenv") != std::string::npos ||
           makeprogram.find("DEVENV") != std::string::npos )
    {
    makecommand = "\"";
    makecommand += makeprogram;
    makecommand += "\"";
    makecommand += " ";
    makecommand += m_Makefile->GetProjectName();
    makecommand += ".sln /build Release /project ALL_BUILD";
    }
  else if(m_Makefile->GetDefinition("BORLAND"))
    {
    makecommand = makeprogram;
    makecommand += " -i";
    }
  else
    {
    makecommand = makeprogram;
    makecommand += " -i";
    }
  m_Makefile->AddCacheDefinition(define,
                                 makecommand.c_str(),
                                 "Command used to build entire project "
                                 "from the command line.",
                                 cmCacheManager::STRING);
  return true;
}

