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
#ifndef cmCMakeMinimumRequired_h
#define cmCMakeMinimumRequired_h

#include "cmCommand.h"

/** \class cmCMakeMinimumRequired
 * \brief Build a CMAKE variable
 *
 * cmCMakeMinimumRequired sets a variable to a value with expansion.  
 */
class cmCMakeMinimumRequired : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone() 
    {
    return new cmCMakeMinimumRequired;
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
   * This determines if the command is invoked when in script mode.
   */
  virtual bool IsScriptable() { return true; }

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() {return "CMAKE_MINIMUM_REQUIRED";}
  
  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Set the minimum required version of cmake for a project.";
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "CMAKE_MINIMUM_REQUIRED(VERSION versionNumber)\n"
      "Let cmake know that the project requires a certain version of a cmake, or newer.  CMake will also try to backwards compatible to the version of cmake specified, if a newer version of cmake is running.";
    }
  
  cmTypeMacro(cmCMakeMinimumRequired, cmCommand);
};



#endif
