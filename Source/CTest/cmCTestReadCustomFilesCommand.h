/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc.

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmCTestReadCustomFilesCommand_h
#define cmCTestReadCustomFilesCommand_h

#include "cmCTestCommand.h"

/** \class cmCTestReadCustomFiles
 * \brief Run a ctest script
 *
 * cmLibrarysCommand defines a list of executable (i.e., test)
 * programs to create.
 */
class cmCTestReadCustomFilesCommand : public cmCTestCommand
{
public:
  cmCTestReadCustomFilesCommand() {}

  /**
   * This is a virtual constructor for the command.
   */
  cmCommand* Clone() CM_OVERRIDE
  {
    cmCTestReadCustomFilesCommand* ni = new cmCTestReadCustomFilesCommand;
    ni->CTest = this->CTest;
    return ni;
  }

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  bool InitialPass(std::vector<std::string> const& args,
                   cmExecutionStatus& status) CM_OVERRIDE;

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  std::string GetName() const CM_OVERRIDE { return "ctest_read_custom_files"; }

  cmTypeMacro(cmCTestReadCustomFilesCommand, cmCTestCommand);
};

#endif
