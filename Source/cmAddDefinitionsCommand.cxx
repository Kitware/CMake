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
#include "cmAddDefinitionsCommand.h"

// cmAddDefinitionsCommand
bool cmAddDefinitionsCommand::InitialPass(std::vector<std::string> const& argsIn)
{
  // it is OK to have no arguments
  if(argsIn.size() < 1 )
    {
    return true;
    }
  std::vector<std::string> args;
  cmSystemTools::ExpandListArguments(argsIn, args);

  for(std::vector<std::string>::const_iterator i = args.begin();
      i != args.end(); ++i)
    {
    m_Makefile->AddDefineFlag(i->c_str());
    }
  return true;
}

