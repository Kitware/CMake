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
#ifndef cmSetDirectoryPropertiesCommand_h
#define cmSetDirectoryPropertiesCommand_h

#include "cmCommand.h"

class cmSetDirectoryPropertiesCommand : public cmCommand
{
public:
  virtual cmCommand* Clone() 
    {
      return new cmSetDirectoryPropertiesCommand;
    }

  /**
   * This is called when the command is first encountered in
   * the input file.
   */
  virtual bool InitialPass(std::vector<std::string> const& args);

  /**
   * This determines if the command gets propagated down
   * to makefiles located in subdirectories.
   */
  virtual bool IsInherited() {return true;}

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() { return "SET_DIRECTORY_PROPERTIES";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Set a property of the directory.";
    }
  
  /**
   * Longer documentation.
   */
  virtual const char* GetFullDocumentation()
    {
      return
        "  SET_DIRECTORY_PROPERTIES(PROPERTIES prop1 value1 prop2 value2)\n"
        "Set a property for the current directory and subdirectories. If the "
        "property is not found, CMake will report an error. The properties "
        "include: INCLUDE_DIRECTORIES, LINK_DIRECTORIES, "
        "INCLUDE_REGULAR_EXPRESSION, and ADDITIONAL_MAKE_CLEAN_FILES.\n"
        "ADDITIONAL_MAKE_CLEAN_FILES is a list of files that will be cleaned "
        "as a part of \"make clean\" stage.";
    }
  
  cmTypeMacro(cmSetDirectoryPropertiesCommand, cmCommand);
};



#endif
