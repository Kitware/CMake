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
#include "cmSetDirectoryPropertiesCommand.h"

#include "cmake.h"

// cmSetDirectoryPropertiesCommand
bool cmSetDirectoryPropertiesCommand::InitialPass(
  std::vector<std::string> const& args)
{
  if(args.size() < 1 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }

  std::vector<std::string>::const_iterator ait;
  for ( ait = args.begin()+1; 
    ait != args.end(); 
    ait += 2 )
    {
    if ( ait +1 == args.end() )
      {
      this->SetError("Wrong number of arguments");
      return false;
      }
    const std::string& prop = *ait;
    const std::string& value = *(ait+1);
    if ( prop == "VARIABLES" )
      {
      this->SetError("Variables and cache variables should be set using SET command");
      return false;
      }
    else if ( prop == "MACROS" )
      {
      this->SetError("Commands and macros cannot be set using SET_CMAKE_PROPERTIES");
      return false;
      }
    else if ( prop == "INCLUDE_DIRECTORIES" )
      {
      std::vector<std::string> varArgsExpanded;
      cmSystemTools::ExpandListArgument(value, varArgsExpanded);
      m_Makefile->SetIncludeDirectories(varArgsExpanded);
      }
    else if ( prop == "LINK_DIRECTORIES" )
      {
      std::vector<std::string> varArgsExpanded;
      cmSystemTools::ExpandListArgument(value, varArgsExpanded);
      m_Makefile->SetLinkDirectories(varArgsExpanded);
      }
    else if ( prop == "INCLUDE_REGULAR_EXPRESSION" )
      {
      m_Makefile->SetIncludeRegularExpression(value.c_str());
      }
    else
      {
      if ( prop == "ADDITIONAL_MAKE_CLEAN_FILES" )
        {
        // This property is not inherrited
        if ( strcmp(m_Makefile->GetCurrentDirectory(), m_Makefile->GetStartDirectory()) != 0 )
          {
          continue;
          }
        }
      m_Makefile->SetProperty(prop.c_str(), value.c_str());
      }
    }
  
  return true;
}

