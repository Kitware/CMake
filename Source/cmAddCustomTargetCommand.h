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
#ifndef cmAddCustomTargetCommand_h
#define cmAddCustomTargetCommand_h

#include "cmCommand.h"

/** \class cmAddCustomTargetCommand
 * \brief Command that adds a target to the build system.
 *
 * cmAddCustomTargetCommand adds an extra target to the build system.
 * This is useful when you would like to add special
 * targets like "install,", "clean," and so on.
 */
class cmAddCustomTargetCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone() 
    {
    return new cmAddCustomTargetCommand;
    }

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  virtual bool InitialPass(std::vector<std::string> const& args);
  
  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() 
    {return "ADD_CUSTOM_TARGET";}
  
  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Add a target with no output so it will always be built.";
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "  ADD_CUSTOM_TARGET(Name [ALL] [command1 [args1...]]\n"
      "                    [COMMAND command2 [args2...] ...]\n"
      "                    [DEPENDS depend depend depend ... ])\n"
      "                    [WORKING_DIRECTORY dir]\n"
      "Adds a target with the given name that executes the given commands. "
      "The target has no output file and is ALWAYS CONSIDERED OUT OF DATE "
      "even if the commands try to create a file with the name of the "
      "target. Use ADD_CUSTOM_COMMAND to generate a file with dependencies. "
      "By default nothing depends on the custom target. Use "
      "ADD_DEPENDENCIES to add dependencies to or from other targets. "
      "If the ALL option is specified "
      "it indicates that this target should be added to the default build "
      "target so that it will be run every time "
      "(the command cannot be called ALL). "
      "The command and arguments are optional and if not specified an "
      "empty target will be created. "
      "If WORKING_DIRECTORY is set, then the command will be run in that "
      "directory. "
      "Dependencies listed with the DEPENDS argument may reference files "
      "and outputs of custom commands created with ADD_CUSTOM_COMMAND.";
    }
  
  cmTypeMacro(cmAddCustomTargetCommand, cmCommand);
};

#endif
