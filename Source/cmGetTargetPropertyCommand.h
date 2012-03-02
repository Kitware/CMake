/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmGetTargetPropertyCommand_h
#define cmGetTargetPropertyCommand_h

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
  virtual bool InitialPass(std::vector<std::string> const& args,
                           cmExecutionStatus &status);

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() const { return "get_target_property";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() const
    {
    return "Get a property from a target.";
    }
  
  /**
   * Longer documentation.
   */
  virtual const char* GetFullDocumentation() const
    {
      return
        "  get_target_property(VAR target property)\n"
        "Get a property from a target.   The value of the property is "
        "stored in the variable VAR.  If the property is not found, VAR "
        "will be set to \"NOTFOUND\".  Use set_target_properties to set "
        "property values.  Properties are usually used to control how "
        "a target is built, but some query the target instead.  "
        "This command can get properties for any target so far created. "
        "The targets do not need to be in the current CMakeLists.txt file."
        "\n"
        "See also the more general get_property() command.";
    }
  
  cmTypeMacro(cmGetTargetPropertyCommand, cmCommand);
};



#endif
