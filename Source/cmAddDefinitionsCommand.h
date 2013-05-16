/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmAddDefinitionsCommand_h
#define cmAddDefinitionsCommand_h

#include "cmCommand.h"

/** \class cmAddDefinitionsCommand
 * \brief Specify a list of compiler defines
 *
 * cmAddDefinitionsCommand specifies a list of compiler defines. These defines
 * will be added to the compile command.
 */
class cmAddDefinitionsCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone()
    {
    return new cmAddDefinitionsCommand;
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
  virtual const char* GetName() const {return "add_definitions";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() const
    {
    return "Adds -D define flags to the compilation of source files.";
    }

  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation() const
    {
    return
      "  add_definitions(-DFOO -DBAR ...)\n"
      "Adds flags to the compiler command line for sources in the current "
      "directory and below.  This command can be used to add any flags, "
      "but it was originally intended to add preprocessor definitions.  "
      "Flags beginning in -D or /D that look like preprocessor definitions "
      "are automatically added to the COMPILE_DEFINITIONS property for "
      "the current directory.  Definitions with non-trivial values may be "
      "left in the set of flags instead of being converted for reasons of "
      "backwards compatibility.  See documentation of the directory, "
      "target, and source file COMPILE_DEFINITIONS properties for details "
      "on adding preprocessor definitions to specific scopes and "
      "configurations."
      ;
    }

  cmTypeMacro(cmAddDefinitionsCommand, cmCommand);
private:
  bool ParseDefinition(std::string const& def);
};



#endif
