/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef cmExecProgramCommand_h
#define cmExecProgramCommand_h

#include "cmStandardIncludes.h"
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
      "EXEC_PROGRAM(Executable [Directory to run in] [ARGS arguments to executable] [OUTPUT_VARIABLE var] [RETURN_VALUE var])"
      "The executable is run in the optionally specified Directory.  The executable "
      "can include arguments if it is double quoted, but it is better to use the "
      "optional ARGS argument to specify arguments to the program.   This is because "
      "cmake will then be able to escape spaces in the Executable path.  An optiona "
      "argument OUPUT_VARIABLE specifies a variable to which the output will be set. "
      "To capture the return value of the execution, use RETURN_VALUE variable.";
    }
  
  cmTypeMacro(cmExecProgramCommand, cmCommand);
};

#endif
