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
#ifndef cmGetCMakePropertyCommand_h
#define cmGetCMakePropertyCommand_h

#include "cmStandardIncludes.h"
#include "cmCommand.h"

class cmGetCMakePropertyCommand : public cmCommand
{
public:
  virtual cmCommand* Clone() 
    {
      return new cmGetCMakePropertyCommand;
    }

  /**
   * This is called when the command is first encountered in
   * the input file.
   */
  virtual bool InitialPass(std::vector<std::string> const& args);

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() { return "GET_CMAKE_PROPERTY";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Get a property of the CMake.";
    }
  
  /**
   * Longer documentation.
   */
  virtual const char* GetFullDocumentation()
    {
      return
        "  GET_CMAKE_PROPERTY(VAR property)\n"
        "Get a property from the CMake.  The value of the property is" 
        "stored in the variable VAR. If the property is not found,"
        "CMake will report an error. The properties include: VARIABLES, "
        "CACHE_VARIABLES, COMMANDS, and MACROS.";
    }
  
  cmTypeMacro(cmGetCMakePropertyCommand, cmCommand);
};



#endif
