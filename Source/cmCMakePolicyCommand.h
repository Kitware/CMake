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
    return "Manage CMake policy settings.";
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "  cmake_policy(VERSION major.minor[.patch])\n"
      "Specify that the current CMake list file is written for the "
      "given version of CMake.  "
      "All policies introduced in the specified version or earlier "
      "will be set NEW.  "
      "All policies introduced after the specified version will be set "
      "to WARN, which is like OLD but also produces a warning.  "
      "This effectively requests behavior preferred as of a given CMake "
      "version and tells newer CMake versions to warn about their new "
      "policies.  "
      "The policy version specified must be at least 2.4 or the command "
      "will report an error.  "
      "In order to get compatibility features supporting versions earlier "
      "than 2.4 see documentation of policy CMP_0001."
      "\n"
      "  cmake_policy(SET <CMP_NNNN> NEW)\n"
      "  cmake_policy(SET <CMP_NNNN> OLD)\n"
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
      "written for a different version of CMake."
      ;
    }
  
  cmTypeMacro(cmCMakePolicyCommand, cmCommand);
private:
  bool HandleSetMode(std::vector<std::string> const& args);
  bool HandleVersionMode(std::vector<std::string> const& args);
};



#endif
