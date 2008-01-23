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
#ifndef cmGetDirectoryPropertyCommand_h
#define cmGetDirectoryPropertyCommand_h

#include "cmCommand.h"

class cmGetDirectoryPropertyCommand : public cmCommand
{
public:
  virtual cmCommand* Clone() 
    {
      return new cmGetDirectoryPropertyCommand;
    }

  /**
   * This is called when the command is first encountered in
   * the input file.
   */
  virtual bool InitialPass(std::vector<std::string> const& args,
                           cmExecutionStatus &status);

  /**
   * This determines if the command is invoked when in script mode.
   */
  virtual bool IsScriptable() { return true; }

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() { return "get_directory_property";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Get a property of the directory.";
    }
  
  /**
   * Longer documentation.
   */
  virtual const char* GetFullDocumentation()
    {
      return
        "  get_directory_property(VAR [DIRECTORY dir] property)\n"
        "Get a property from the Directory.  The value of the property is " 
        "stored in the variable VAR. If the property is not found, "
        "CMake will report an error. The properties include: VARIABLES, "
        "CACHE_VARIABLES, COMMANDS, MACROS, INCLUDE_DIRECTORIES, "
        "LINK_DIRECTORIES, DEFINITIONS, INCLUDE_REGULAR_EXPRESSION, "
        "LISTFILE_STACK, PARENT_DIRECTORY, and "
        "DEFINITION varname.  If the DIRECTORY argument is provided then "
        "the property of the provided directory will be retrieved "
        "instead of the current directory. You can only get properties "
        "of a directory during or after it has been traversed by cmake.";
    }
  
  cmTypeMacro(cmGetDirectoryPropertyCommand, cmCommand);
};



#endif
