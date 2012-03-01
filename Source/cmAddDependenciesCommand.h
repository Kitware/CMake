/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmDependenciessCommand_h
#define cmDependenciessCommand_h

#include "cmCommand.h"

/** \class cmAddDependenciesCommand
 * \brief Add a dependency to a target
 *
 * cmAddDependenciesCommand adds a dependency to a target
 */
class cmAddDependenciesCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone() 
    {
    return new cmAddDependenciesCommand;
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
  virtual const char* GetName() const { return "add_dependencies";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() const
    {
    return "Add a dependency between top-level targets.";
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation() const
    {
    return
      "  add_dependencies(target-name depend-target1\n"
      "                   depend-target2 ...)\n"
      "Make a top-level target depend on other top-level targets.  A "
      "top-level target is one created by ADD_EXECUTABLE, ADD_LIBRARY, "
      "or ADD_CUSTOM_TARGET.  Adding dependencies with this command "
      "can be used to make sure one target is built before another target.  "
      "Dependencies added to an IMPORTED target are followed transitively "
      "in its place since the target itself does not build.  "
      "See the DEPENDS option of ADD_CUSTOM_TARGET "
      "and ADD_CUSTOM_COMMAND for adding file-level dependencies in custom "
      "rules.  See the OBJECT_DEPENDS option in "
      "SET_SOURCE_FILES_PROPERTIES to add file-level dependencies to object "
      "files.";
    }
  
  cmTypeMacro(cmAddDependenciesCommand, cmCommand);
};


#endif
