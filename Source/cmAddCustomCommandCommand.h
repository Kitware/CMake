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
      "  ADD_CUSTOM_COMMAND(OUTPUT output1 [output2 ...]\n"
      "                     COMMAND command1 [ARGS] [args1...]\n"
      "                     [COMMAND command2 [ARGS] [args2...] ...]\n"
      "                     [MAIN_DEPENDENCY depend]\n"
      "                     [DEPENDS [depends...]]\n"
      "                     [WORKING_DIRECTORY dir]\n"
      "                     [COMMENT comment] [VERBATIM])\n"
      "This defines a new command that can be executed during the build "
      "process. The outputs named should be listed as source files in the "
      "target for which they are to be generated. "
      "Note that MAIN_DEPENDENCY is completely optional and is "
      "used as a suggestion to visual studio about where to hang the "
      "custom command. In makefile terms this creates a new target in the "
      "following form:\n"
      "  OUTPUT: MAIN_DEPENDENCY DEPENDS\n"
      "          COMMAND\n"
      "If more than one command is specified they will be executed in order. "
      "The optional ARGS argument is for backward compatibility and will be "
      "ignored.\n"
      "The second signature adds a custom command to a target "
      "such as a library or executable. This is useful for "
      "performing an operation before or after building the target:\n"
      "  ADD_CUSTOM_COMMAND(TARGET target\n"
      "                     PRE_BUILD | PRE_LINK | POST_BUILD\n"
      "                     COMMAND command1 [ARGS] [args1...]\n"
      "                     [COMMAND command2 [ARGS] [args2...] ...]\n"
      "                     [WORKING_DIRECTORY dir]\n"
      "                     [COMMENT comment] [VERBATIM])\n"
      "This defines a new command that will be associated with "
      "building the specified target. When the command will "
      "happen is determined by which of the following is specified:\n"
      "  PRE_BUILD - run before all other dependencies\n"
      "  PRE_LINK - run after other dependencies\n"
      "  POST_BUILD - run after the target has been built\n"
      "Note that the PRE_BUILD option is only supported on Visual "
      "Studio 7 or later. For all other generators PRE_BUILD "
      "will be treated as PRE_LINK. "
      "If WORKING_DIRECTORY is specified the command will be executed "
      "in the directory given.\n"
      "If VERBATIM is given then all the arguments to the commands will be "
      "passed exactly as specified no matter the build tool used. "
      "Note that one level of escapes is still used by the CMake language "
      "processor before ADD_CUSTOM_TARGET even sees the arguments. "
      "Use of VERBATIM is recommended as it enables correct behavior. "
      "When VERBATIM is not given the behavior is platform specific. "
      "In the future VERBATIM may be enabled by default. The only reason "
      "it is an option is to preserve compatibility with older CMake code.\n"
      "If the output of the custom command is not actually "
      "created as a file on disk it should be marked as SYMBOLIC with "
      "SET_SOURCE_FILES_PROPERTIES.";
    }
  
  cmTypeMacro(cmAddCustomCommandCommand, cmCommand);
protected:
  bool CheckOutputs(const std::vector<std::string>& outputs);
};


#endif
