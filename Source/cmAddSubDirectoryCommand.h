/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
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
  virtual bool InitialPass(std::vector<std::string> const& args,
                           cmExecutionStatus &status);

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() const { return "add_subdirectory";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() const
    {
    return "Add a subdirectory to the build.";
    }

  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation() const
    {
    return
      "  add_subdirectory(source_dir [binary_dir] \n"
      "                   [EXCLUDE_FROM_ALL])\n"
      "Add a subdirectory to the build. The source_dir specifies the "
      "directory in which the source CMakeLists.txt and code files are "
      "located. If it is a relative "
      "path it will be evaluated with respect to the current "
      "directory (the typical usage), but it may also be an absolute path. "
      "The binary_dir specifies the directory in which to place the output "
      "files. If it is a relative path it will be evaluated with respect "
      "to the current output directory, but it may also be an absolute "
      "path. If binary_dir is not specified, the value of source_dir, "
      "before expanding any relative path, will be used (the typical usage). "
      "The CMakeLists.txt file in the specified source directory will "
      "be processed immediately by CMake before processing in the current "
      "input file continues beyond this command.\n"

      "If the EXCLUDE_FROM_ALL argument is provided then targets in the "
      "subdirectory will not be included in the ALL target of the parent "
      "directory by default, and will be excluded from IDE project files.  "
      "Users must explicitly build targets in the subdirectory.  "
      "This is meant for use when the subdirectory contains a separate part "
      "of the project that is useful but not necessary, such as a set of "
      "examples.  "
      "Typically the subdirectory should contain its own project() command "
      "invocation so that a full build system will be generated in the "
      "subdirectory (such as a VS IDE solution file).  "
      "Note that inter-target dependencies supercede this exclusion.  "
      "If a target built by the parent project depends on a target in the "
      "subdirectory, the dependee target will be included in the parent "
      "project build system to satisfy the dependency."
      ;
    }

  cmTypeMacro(cmAddSubDirectoryCommand, cmCommand);
};



#endif
