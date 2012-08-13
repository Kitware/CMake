/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmGetTestPropertyCommand_h
#define cmGetTestPropertyCommand_h

#include "cmCommand.h"

class cmGetTestPropertyCommand : public cmCommand
{
public:
  virtual cmCommand* Clone()
    {
    return new cmGetTestPropertyCommand;
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
  virtual const char* GetName() const { return "get_test_property";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() const
    {
    return "Get a property of the test.";
    }

  /**
   * Longer documentation.
   */
  virtual const char* GetFullDocumentation() const
    {
    return
      "  get_test_property(test property VAR)\n"
      "Get a property from the Test.  The value of the property is "
      "stored in the variable VAR.  If the property is not found, VAR "
      "will be set to \"NOTFOUND\". For a list of standard properties "
      "you can type cmake --help-property-list"
      "\n"
      "See also the more general get_property() command.";
    }

  cmTypeMacro(cmGetTestPropertyCommand, cmCommand);
};



#endif
