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
#include "cmGetTargetPropertyCommand.h"

// cmSetTargetPropertyCommand
bool cmGetTargetPropertyCommand::InitialPass(
  std::vector<std::string> const& args)
{
  if(args.size() != 3 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }
  const char* var = args[0].c_str();
  const char* targetName = args[1].c_str();
  cmTargets& targets = m_Makefile->GetTargets(); 
  cmTargets::iterator i = targets.find(targetName);
  if ( i != targets.end())
    {
    cmTarget& target = i->second;
    if ( args[2] == "LOCATION" )
      {
      std::string target_location;
      switch( target.GetType() )
        {
      case cmTarget::STATIC_LIBRARY:
      case cmTarget::MODULE_LIBRARY:
      case cmTarget::SHARED_LIBRARY:
        target_location = m_Makefile->GetSafeDefinition("LIBRARY_OUTPUT_PATH");
        break;
      case cmTarget::EXECUTABLE:
        target_location = m_Makefile->GetSafeDefinition("EXECUTABLE_OUTPUT_PATH");
        break;
      default:
        m_Makefile->AddDefinition(var, "NOTFOUND");
        return true;
        }
      if ( target_location.size() == 0 )
        {
        target_location += m_Makefile->GetCurrentOutputDirectory();
        }
      if ( target_location.size() > 0 )
        {
        target_location += "/";
        }
      const char* cfgid = m_Makefile->GetDefinition("CMAKE_CFG_INTDIR");
      if ( cfgid && strcmp(cfgid, ".") != 0 )
        {
        target_location += cfgid;
        target_location += "/";
        }

      cmLocalGenerator* lg = m_Makefile->GetLocalGenerator();
      target_location += lg->GetFullTargetName(targetName, target);
      m_Makefile->AddDefinition(var, target_location.c_str());
      return true;
      }
    else
      {
      const char *prop = target.GetProperty(args[2].c_str());
      if (prop)
        {
        m_Makefile->AddDefinition(var, prop);
        return true;
        }
      }
    }
  m_Makefile->AddDefinition(var, "NOTFOUND");
  return true;
}

