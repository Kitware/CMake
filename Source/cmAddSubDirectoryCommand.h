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
      "  ADD_SUBDIRECTORY(binary_dir [source_dir] [EXCLUDE_FROM_ALL] [PREORDER] )\n"
      "Add a subdirectory to the build. The binary_dir specified can be "
      "relative to the current otuput directory (the typical usage) or "
      "absolute. If the source dir is not specified it "
      "will be set to the same value as the binary_dir. "
      "This will cause any CMakeLists.txt files in the sub directories "
      "to be processed by CMake.  If the PREORDER flag "
      "is specified then this directory will be traversed first by makefile "
      "builds, the PRORDER flag has no effect on IDE projects. " 
      "If the EXCLUDE_FROM_ALL argument is provided then this subdirectory "
      "will not be included in the top level makefile or project file.  "
      "This is useful for having cmake create makefiles or projects for a "
      "set of examples in a project. You would want cmake to generated "
      "makefiles or project files for all the examples at the same time, "
      "but you would not want them to show up in the top level project or "
      "be built each time make is run from the top.";
    }
  
  cmTypeMacro(cmAddSubDirectoryCommand, cmCommand);
};



#endif
