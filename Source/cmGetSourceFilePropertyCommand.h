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
#ifndef cmGetSourceFilePropertyCommand_h
#define cmGetSourceFilePropertyCommand_h

#include "cmCommand.h"

class cmGetSourceFilePropertyCommand : public cmCommand
{
public:
  virtual cmCommand* Clone() 
    {
      return new cmGetSourceFilePropertyCommand;
    }

  /**
   * This is called when the command is first encountered in
   * the input file.
   */
  virtual bool InitialPass(std::vector<std::string> const& args);

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() { return "GET_SOURCE_FILE_PROPERTY";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Get a property for a source file.";
    }
  
  /**
   * Longer documentation.
   */
  virtual const char* GetFullDocumentation()
    {
      return
        "  GET_SOURCE_FILE_PROPERTY(VAR file property)\n"
        "Get a property from a source file.  The value of the property is " 
        "stored in the variable VAR.  If the property is not found, var "
        "will be set to NOT_FOUND.  Use SET_SOURCE_FILES_PROPERTIES to set "
        "property values.  Source file properties usually control how the "
        "file is built.";
    }
  
  cmTypeMacro(cmGetSourceFilePropertyCommand, cmCommand);
};



#endif
