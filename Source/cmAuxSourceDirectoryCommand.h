/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
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
  virtual bool InitialPass(std::vector<std::string> const& args,
                           cmExecutionStatus &status);

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() const { return "aux_source_directory";}
  
  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() const
    {
    return "Find all source files in a directory.";
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation() const
    {
    return
      "  aux_source_directory(<dir> <variable>)\n"
      "Collects the names of all the source files in the specified "
      "directory and stores the list in the <variable> provided.  This "
      "command is intended to be used by projects that use explicit "
      "template instantiation.  Template instantiation files can be "
      "stored in a \"Templates\" subdirectory and collected automatically "
      "using this command to avoid manually listing all instantiations.\n"
      "It is tempting to use this command to avoid writing the list of "
      "source files for a library or executable target.  While this seems "
      "to work, there is no way for CMake to generate a build system that "
      "knows when a new source file has been added.  Normally the "
      "generated build system knows when it needs to rerun CMake because "
      "the CMakeLists.txt file is modified to add a new source.  When the "
      "source is just added to the directory without modifying this file, "
      "one would have to manually rerun CMake to generate a build system "
      "incorporating the new file.";
    }
  
  cmTypeMacro(cmAuxSourceDirectoryCommand, cmCommand);
};



#endif
