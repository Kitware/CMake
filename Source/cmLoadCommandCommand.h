/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmLoadCommandCommand_h
#define cmLoadCommandCommand_h

#include "cmCommand.h"

/** \class cmLoadCommandCommand
 * \brief Load in a Command plugin
 *
 * cmLoadCommandCommand loads a command into CMake
 */
class cmLoadCommandCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone()
    {
    return new cmLoadCommandCommand;
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
  virtual const char* GetName() const {return "load_command";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() const
    {
    return "Deprecated.  Use macro() or function() instead.";
    }

  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation() const
    {
    return
      "This command will be removed in CMake 3.0.  "
      "It works only when the target architecture matches the "
      "running CMake binary.  "
      "Use macro() or function() to add commands.  "
      "Use execute_process() to run advanced computations "
      "in external processes."
      "\n"
      "  load_command(COMMAND_NAME <loc1> [loc2 ...])\n"
      "The given locations are searched for a library whose name is "
      "cmCOMMAND_NAME.  If found, it is loaded as a module and the command "
      "is added to the set of available CMake commands.  Usually, "
      "TRY_COMPILE is used before this command to compile the module. "
      "If the command is successfully loaded a variable named\n"
      "  CMAKE_LOADED_COMMAND_<COMMAND_NAME>\n"
      "will be set to the full path of the module that was loaded.  "
      "Otherwise the variable will not be set.";
    }

  /** This command is kept for compatibility with older CMake versions. */
  virtual bool IsDiscouraged() const
    {
    return true;
    }

  cmTypeMacro(cmLoadCommandCommand, cmCommand);
};



#endif
