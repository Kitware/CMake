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
  virtual bool InitialPass(std::vector<std::string> const& args);

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() { return "DEFINE_PROPERTY";}

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
        "  DEFINE_PROPERTY(property_name scope_value\n"
        "                  short_description\n"
        "                  full_description chain)\n"
        "Define a property for a scope. The scope_value is either GLOBAL "
        "DIRECTORY, TARGET, TEST, SOURCE_FILE, VARIABLE, CACHED_VARIABLE. "
        "The short and full "
        "descriptions are used to document the property, chain indicates "
        "if that property chains such that a request for the property "
        "on a target will chain up to the directory if it is not set on the "
        "target. In such cases the property's scope is the most specific. "
        "In that example the scope would be TARGET even though it can "
        "chain up to DIRECTORY and GLOBAL."
        ;
    }
  
  cmTypeMacro(cmDefinePropertyCommand, cmCommand);
};



#endif
