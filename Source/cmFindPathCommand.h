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
#ifndef cmFindPathCommand_h
#define cmFindPathCommand_h

#include "cmCommand.h"


/** \class cmFindPathCommand
 * \brief Define a command to search for a library.
 *
 * cmFindPathCommand is used to define a CMake variable
 * that specifies a library. The command searches for a given
 * file in a list of directories.
 */
class cmFindPathCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone() 
    {
    return new cmFindPathCommand;
    }

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  virtual bool InitialPass(std::vector<std::string> const& args);

  /**
   * This determines if the command is invoked when in script mode.
   */
  virtual bool IsScriptable() { return true; }

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() {return "FIND_PATH";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Find the directory containing a file.";
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "  FIND_PATH(<VAR> fileName path1 [path2 ...]\n"
      "            [DOC \"docstring\"])\n"
      "Find the directory containing a file named by fileName.  "
      "A cache entry named by "
      "<VAR> is created to store the result.  If the file is not "
      "found, the result will be <VAR>-NOTFOUND.  If DOC is specified "
      "then the next argument is treated as a documentation string for "
      "the cache entry <VAR>.  "
      "The search proceeds first in paths listed in the CMAKE_INCLUDE_PATH "
      "CMake variable (which is generally set by the user on the command line), "
      "then in paths listed in the CMAKE_INCLUDE_PATH environment variable, "
      "then in paths given to the command, and finally in paths listed in the "
      "PATH environment variable.";
    }
  cmStdString FindHeaderInFrameworks( std::vector<std::string> path,
                                      const char* var, const char* file);

  cmTypeMacro(cmFindPathCommand, cmCommand);
};



#endif
