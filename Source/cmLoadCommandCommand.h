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
#ifndef cmLoadCommandCommand_h
#define cmLoadCommandCommand_h

#include "cmCommand.h"

/** \class cmLoadCommandCommand
 * \brief Load in a Command plugin
 *
 * cmLoadCommandCommand loads a command into CMake
 */
class cmLoadCommandCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone() 
    {
    return new cmLoadCommandCommand;
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
  virtual const char* GetName() {return "LOAD_COMMAND";}
  
  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Load a command into a running CMake.";
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "  LOAD_COMMAND(COMMAND_NAME <loc1> [loc2 ...])\n"
      "The given locations are searched for a library whose name is "
      "cmCOMMAND_NAME.  If found, it is loaded as a module and the command "
      "is added to the set of available CMake commands.  Usually, TRY_COMPILE "
      "is used before this command to compile the module.";
    }
  
  cmTypeMacro(cmLoadCommandCommand, cmCommand);
};



#endif
