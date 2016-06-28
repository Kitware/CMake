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
  cmCommand* Clone() CM_OVERRIDE
  {
    cmCTestUpdateCommand* ni = new cmCTestUpdateCommand;
    ni->CTest = this->CTest;
    ni->CTestScriptHandler = this->CTestScriptHandler;
    return ni;
  }

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  std::string GetName() const CM_OVERRIDE { return "ctest_update"; }

  cmTypeMacro(cmCTestUpdateCommand, cmCTestHandlerCommand);

protected:
  cmCTestGenericHandler* InitializeHandler() CM_OVERRIDE;
};

#endif
