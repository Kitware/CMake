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
#include "cmBuildNameCommand.h"

#include <cmsys/RegularExpression.hxx>

// cmBuildNameCommand
bool cmBuildNameCommand::InitialPass(std::vector<std::string> const& args)
{
  if(args.size() < 1 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }
  const char* cacheValue = m_Makefile->GetDefinition(args[0].c_str());
  if(cacheValue)
    {
    // do we need to correct the value? 
    cmsys::RegularExpression reg("[()/]");
    if (reg.find(cacheValue))
      {
      std::string cv = cacheValue;
      cmSystemTools::ReplaceString(cv,"/", "_");
      cmSystemTools::ReplaceString(cv,"(", "_");
      cmSystemTools::ReplaceString(cv,")", "_");
      m_Makefile->AddCacheDefinition(args[0].c_str(),
                                     cv.c_str(),
                                     "Name of build.",
                                     cmCacheManager::STRING);
      }
    return true;
    }

  
  std::string buildname = "WinNT";
  if(m_Makefile->GetDefinition("UNIX"))
    {
    buildname = "";
    cmSystemTools::RunSingleCommand("uname -a", &buildname);
    if(buildname.length())
      {
      std::string RegExp = "([^ ]*) [^ ]* ([^ ]*) ";
      cmsys::RegularExpression reg( RegExp.c_str() );
      if(reg.find(buildname.c_str()))
        {
        buildname = reg.match(1) + "-" + reg.match(2);
        }
      }
    }
  std::string compiler = "${CMAKE_CXX_COMPILER}";
  m_Makefile->ExpandVariablesInString ( compiler );
  buildname += "-";
  buildname += cmSystemTools::GetFilenameName(compiler);
  cmSystemTools::ReplaceString(buildname,
                               "/", "_");
  cmSystemTools::ReplaceString(buildname,
                               "(", "_");
  cmSystemTools::ReplaceString(buildname,
                               ")", "_");
  
  m_Makefile->AddCacheDefinition(args[0].c_str(),
                                 buildname.c_str(),
                                 "Name of build.",
                                 cmCacheManager::STRING);
  return true;
}

