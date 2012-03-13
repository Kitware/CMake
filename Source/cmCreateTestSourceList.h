/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmCreateTestSourceList_h
#define cmCreateTestSourceList_h

#include "cmCommand.h"

/** \class cmCreateTestSourceList
 * \brief 
 *
 */

class cmCreateTestSourceList : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone() 
    {
    return new cmCreateTestSourceList;
    }

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  virtual bool InitialPass(std::vector<std::string> const& args,
                           cmExecutionStatus &status);

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() const {return "create_test_sourcelist";}
  
  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() const
    {
    return "Create a test driver and source list for building test programs.";
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation() const
    {
    return
      "  create_test_sourcelist(sourceListName driverName\n"
      "                         test1 test2 test3\n"
      "                         EXTRA_INCLUDE include.h\n"
      "                         FUNCTION function)\n"
      "A test driver is a program that links together many small tests into "
      "a single executable.  This is useful when building static executables "
      "with large libraries to shrink the total required size.  "
      "The list of source files "
      "needed to build the test driver will be in sourceListName.  "
      "DriverName is the name of the test driver program.  The rest of "
      "the arguments consist of a list of test source files, can be "
      "semicolon separated.  Each test source file should have a function in "
      "it that is the same name as the file with no extension (foo.cxx "
      "should have int foo(int, char*[]);) DriverName will be able to "
      "call each of the "
      "tests by name on the command line. If EXTRA_INCLUDE is specified, "
      "then the next argument is included into the generated file. If "
      "FUNCTION is specified, then the next argument is taken as a function "
      "name that is passed a pointer to ac and av.  This can be used to add "
      "extra command line processing to each test. The cmake variable "
      "CMAKE_TESTDRIVER_BEFORE_TESTMAIN can be set to have code that will be "
      "placed directly before calling the test main function.   "
      "CMAKE_TESTDRIVER_AFTER_TESTMAIN can be set to have code that will be "
      "placed directly after the call to the test main function.";
    }
  
  cmTypeMacro(cmCreateTestSourceList, cmCommand);
};



#endif
