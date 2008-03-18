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
    return "Manage CMake Policy settings.";
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "As CMake evolves it is sometimes necessary to change existing "
      "behavior in order to fix bugs or improve implementations of "
      "existing features.  "
      "The CMake Policy mechanism is designed to help keep existing projects "
      "building as new versions of CMake introduce changes in behavior.  "
      "Each new policy (behavioral change) is given an identifier of "
      "the form \"CMP<NNNN>\" where \"<NNNN>\" is an integer index.  "
      "Documentation associated with each policy describes the OLD and NEW "
      "behavior and the reason the policy was introduced.  "
      "Projects may set each policy to select the desired behavior.  "
      "When CMake needs to know which behavior to use it checks for "
      "a setting specified by the project.  "
      "If no setting is available the OLD behavior is assumed and a warning "
      "is produced requesting that the policy be set.\n"
      "The cmake_policy command is used to set policies to OLD or NEW "
      "behavior.  "
      "While setting policies individually is supported, we encourage "
      "projects to set policies based on CMake versions.\n"
      "  cmake_policy(VERSION major.minor[.patch])\n"
      "Specify that the current CMake list file is written for the "
      "given version of CMake.  "
      "All policies introduced in the specified version or earlier "
      "will be set to use NEW behavior.  "
      "All policies introduced after the specified version will be reset "
      "to use OLD behavior with a warning.  "
      "This effectively requests behavior preferred as of a given CMake "
      "version and tells newer CMake versions to warn about their new "
      "policies.  "
      "The policy version specified must be at least 2.4 or the command "
      "will report an error.  "
      "In order to get compatibility features supporting versions earlier "
      "than 2.4 see documentation of policy CMP0001."
      "\n"
      "  cmake_policy(SET CMP<NNNN> NEW)\n"
      "  cmake_policy(SET CMP<NNNN> OLD)\n"
      "Tell CMake to use the OLD or NEW behavior for a given policy.  "
      "Projects depending on the old behavior of a given policy may "
      "silence a policy warning by setting the policy state to OLD.  "
      "Alternatively one may fix the project to work with the new behavior "
      "and set the policy state to NEW."
      "\n"
      "  cmake_policy(PUSH)\n"
      "  cmake_policy(POP)\n"
      "Push and pop the current policy setting state on a stack.  "
      "Each PUSH must have a matching POP.  "
      "This is useful when mixing multiple projects, subprojects, and "
      "files included from external projects that may each have been "
      "written for a different version of CMake.  "
      "Each subdirectory entered by the project automatically pushes "
      "a new level on the stack to isolate the subdirectories from "
      "their parents.";
    }
  
  cmTypeMacro(cmCMakePolicyCommand, cmCommand);
private:
  bool HandleSetMode(std::vector<std::string> const& args);
  bool HandleVersionMode(std::vector<std::string> const& args);
};



#endif
