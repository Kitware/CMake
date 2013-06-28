/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmGetFilenameComponentCommand_h
#define cmGetFilenameComponentCommand_h

#include "cmCommand.h"

/** \class cmGetFilenameComponentCommand
 * \brief Get a specific component of a filename.
 *
 * cmGetFilenameComponentCommand is a utility command used to get the path,
 * name, extension or name without extension of a full filename.
 */
class cmGetFilenameComponentCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone()
    {
    return new cmGetFilenameComponentCommand;
    }

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  virtual bool InitialPass(std::vector<std::string> const& args,
                           cmExecutionStatus &status);

  /**
   * This determines if the command is invoked when in script mode.
   */
  virtual bool IsScriptable() const { return true; }

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() const { return "get_filename_component";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() const
    {
    return "Get a specific component of a full filename.";
    }

  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation() const
    {
    return
      "  get_filename_component(<VAR> <FileName> <COMP> [CACHE])\n"
      "Set <VAR> to a component of <FileName>, where <COMP> is one of:\n"
      " DIRECTORY = Directory without file name\n"
      " NAME      = File name without directory\n"
      " EXT       = File name longest extension (.b.c from d/a.b.c)\n"
      " NAME_WE   = File name without directory or longest extension\n"
      " ABSOLUTE  = Full path to file\n"
      " REALPATH  = Full path to existing file with symlinks resolved\n"
      " PATH      = Legacy alias for DIRECTORY (use for CMake <= 2.8.11)\n"
      "Paths are returned with forward slashes and have no trailing slahes. "
      "The longest file extension is always considered. "
      "If the optional CACHE argument is specified, the result variable is "
      "added to the cache.\n"
      "  get_filename_component(<VAR> FileName\n"
      "                         PROGRAM [PROGRAM_ARGS <ARG_VAR>]\n"
      "                         [CACHE])\n"
      "The program in FileName will be found in the system search path or "
      "left as a full path.  If PROGRAM_ARGS is present with PROGRAM, then "
      "any command-line arguments present in the FileName string are split "
      "from the program name and stored in <ARG_VAR>.  This is used to "
      "separate a program name from its arguments in a command line string.";
    }

  cmTypeMacro(cmGetFilenameComponentCommand, cmCommand);
};



#endif
