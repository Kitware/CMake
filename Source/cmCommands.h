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
#ifndef cmCommands_h
#define cmCommands_h
#include "cmStandardIncludes.h"

class cmCommand;
/**
 * Global function to return all compiled in commands.
 * To add a new command edit cmCommands.cxx or cmBootstrapCommands.cxx
 * and add your command.
 * It is up to the caller to delete the commands created by this
 * call.
 */
void GetBootstrapCommands(std::list<cmCommand*>& commands);
void GetPredefinedCommands(std::list<cmCommand*>& commands);


#endif
