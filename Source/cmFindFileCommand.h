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
#ifndef cmFindFileCommand_h
#define cmFindFileCommand_h

#include "cmCommand.h"

/** \class cmFindFileCommand
 * \brief Define a command to search for an executable program.
 *
 * cmFindFileCommand is used to define a CMake variable
 * that specifies an executable program. The command searches 
 * in the current path (e.g., PATH environment variable) for
 * an executable that matches one of the supplied names.
 */
class cmFindFileCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone() 
    {
    return new cmFindFileCommand;
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
   * This determines if the command is invoked when in script mode.
   */
  virtual bool IsScriptable() { return true; }

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() { return "FIND_FILE";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Find the full path to a file.";
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "  FIND_FILE(<VAR> fileName path1 [path2 ...]\n"
      "            [DOC \"docstring\"])\n"
      "Find the full path to a file named by fileName.  Paths "
      "are searched in the order specified.  A cache entry named by "
      "<VAR> is created to store the result.  If the file is not "
      "found, the result will be <VAR>-NOTFOUND.  If DOC is specified "
      "then the next argument is treated as a documentation string for "
      "the cache entry <VAR>.  Note that since executables can have "
      "different extensions on different platforms, FIND_PROGRAM "
      "should be used instead of FIND_FILE when looking for them.";
    }
  
  cmTypeMacro(cmFindFileCommand, cmCommand);
};



#endif
