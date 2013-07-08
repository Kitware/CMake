/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmIncludeCommand_h
#define cmIncludeCommand_h

#include "cmCommand.h"

/** \class cmIncludeCommand
 * \brief
 *
 *  cmIncludeCommand defines a list of distant
 *  files that can be "included" in the current list file.
 *  In almost every sense, this is identical to a C/C++
 *  #include command.  Arguments are first expended as usual.
 */
class cmIncludeCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone()
    {
    return new cmIncludeCommand;
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
  virtual const char* GetName() const {return "include";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() const
    {
    return "Read CMake listfile code from the given file.";
    }

  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation() const
    {
    return
      "  include(<file|module> [OPTIONAL] [RESULT_VARIABLE <VAR>]\n"
      "                        [NO_POLICY_SCOPE])\n"
      "Reads CMake listfile code from the given file.  Commands in the file "
      "are processed immediately as if they were written in place of the "
      "include command.  If OPTIONAL is present, then no error "
      "is raised if the file does not exist.  If RESULT_VARIABLE is given "
      "the variable will be set to the full filename which "
      "has been included or NOTFOUND if it failed.\n"
      "\n"
      "Processing of files occurs after searching for the project() command, "
      "so a project() command cannot be issued via a file included with include(). "
      "See the project() command for more information."
      "\n"
      "If a module is specified instead of a file, the file with name "
      "<modulename>.cmake is searched first in CMAKE_MODULE_PATH, then in the "
      "CMake module directory. There is one exception to this: if the file "
      "which calls include() is located itself in the CMake module directory, "
      "then first the CMake module directory is searched and "
      "CMAKE_MODULE_PATH afterwards. See also policy CMP0017."
      "\n"
      "See the cmake_policy() command documentation for discussion of the "
      "NO_POLICY_SCOPE option."
      ;
    }

  cmTypeMacro(cmIncludeCommand, cmCommand);
};



#endif
