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
  virtual bool InitialPass(std::vector<std::string> const& args,
                           cmExecutionStatus &status);

  /**
   * This determines if the command is invoked when in script mode.
   */
  virtual bool IsScriptable() { return true; }

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() {return "cmake_minimum_required";}
  
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
      "  cmake_minimum_required(VERSION major[.minor[.patch]]\n"
      "                         [FATAL_ERROR])\n"
      "If the current version of CMake is lower than that required "
      "it will stop processing the project and report an error.\n"
      "The FATAL_ERROR option is accepted but ignored.  It is left from "
      "CMake versions 2.4 and lower in which failure to meet the minimum "
      "version was a warning by default.";
    }
  
  cmTypeMacro(cmCMakeMinimumRequired, cmCommand);
};



#endif
