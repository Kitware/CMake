/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmGetCMakePropertyCommand_h
#define cmGetCMakePropertyCommand_h

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
  virtual bool InitialPass(std::vector<std::string> const& args,
                           cmExecutionStatus &status);

  /**
   * This determines if the command is invoked when in script mode.
   */
  virtual bool IsScriptable() { return true; }

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() { return "get_cmake_property";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Get a property of the CMake instance.";
    }
  
  /**
   * Longer documentation.
   */
  virtual const char* GetFullDocumentation()
    {
      return
        "  get_cmake_property(VAR property)\n"
        "Get a property from the CMake instance.  The value of the "
        "property is stored in the variable VAR. If the property is "
        "not found, CMake will report an error. Some supported properties "
        "include: VARIABLES, CACHE_VARIABLES, COMMANDS, MACROS, and "
        "COMPONENTS.";
    }
  
  cmTypeMacro(cmGetCMakePropertyCommand, cmCommand);
};



#endif
