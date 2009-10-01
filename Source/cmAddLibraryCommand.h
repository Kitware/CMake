/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmLibrarysCommand_h
#define cmLibrarysCommand_h

#include "cmCommand.h"

/** \class cmLibrarysCommand
 * \brief Defines a list of executables to build.
 *
 * cmLibrarysCommand defines a list of executable (i.e., test)
 * programs to create.
 */
class cmAddLibraryCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone() 
    {
    return new cmAddLibraryCommand;
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
  virtual const char* GetName() { return "add_library";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Add a library to the project using the specified source files.";
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "  add_library(<name> [STATIC | SHARED | MODULE]\n"
      "              [EXCLUDE_FROM_ALL]\n"
      "              source1 source2 ... sourceN)\n"
      "Adds a library target called <name> to be built from the "
      "source files listed in the command invocation.  "
      "The <name> corresponds to the logical target name and must be "
      "globally unique within a project.  "
      "The actual file name of the library built is constructed based on "
      "conventions of the native platform "
      "(such as lib<name>.a or <name>.lib)."
      "\n"
      "STATIC, SHARED, or MODULE may be given to specify the type of library "
      "to be created.  "
      "STATIC libraries are archives of object files for use when linking "
      "other targets.  "
      "SHARED libraries are linked dynamically and loaded at runtime.  "
      "MODULE libraries are plugins that are not linked into other targets "
      "but may be loaded dynamically at runtime using dlopen-like "
      "functionality.  "
      "If no type is given explicitly the type is STATIC or SHARED based "
      "on whether the current value of the variable BUILD_SHARED_LIBS is "
      "true."
      "\n"
      "By default the library file will be created in the build tree "
      "directory corresponding to the source tree directory in which "
      "the command was invoked.  "
      "See documentation of the ARCHIVE_OUTPUT_DIRECTORY, "
      "LIBRARY_OUTPUT_DIRECTORY, and RUNTIME_OUTPUT_DIRECTORY "
      "target properties to change this location.  "
      "See documentation of the OUTPUT_NAME target property to change "
      "the <name> part of the final file name.  "
      "\n"
      "If EXCLUDE_FROM_ALL is given the corresponding property will be "
      "set on the created target.  "
      "See documentation of the EXCLUDE_FROM_ALL target property for "
      "details."
      "\n"
      "The add_library command can also create IMPORTED library "
      "targets using this signature:\n"
      "  add_library(<name> <SHARED|STATIC|MODULE|UNKNOWN> IMPORTED)\n"
      "An IMPORTED library target references a library file located "
      "outside the project.  "
      "No rules are generated to build it.  "
      "The target name has scope in the directory in which it is created "
      "and below.  "
      "It may be referenced like any target built within the project.  "
      "IMPORTED libraries are useful for convenient reference from "
      "commands like target_link_libraries.  "
      "Details about the imported library are specified by setting "
      "properties whose names begin in \"IMPORTED_\".  "
      "The most important such property is IMPORTED_LOCATION "
      "(and its per-configuration version IMPORTED_LOCATION_<CONFIG>) "
      "which specifies the location of the main library file on disk.  "
      "See documentation of the IMPORTED_* properties for more information."
      ;
    }
  
  cmTypeMacro(cmAddLibraryCommand, cmCommand);
};


#endif
