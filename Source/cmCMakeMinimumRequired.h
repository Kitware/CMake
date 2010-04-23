/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
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
      "  cmake_minimum_required(VERSION major[.minor[.patch[.tweak]]]\n"
      "                         [FATAL_ERROR])\n"
      "If the current version of CMake is lower than that required "
      "it will stop processing the project and report an error.  "
      "When a version higher than 2.4 is specified the command implicitly "
      "invokes\n"
      "  cmake_policy(VERSION major[.minor[.patch[.tweak]]])\n"
      "which sets the cmake policy version level to the version specified.  "
      "When version 2.4 or lower is given the command implicitly invokes\n"
      "  cmake_policy(VERSION 2.4)\n"
      "which enables compatibility features for CMake 2.4 and lower.\n"
      "The FATAL_ERROR option is accepted but ignored by CMake 2.6 "
      "and higher.  "
      "It should be specified so CMake versions 2.4 and lower fail with an "
      "error instead of just a warning.";
    }
  
  cmTypeMacro(cmCMakeMinimumRequired, cmCommand);

private:
  std::vector<std::string> UnknownArguments;
  bool EnforceUnknownArguments();
};



#endif
