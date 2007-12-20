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
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() {return "build_command";}
  
  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Get the command line that will build this project.";
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "  build_command(<variable> <makecommand>)\n"
      "Sets the given <variable> to a string containing the command that "
      "will build this project from the root of the build tree using the "
      "build tool given by <makecommand>.  <makecommand> should be msdev, "
      "nmake, make or one of the end user build tools.  "
      "This is useful for configuring testing systems.";
    }
  
  cmTypeMacro(cmBuildCommand, cmCommand);
};



#endif
