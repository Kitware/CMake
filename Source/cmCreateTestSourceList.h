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
  virtual bool InitialPass(std::vector<std::string> const& args);

  /**
   * This determines if the command gets propagated down
   * to makefiles located in subdirectories.
   */
  virtual bool IsInherited() {return true;}

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() {return "CREATE_TEST_SOURCELIST";}
  
  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Create a test driver and source list for building test programs.";
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "  CREATE_TEST_SOURCELIST(SourceListName DriverName\n"
      "                         test1 test2 test3\n"
      "                         EXTRA_INCLUDE include.h\n"
      "                         FUNCTION function)\n"
      "A test driver is a program that links together many small tests into "
      "a single executable.  This is useful when building static executables "
      "with large libraries to shrink the total required size.  "
      "The list of source files "
      "needed to build the testdriver will be in SourceListName.  "
      "DriverName is the name of the test driver program.  The rest of "
      "the arguments consist of a list of test source files, can be "
      "; separated.  Each test source file should have a function in it that "
      "is the same name as the file with no extension (foo.cxx should have "
      "int foo();) DriverName will be able to call each of the tests by "
      "name on the command line. If EXTRA_INCLUDE is specified, then the "
      "next argument is included into the generated file. If FUNCTION is "
      "specified, then the next argument is taken as a function name that "
      "is passed a pointer to ac and av.  This can be used to add extra "
      "command line processing to each test. ";
    }
  
  cmTypeMacro(cmCreateTestSourceList, cmCommand);
};



#endif
