/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmSubdirCommand_h
#define cmSubdirCommand_h

#include "cmCommand.h"

/** \class cmSubdirCommand
 * \brief Specify a list of subdirectories to build.
 *
 * cmSubdirCommand specifies a list of subdirectories to process
 * by CMake. For each subdirectory listed, CMake will descend
 * into that subdirectory and process any CMakeLists.txt found.
 */
class cmSubdirCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone() 
    {
    return new cmSubdirCommand;
    }

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  virtual bool InitialPass(std::vector<std::string> const& args,
                           cmExecutionStatus &status);

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() { return "subdirs";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Deprecated. Use the add_subdirectory() command instead.";
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "Add a list of subdirectories to the build.\n"
      "  subdirs(dir1 dir2 ..."
      "[EXCLUDE_FROM_ALL exclude_dir1 exclude_dir2 ...]\n"
      "          [PREORDER] )\n"
      "Add a list of subdirectories to the build. The add_subdirectory "
      "command should be used instead of subdirs although subdirs will "
      "still work. "
      "This will cause any CMakeLists.txt files in the sub directories "
      "to be processed by CMake.  Any directories after the PREORDER flag "
      "are traversed first by makefile builds, the PREORDER flag has no "
      "effect on IDE projects. " 
      " Any directories after the EXCLUDE_FROM_ALL marker "
      "will not be included in the top level makefile or project file. "
      "This is useful for having CMake create makefiles or projects for "
      "a set of examples in a project. You would want CMake to "
      "generate makefiles or project files for all the examples at "
      "the same time, but you would not want them to show up in the "
      "top level project or be built each time make is run from the top.";
    }
  
  /** This command is kept for compatibility with older CMake versions. */
  virtual bool IsDiscouraged()
    {
    return true;
    }

  cmTypeMacro(cmSubdirCommand, cmCommand);
};



#endif
