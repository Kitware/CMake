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
#ifndef cmCMakePolicyCommand_h
#define cmCMakePolicyCommand_h

#include "cmCommand.h"

/** \class cmCMakePolicyCommand
 * \brief Set how CMake should handle policies
 *
 * cmCMakePolicyCommand sets how CMake should deal with backwards 
 * compatibility policies.   
 */
class cmCMakePolicyCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone() 
    {
    return new cmCMakePolicyCommand;
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
  virtual const char* GetName() {return "cmake_policy";}
  
 /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Set how CMake should handle policies.";
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "  cmake_policy(NEW id)\n"
      "  cmake_policy(OLD id)\n"
      "  cmake_policy(VERSION version)\n"
      "  cmake_policy(PUSH)\n"
      "  cmake_policy(POP)\n"
      "The first two forms of this command sets a specified policy to "
      "use the OLD or NEW implementation respectively. For example "
      "if a new policy is created in CMake 2.6 then you could use "
      "this command to tell the running CMake to use the OLD behavior "
      "(before the change in 2.6) or the NEW behavior.\n"
      "The third form of this command indicates that the CMake List file "
      "has been written to the specified version of CMake and to the "
      "policies of that version of CMake. All policies introduced in "
      "the specified version of CMake or earlier will be set to NEW. "
      "All policies introduced after the specified version of CMake will "
      "be set to WARN (WARN is like OLD but also produces a warning) if "
      "that is possible.\n"
      "The last two forms of this command push and pop the current "
      "handling of policies in CMake. This is useful when mixing multiple "
      "projects that may have been written to different versions of CMake."
      ;
    }
  
  cmTypeMacro(cmCMakePolicyCommand, cmCommand);
};



#endif
