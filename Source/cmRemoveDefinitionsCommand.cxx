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
#include "cmRemoveDefinitionsCommand.h"

// cmRemoveDefinitionsCommand
bool cmRemoveDefinitionsCommand::InitialPass(std::vector<std::string> const& args)
{
  // it is OK to have no arguments
  if(args.size() < 1 )
    {
    return true;
    }

  for(std::vector<std::string>::const_iterator i = args.begin();
      i != args.end(); ++i)
    {
    m_Makefile->RemoveDefineFlag(i->c_str());
    }
  return true;
}

