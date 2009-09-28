/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmCTestUpdateCommand_h
#define cmCTestUpdateCommand_h

#include "cmCTestHandlerCommand.h"

/** \class cmCTestUpdate
 * \brief Run a ctest script
 *
 * cmCTestUpdateCommand defineds the command to updates the repository.
 */
class cmCTestUpdateCommand : public cmCTestHandlerCommand
{
public:

  cmCTestUpdateCommand() {}

  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone()
    {
    cmCTestUpdateCommand* ni = new cmCTestUpdateCommand;
    ni->CTest = this->CTest;
    ni->CTestScriptHandler = this->CTestScriptHandler;
    return ni;
    }

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() { return "ctest_update";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation()
    {
    return "Update the work tree from version control.";
    }

  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "  ctest_update([SOURCE source] [RETURN_VALUE res])\n"
      "Updates the given source directory and stores results in Update.xml. "
      "If no SOURCE is given, the CTEST_SOURCE_DIRECTORY variable is used. "
      "The RETURN_VALUE option specifies a variable in which to store the "
      "result, which is the number of files updated or -1 on error."
      ;
    }

  cmTypeMacro(cmCTestUpdateCommand, cmCTestHandlerCommand);

protected:
  cmCTestGenericHandler* InitializeHandler();
};


#endif
