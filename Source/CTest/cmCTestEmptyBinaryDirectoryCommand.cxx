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
#include "cmCTestEmptyBinaryDirectoryCommand.h"

#include "cmCTestScriptHandler.h"

bool cmCTestEmptyBinaryDirectoryCommand::InitialPass(
  std::vector<std::string> const& args)
{
  if(args.size() != 1 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }

  cmCTestScriptHandler::EmptyBinaryDirectory(args[0].c_str());
  
  return true;
}


