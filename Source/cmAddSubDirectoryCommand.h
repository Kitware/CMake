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
#ifndef cmAddSubDirectoryCommand_h
#define cmAddSubDirectoryCommand_h

#include "cmCommand.h"

/** \class cmAddSubDirectoryCommand
 * \brief Specify a subdirectory to build
 *
 * cmAddSubDirectoryCommand specifies a subdirectory to process
 * by CMake. CMake will descend
 * into the specified source directory and process any CMakeLists.txt found.
 */
class cmAddSubDirectoryCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone() 
    {
    return new cmAddSubDirectoryCommand;
    }

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  virtual bool InitialPass(std::vector<std::string> const& args);

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() { return "ADD_SUBDIRECTORY";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Add a subdirectory to the build.";
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "  ADD_SUBDIRECTORY(binary_dir [source_dir]\n"
      "                   [EXCLUDE_FROM_ALL] [PREORDER])\n"
      "Add a subdirectory to the build. The binary_dir specifies the "
      "directory in which to store the build files. If it is a relative "
      "path it will be evaluated with respect to the current output "
      "directory (the typical usage), but it may also be an absolute path. "
      "The source_dir specifies the directory in which to find the source "
      "files. If it is a relative path it will be evaluated with respect "
      "to the current source directory, but it may also be an absolute "
      "path. If source_dir is not specified the value of binary_dir, "
      "before expanding any relative path, will be used (the typical usage). "
      "The CMakeLists.txt file in the specified source directory will "
      "be processed immediately by CMake before processing in the current "
      "input file continues beyond this command.\n"

      "The PREORDER flag tells recursive makefile generators to build "
      "targets in the subdirectory before those in the current directory. "
      "If this flag is not specified then the targets in the current "
      "directory are always built first. The PREORDER flag has no meaning "
      "for global build system generators.\n"

      "If the EXCLUDE_FROM_ALL argument is provided then this subdirectory "
      "will not be included in build by default. Users will have to "
      "explicitly start a build in the generated output directory. "
      "This is useful for having cmake create a build system for a "
      "set of examples in a project. One would want cmake to generate "
      "a single build system for all the examples, but one may not want "
      "the targets to show up in the main build system.";
    }
  
  cmTypeMacro(cmAddSubDirectoryCommand, cmCommand);
};



#endif
