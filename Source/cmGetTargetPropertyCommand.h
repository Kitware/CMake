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
#ifndef cmGetTargetPropertyCommand_h
#define cmGetTargetPropertyCommand_h

#include "cmStandardIncludes.h"
#include "cmCommand.h"

class cmGetTargetPropertyCommand : public cmCommand
{
public:
  virtual cmCommand* Clone() 
    {
      return new cmGetTargetPropertyCommand;
    }

  /**
   * This is called when the command is first encountered in
   * the input file.
   */
  virtual bool InitialPass(std::vector<std::string> const& args);

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() { return "GET_TARGET_PROPERTY";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Get a property from a target.";
    }
  
  /**
   * Longer documentation.
   */
  virtual const char* GetFullDocumentation()
    {
      return
        "  GET_TARGET_PROPERTY(VAR target property)\n"
        "Get a property from a target.   The value of the property is " 
        "stored in the variable VAR.  If the property is not found, var "
        "will be set to NOT_FOUND.  Use SET_TARGET_PROPERTIES to set "
        "property values.  Properties are usually used to control how "
        "a target is built.";
    }
  
  cmTypeMacro(cmGetTargetPropertyCommand, cmCommand);
};



#endif
