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
#ifndef cmMakeDirectoryCommand_h
#define cmMakeDirectoryCommand_h

#include "cmStandardIncludes.h"
#include "cmCommand.h"

/** \class cmMakeDirectoryCommand
 * \brief Specify auxiliary source code directories.
 *
 * cmMakeDirectoryCommand specifies source code directories
 * that must be built as part of this build process. This directories
 * are not recursively processed like the SUBDIR command (cmSubdirCommand).
 * A side effect of this command is to create a subdirectory in the build
 * directory structure.
 */
class cmMakeDirectoryCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone() 
    {
    return new cmMakeDirectoryCommand;
    }

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  virtual bool InitialPass(std::vector<std::string> const& args);

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() { return "MAKE_DIRECTORY";}
  
  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Create a directory on the file system.";
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "  MAKE_DIRECTORY(directory)\n"
      "Creates the specified directory.  Full paths should be given.  Any "
      "parent directories that do not exist will also be created.  Use with "
      "care.";
    }
  
  cmTypeMacro(cmMakeDirectoryCommand, cmCommand);
};



#endif
