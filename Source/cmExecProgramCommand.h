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
#ifndef cmExecProgramCommand_h
#define cmExecProgramCommand_h

#include "cmCommand.h"

/** \class cmExecProgramCommand
 * \brief Command that adds a target to the build system.
 *
 * cmExecProgramCommand adds an extra target to the build system.
 * This is useful when you would like to add special
 * targets like "install,", "clean," and so on.
 */
class cmExecProgramCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone() 
    {
    return new cmExecProgramCommand;
    }

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  virtual bool InitialPass(std::vector<std::string> const& args);
  
  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() 
    {return "EXEC_PROGRAM";}
  
  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Run and executable program during the processing of the CMakeList.txt file.";
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "  EXEC_PROGRAM(Executable [directory in which to run]\n"
      "               [ARGS <arguments to executable>]\n"
      "               [OUTPUT_VARIABLE <var>]\n"
      "               [RETURN_VALUE <var>])\n"
      "The executable is run in the optionally specified Directory.  The "
      "executable can include arguments if it is double quoted, but it is "
      "better to use the optional ARGS argument to specify arguments to the "
      "program.   This is because cmake will then be able to escape spaces "
      "in the Executable path.  An optional argument OUTPUT_VARIABLE "
      "specifies a variable in which to store the output. "
      "To capture the return value of the execution, use RETURN_VALUE variable. "
      "If OUTPUT_VARIABLE is specified, then no output will go to the stdout/stderr "
      "of the console running cmake.";
    }
  
  cmTypeMacro(cmExecProgramCommand, cmCommand);
};

#endif
