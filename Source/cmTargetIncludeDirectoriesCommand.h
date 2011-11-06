/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2011 Stephen Kelly <steveire@gmail.com>

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmTargetIncludeDirectoriesCommand_h
#define cmTargetIncludeDirectoriesCommand_h

#include "cmCommand.h"

/** \class cmTargetIncludeDirectoriesCommand
 * \brief Add include directories to the build for a particular target.
 *
 * cmIncludeDirectoriesCommand is used to specify directory locations
 * to search for included files when building a particular target.
 */
class cmTargetIncludeDirectoriesCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone()
    {
    return new cmTargetIncludeDirectoriesCommand;
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
  virtual const char* GetName() { return "target_include_directories";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation()
    {
    return "Add include directories to the build for a particular target.";
    }

  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "  target_include_directories(target [BEFORE|AFTER] "
      "                            [CONFIG_TYPE <config>] dir1 dir2 ...)\n"
      "Add the given directories to those searched by the compiler for "
      "include files when building target. By default the directories are "
      "appended onto the current list of directories for the target.  "
      "By using BEFORE or AFTER you can select between appending and "
      "prepending.  The CONFIG_TYPE can be used to specify a build type "
      "associated with the target include.";
    }

  cmTypeMacro(cmTargetIncludeDirectoriesCommand, cmCommand);

protected:
  // used internally
  void AddDirectory(const char *arg, bool before, const char *config);

private:
  cmTarget *Target;
};

#endif
