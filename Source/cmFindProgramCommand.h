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
#ifndef cmFindProgramCommand_h
#define cmFindProgramCommand_h

#include "cmCommand.h"

/** \class cmFindProgramCommand
 * \brief Define a command to search for an executable program.
 *
 * cmFindProgramCommand is used to define a CMake variable
 * that specifies an executable program. The command searches 
 * in the current path (e.g., PATH environment variable) for
 * an executable that matches one of the supplied names.
 */
class cmFindProgramCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone() 
    {
    return new cmFindProgramCommand;
    }

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  virtual bool InitialPass(std::vector<std::string> const& args);

  /**
   * This determines if the command gets propagated down
   * to makefiles located in subdirectories.
   */
  virtual bool IsInherited() { return false;  }

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() { return "FIND_PROGRAM";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Find an executable program.";
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "  FIND_PROGRAM(<VAR> NAMES name1 [name2 ...]\n"
      "               [PATHS path1 path2 ...]\n"
      "               [NO_SYSTEM_PATH]\n"
      "               [DOC \"docstring\"])\n"
      "Find an executable named by one of the names given after the NAMES "
      "argument.  Paths specified after the PATHS argument are searched "
      "in the order specified.  If the NO_SYSTEM_PATH argument is not "
      "specified, the search continues with the system search path "
      "specified by the PATH environment variable.  A cache entry named "
      "by <VAR> is created to store the result.  If the program is not "
      "found, the result will be <VAR>-NOTFOUND.  If DOC is specified "
      "then the next argument is treated as a documentation string for "
      "the cache entry <VAR>.\n"
      "  FIND_PROGRAM(VAR executableName [path1 path2 ...])\n"
      "Find a program with the given name by searching in the specified "
      "paths.  This is a short-hand signature for the command that is "
      "sufficient in many cases.";
    }
  
  cmTypeMacro(cmFindProgramCommand, cmCommand);
};



#endif
