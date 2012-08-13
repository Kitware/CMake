/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
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
  virtual bool InitialPass(std::vector<std::string> const& args,
                           cmExecutionStatus &status);

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() const { return "get_source_file_property";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() const
    {
    return "Get a property for a source file.";
    }

  /**
   * Longer documentation.
   */
  virtual const char* GetFullDocumentation() const
    {
      return
        "  get_source_file_property(VAR file property)\n"
        "Get a property from a source file.  The value of the property is "
        "stored in the variable VAR.  If the property is not found, VAR "
        "will be set to \"NOTFOUND\". Use set_source_files_properties to set "
        "property values.  Source file properties usually control how the "
        "file is built. One property that is always there is LOCATION"
        "\n"
        "See also the more general get_property() command.";
    }

  cmTypeMacro(cmGetSourceFilePropertyCommand, cmCommand);
};



#endif
