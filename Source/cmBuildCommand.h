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
#ifndef cmBuildCommand_h
#define cmBuildCommand_h

#include "cmStandardIncludes.h"
#include "cmCommand.h"

/** \class cmBuildCommand
 * \brief Build a CMAKE variable
 *
 * cmBuildCommand sets a variable to a value with expansion.  
 */
class cmBuildCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone() 
    {
    return new cmBuildCommand;
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
  virtual const char* GetName() {return "BUILD_COMMAND";}
  
  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Determine the command line that will build this project.";
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "BUILD_COMMAND(NAME MAKECOMMAND)\n"
      "Within CMAKE set NAME to the command that will build this project from the command line using MAKECOMMAND.   MAKECOMMAND should be msdev, nmake, make or one of the end use build tools.   This command will construct the command line to use that will build all the targets in this project.   This is useful for testing systems.";
    }
  
  cmTypeMacro(cmBuildCommand, cmCommand);
};



#endif
