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
      "  FIND_FILE(VAR fileName path1 path2 ... [DOC docstring])\n"
      "If the file is found, then VAR is set to the path where it was found.  "
      "A cache entry named by VAR is created to "
      "store the result.  VAR-NOTFOUND is the value used if the file was "
      "not found.  CMake will continue to look as long as the value "
      "is not found.  If DOC is specified the next argument is the "
      "documentation string for the cache entry VAR.  Since Executables "
      "can have different extensions on different platforms, FIND_PROGRAM "
      "should be used instead of FIND_FILE when looking for and executable.";
    }
  
  cmTypeMacro(cmFindFileCommand, cmCommand);
};



#endif
