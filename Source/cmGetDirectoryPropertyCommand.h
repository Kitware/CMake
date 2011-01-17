/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
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
    return "Get a property of DIRECTORY scope.";
    }
  
  /**
   * Longer documentation.
   */
  virtual const char* GetFullDocumentation()
    {
      return
        "  get_directory_property(<variable> [DIRECTORY <dir>] <prop-name>)\n"
        "Store a property of directory scope in the named variable.  "
        "If the property is not defined the empty-string is returned.  "
        "The DIRECTORY argument specifies another directory from which "
        "to retrieve the property value.  "
        "The specified directory must have already been traversed by "
        "CMake."
        "\n"
        "  get_directory_property(<variable> [DIRECTORY <dir>]\n"
        "                         DEFINITION <var-name>)\n"
        "Get a variable definition from a directory.  "
        "This form is useful to get a variable definition from another "
        "directory."
        "\n"
        "See also the more general get_property() command.";
    }
  
  cmTypeMacro(cmGetDirectoryPropertyCommand, cmCommand);
};



#endif
