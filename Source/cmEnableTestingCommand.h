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
#ifndef cmEnableTestingCommand_h
#define cmEnableTestingCommand_h

#include "cmCommand.h"

/** \class cmEnableTestingCommand
 * \brief Enable testing for this directory and below.
 *
 * Produce the output testfile. This produces a file in the build directory
 * called CMakeTestfile with a syntax similar to CMakeLists.txt.  It contains
 * the SUBDIRS() and ADD_TEST() commands from the source CMakeLists.txt
 * file with CMake variables expanded.  Only the subdirs and tests
 * within the valid control structures are replicated in Testfile
 * (i.e. SUBDIRS() and ADD_TEST() commands within IF() commands that are
 * not entered by CMake are not replicated in Testfile).
 * Note that Dart expects to find this file in the build directory root; 
 * therefore, this command should be in the source directory root too.
 */
class cmEnableTestingCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone() 
    {
    return new cmEnableTestingCommand;
    }

  /**
   * This determines if the command gets propagated down
   * to makefiles located in subdirectories.
   */
  virtual bool IsInherited() {return true;}

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  virtual bool InitialPass(std::vector<std::string> const&) {return true;}

  /**
   * This is called at the end after all the information
   * specified by the command is accumulated. Most commands do
   * not implement this method.  At this point, reading and
   * writing to the cache can be done.
   */
  virtual void FinalPass();

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() { return "ENABLE_TESTING";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Enable testing for current directory and below.";
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "  ENABLE_TESTING()\n"
      "Enables testing for this directory and below.  "
      "See also the ADD_TEST command.  Note that Dart expects to find "
      "a test file in the build directory root.  Therefore, this command "
      "should be in the source directory root too.";
    }
  
  cmTypeMacro(cmEnableTestingCommand, cmCommand);
};


#endif
