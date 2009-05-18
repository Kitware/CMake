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
#include "cmFindFileCommand.h"
#include "cmSystemTools.h"

cmFindFileCommand::cmFindFileCommand()
{
  this->IncludeFileInPath = true;
  cmSystemTools::ReplaceString(this->GenericDocumentation,
                               "find_path", "find_file");
  cmSystemTools::ReplaceString(this->GenericDocumentation,
                               "directory containing the named file", 
                               "full path to named file");
  cmSystemTools::ReplaceString(this->GenericDocumentation,
                                "file in a directory", "full path to a file");
}
