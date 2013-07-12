/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmExecutablesCommand_h
#define cmExecutablesCommand_h

#include "cmCommand.h"

/** \class cmExecutablesCommand
 * \brief Defines a list of executables to build.
 *
 * cmExecutablesCommand defines a list of executable (i.e., test)
 * programs to create.
 */
class cmAddExecutableCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone()
    {
    return new cmAddExecutableCommand;
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
  virtual const char* GetName() const { return "add_executable";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() const
    {
    return
      "Add an executable to the project using the specified source files.";
    }

  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation() const
    {
    return
      "  add_executable(<name> [WIN32] [MACOSX_BUNDLE]\n"
      "                 [EXCLUDE_FROM_ALL]\n"
      "                 source1 source2 ... sourceN)\n"
      "Adds an executable target called <name> to be built from the "
      "source files listed in the command invocation.  "
      "The <name> corresponds to the logical target name and must be "
      "globally unique within a project.  "
      "The actual file name of the executable built is constructed based on "
      "conventions of the native platform "
      "(such as <name>.exe or just <name>).  "
      "\n"
      "By default the executable file will be created in the build tree "
      "directory corresponding to the source tree directory in which "
      "the command was invoked.  "
      "See documentation of the RUNTIME_OUTPUT_DIRECTORY "
      "target property to change this location.  "
      "See documentation of the OUTPUT_NAME target property to change "
      "the <name> part of the final file name.  "
      "\n"
      "If WIN32 is given the property WIN32_EXECUTABLE will be set on the "
      "target created.  "
      "See documentation of that target property for details."
      "\n"
      "If MACOSX_BUNDLE is given the corresponding property will be "
      "set on the created target.  "
      "See documentation of the MACOSX_BUNDLE target property for details."
      "\n"
      "If EXCLUDE_FROM_ALL is given the corresponding property will be "
      "set on the created target.  "
      "See documentation of the EXCLUDE_FROM_ALL target property for "
      "details."
      "\n"
      "The add_executable command can also create IMPORTED executable "
      "targets using this signature:\n"
      "  add_executable(<name> IMPORTED [GLOBAL])\n"
      "An IMPORTED executable target references an executable file located "
      "outside the project.  "
      "No rules are generated to build it.  "
      "The target name has scope in the directory in which it is created "
      "and below, but the GLOBAL option extends visibility.  "
      "It may be referenced like any target built within the project.  "
      "IMPORTED executables are useful for convenient reference from "
      "commands like add_custom_command.  "
      "Details about the imported executable are specified by setting "
      "properties whose names begin in \"IMPORTED_\".  "
      "The most important such property is IMPORTED_LOCATION "
      "(and its per-configuration version IMPORTED_LOCATION_<CONFIG>) "
      "which specifies the location of the main executable file on disk.  "
      "See documentation of the IMPORTED_* properties for more information."
      "\n"
      "The signature\n"
      "  add_executable(<name> ALIAS <target>)\n"
      "creates an alias, such that <name> can be used to refer to <target> "
      "in subsequent commands.  The <name> does not appear in the generated "
      "buildsystem as a make target.  The <target> may not be an IMPORTED "
      "target or an ALIAS.  The <name> may not be used to modify properties "
      "of <target>, that is, it may not be used as the operand of "
      "set_property, set_target_properties, target_link_libraries etc."
      ;
    }

  cmTypeMacro(cmAddExecutableCommand, cmCommand);
};


#endif
