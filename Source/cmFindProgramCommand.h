/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmFindProgramCommand_h
#define cmFindProgramCommand_h

#include "cmFindBase.h"

/** \class cmFindProgramCommand
 * \brief Define a command to search for an executable program.
 *
 * cmFindProgramCommand is used to define a CMake variable
 * that specifies an executable program. The command searches
 * in the current path (e.g., PATH environment variable) for
 * an executable that matches one of the supplied names.
 */
class cmFindProgramCommand : public cmFindBase
{
public:
  cmFindProgramCommand();
  /**
   * This is a virtual constructor for the command.
   */
  cmCommand* Clone() CM_OVERRIDE { return new cmFindProgramCommand; }

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  bool InitialPass(std::vector<std::string> const& args,
                   cmExecutionStatus& status) CM_OVERRIDE;

  /**
   * This determines if the command is invoked when in script mode.
   */
  bool IsScriptable() const CM_OVERRIDE { return true; }

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  std::string GetName() const CM_OVERRIDE { return "find_program"; }

  cmTypeMacro(cmFindProgramCommand, cmFindBase);

private:
  std::string FindProgram();
  std::string FindNormalProgram();
  std::string FindNormalProgramDirsPerName();
  std::string FindNormalProgramNamesPerDir();
  std::string FindAppBundle();
  std::string GetBundleExecutable(std::string bundlePath);
};

#endif
