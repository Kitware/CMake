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
#ifndef cmAuxSourceDirectoryCommand_h
#define cmAuxSourceDirectoryCommand_h

#include "cmCommand.h"

/** \class cmAuxSourceDirectoryCommand
 * \brief Specify auxiliary source code directories.
 *
 * cmAuxSourceDirectoryCommand specifies source code directories
 * that must be built as part of this build process. This directories
 * are not recursively processed like the SUBDIR command (cmSubdirCommand).
 * A side effect of this command is to create a subdirectory in the build
 * directory structure.
 */
class cmAuxSourceDirectoryCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone() 
    {
    return new cmAuxSourceDirectoryCommand;
    }

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  virtual bool InitialPass(std::vector<std::string> const& args);

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() { return "AUX_SOURCE_DIRECTORY";}
  
  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Find all source files in a directory.";
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "  AUX_SOURCE_DIRECTORY(dir VARIABLE)\n"
      "Collects the names of all the source files in the specified "
      "directory and stores the list in the variable provided.  This command "
      "is intended to be used by projects that use explicit template "
      "instantiation.  Template instantiation files can be stored in a "
      "\"Templates\" subdirectory and collected automatically using this "
      "command to avoid manually listing all instantiations.\n"
      "It is tempting to use this command to avoid writing the list of source "
      "files for a library or executable target.  While this seems to work, "
      "there is no way for CMake to generate a build system that knows when a "
      "new source file has been added.  Normally the generated build system "
      "knows when it needs to rerun CMake because the CMakeLists.txt file "
      "is modified to add a new source.  When the source is just added to "
      "the directory without modifying this file, one would have to manually "
      "rerun CMake to generate a build system incorporating the new file.";
    }
  
  cmTypeMacro(cmAuxSourceDirectoryCommand, cmCommand);
};



#endif
