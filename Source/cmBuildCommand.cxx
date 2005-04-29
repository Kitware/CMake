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

#include "cmLocalGenerator.h"
#include "cmGlobalGenerator.h"

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
  std::string makecommand;
  std::string makeprogram = args[1];
  std::string makecmd = m_Makefile->GetLocalGenerator()->GetGlobalGenerator()->GenerateBuildCommand(
    makeprogram.c_str(), m_Makefile->GetProjectName(), 0, "Release", true);
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
  else if (makeprogram.find("xcodebuild") != std::string::npos)
    {
    makecommand += makeprogram;
    }
  else
    {
    makecommand = makeprogram;
    makecommand += " -i";
    }
  std::cerr << "-- Compare: " << makecommand.c_str() << " and " << makecmd.c_str() << ": " << (makecmd == makecommand) << std::endl;
  if(cacheValue)
    {
    return true;
    }
  m_Makefile->AddCacheDefinition(define,
                                 makecommand.c_str(),
                                 "Command used to build entire project "
                                 "from the command line.",
                                 cmCacheManager::STRING);
  return true;
}

