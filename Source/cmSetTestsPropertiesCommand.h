/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmSetTestsPropertiesCommand_h
#define cmSetTestsPropertiesCommand_h

#include "cmCommand.h"

class cmSetTestsPropertiesCommand : public cmCommand
{
public:
  virtual cmCommand* Clone() 
    {
    return new cmSetTestsPropertiesCommand;
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
  virtual const char* GetName() const { return "set_tests_properties";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() const
    {
    return "Set a property of the tests.";
    }

  /**
   * Longer documentation.
   */
  virtual const char* GetFullDocumentation() const
    {
    return
      "  set_tests_properties(test1 [test2...] PROPERTIES prop1 value1 prop2"
      " value2)\n"
      "Set a property for the tests. If the property is not found, CMake "
      "will report an error. The properties include:\n"
      "WILL_FAIL: If set to true, this will invert the pass/fail flag of the"
      " test.\n"
      "PASS_REGULAR_EXPRESSION: If set, the test output will be checked "
      "against the specified regular expressions and at least one of the"
      " regular "
      "expressions has to match, otherwise the test will fail.\n"
      "  Example: PASS_REGULAR_EXPRESSION \"TestPassed;All ok\"\n"
      "FAIL_REGULAR_EXPRESSION: If set, if the output will match to one of "
      "specified regular expressions, the test will fail.\n"
      "  Example: PASS_REGULAR_EXPRESSION \"[^a-z]Error;ERROR;Failed\"\n"
      "Both PASS_REGULAR_EXPRESSION and FAIL_REGULAR_EXPRESSION expect a "
      "list of regular expressions.\n"
      "TIMEOUT: Setting this will limit the test runtime to the number of "
      "seconds specified.\n";
    }

  cmTypeMacro(cmSetTestsPropertiesCommand, cmCommand);

  static bool SetOneTest(const char *tname, 
                         std::vector<std::string> &propertyPairs,
                         cmMakefile *mf,
                         std::string &errors);
};



#endif
