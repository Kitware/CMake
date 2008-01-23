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
#ifndef cmDefinesPropertyCommand_h
#define cmDefinesPropertyCommand_h

#include "cmCommand.h"

class cmDefinePropertyCommand : public cmCommand
{
public:
  virtual cmCommand* Clone() 
    {
      return new cmDefinePropertyCommand;
    }

  /**
   * This is called when the command is first encountered in
   * the input file.
   */
  virtual bool InitialPass(std::vector<std::string> const& args,
                           cmExecutionStatus &status);

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() { return "define_property";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Define properties used by CMake.";
    }
  
  /**
   * Longer documentation.
   */
  virtual const char* GetFullDocumentation()
    {
      return
        "  define_property(property_name scope_value\n"
        "                  short_description\n"
        "                  full_description inherit)\n"
        "Define a property for a scope. scope_value is either GLOBAL, "
        "DIRECTORY, TARGET, TEST, SOURCE_FILE, VARIABLE or CACHED_VARIABLE. "
        "The short and full descriptions are used to document the property. "
        "If inherit is TRUE, it will inherit its value from the next more "
        "global property if it hasn't been set at the specified scope. "
        "This means that e.g. a TARGET property inherits it's value from the "
        "DIRECTORY property with the same name if it hasn't been set for the "
        "target, and then from GLOBAL if it hasn't been set for the directory."
        ;
    }
  
  cmTypeMacro(cmDefinePropertyCommand, cmCommand);
};



#endif
