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
#ifndef cmAddCustomCommandCommand_h
#define cmAddCustomCommandCommand_h

#include "cmStandardIncludes.h"
#include "cmCommand.h"

/** \class cmAddCustomCommandCommand
 * \brief 
 *
 *  cmAddCustomCommandCommand defines a new command (rule) that can
 *  be executed within the build process
 *
 */

class cmAddCustomCommandCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone() 
    {
    return new cmAddCustomCommandCommand;
    }

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  virtual bool InitialPass(std::vector<std::string> const& args);

  /**
   * This determines if the command gets propagated down
   * to makefiles located in subdirectories.
   */
  virtual bool IsInherited() {return true;}

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() {return "ADD_CUSTOM_COMMAND";}
  
  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Add a custom build rule to the generated build system.";
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "There are two main signatures for ADD_CUSTOM_COMMAND "
      "The first signature is for adding a custom command "
      "to produce an output.\n" 
      "  ADD_CUSTOM_COMMAND(OUTPUT result\n"
      "                     COMMAND command\n"
      "                     [ARGS [args...]]\n"
      "                     [MAIN_DEPENDENCY depend]\n"
      "                     [DEPENDS [depends...]]\n"
      "                     [COMMENT comment])\n"
      "This defines a new command that can be executed during the build "
      "process. Note that MAIN_DEPENDENCY is completely optional and is "
      "used as a suggestion to visual studio about where to hang the "
      "custom command In makefile terms this creates a new target in the "
      "following form:\n"
      "  OUTPUT: MAIN_DEPENDENCY DEPENDS\n"
      "          COMMAND ARGS\n"
      "\n"
      "The second signature adds a custom command to a target "
      "such as a library or executable. This is useful for "
      "performing an operation before or after building the target "
      "\n"
      "  ADD_CUSTOM_COMMAND(TARGET target\n"
      "                     PRE_BUILD | PRE_LINK | POST_BUILD\n"
      "                     COMMAND command\n"
      "                     [ARGS [args...]]\n"
      "                     [COMMENT comment])\n"
      "This defines a new command that will be associated with "
      "building the specified target. When the command will "
      "happen is determined by whether you specify\n"
      "PRE_BUILD - run before all other dependencies\n"
      "PRE_LINK - run after other dependencies\n"
      "POST_BUILD - run after the target has been built\n";
    }
  
  cmTypeMacro(cmAddCustomCommandCommand, cmCommand);
};


#endif
