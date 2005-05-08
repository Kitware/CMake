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
#ifndef cmCTestBuildCommand_h
#define cmCTestBuildCommand_h

#include "cmCTestCommand.h"

class cmGlobalGenerator;

/** \class cmCTestBuild
 * \brief Run a ctest script
 *
 * cmCTestBuildCommand defineds the command to build the project.
 */
class cmCTestBuildCommand : public cmCTestCommand
{
public:

  cmCTestBuildCommand();
  ~cmCTestBuildCommand();
  
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone() 
    {
    cmCTestBuildCommand* ni = new cmCTestBuildCommand;
    ni->m_CTest = this->m_CTest;
    ni->m_CTestScriptHandler = this->m_CTestScriptHandler;
    return ni;
    }

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  virtual bool InitialPass(std::vector<std::string> const& args);

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() { return "CTEST_BUILD";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Builds the repository.";
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "  CTEST_BUILD([BUILD build_dir] [RETURN_VALUE res])\n"
      "Builds the given build directory and stores results in Build.xml.";
    }

  cmTypeMacro(cmCTestBuildCommand, cmCTestCommand);

  cmGlobalGenerator* m_GlobalGenerator;
};


#endif
