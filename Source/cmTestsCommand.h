/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


  Copyright (c) 2000 National Library of Medicine
  All rights reserved.

  See COPYRIGHT.txt for copyright details.

=========================================================================*/
#ifndef cmTestsCommand_h
#define cmTestsCommand_h

#include "cmStandardIncludes.h"
#include "cmCommand.h"

/** \class cmTestsCommand
 * \brief Specify a list of executables to build and which are 
 *        identified as tests.
 *
 * cmTestsCommand specifies a list of executables to be built by CMake.
 * These executables are identified as tests. This command is similar to
 * the EXECUTABLES() command.
 *
 * \sa cmExecutablesCommand
 */
class cmTestsCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone() 
    {
    return new cmTestsCommand;
    }

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  virtual bool Invoke(std::vector<std::string>& args);

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() {return "TESTS";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Add a list of executables files that are run as tests.";
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "TESTS(file1 file2 ...)";
    }
  
  cmTypeMacro(cmTestsCommand, cmCommand);
};



#endif
