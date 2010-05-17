/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
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
      "  cmake_policy(VERSION major.minor[.patch[.tweak]])\n"
      "Specify that the current CMake list file is written for the "
      "given version of CMake.  "
      "All policies introduced in the specified version or earlier "
      "will be set to use NEW behavior.  "
      "All policies introduced after the specified version will be unset.  "
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
      "  cmake_policy(GET CMP<NNNN> <variable>)\n"
      "Check whether a given policy is set to OLD or NEW behavior.  "
      "The output variable value will be \"OLD\" or \"NEW\" if the "
      "policy is set, and empty otherwise."
      "\n"
      "CMake keeps policy settings on a stack, so changes made by the "
      "cmake_policy command affect only the top of the stack.  "
      "A new entry on the policy stack is managed automatically for each "
      "subdirectory to protect its parents and siblings.  "
      "CMake also manages a new entry for scripts loaded by include() and "
      "find_package() commands except when invoked with the NO_POLICY_SCOPE "
      "option (see also policy CMP0011).  "
      "The cmake_policy command provides an interface to manage custom "
      "entries on the policy stack:\n"
      "  cmake_policy(PUSH)\n"
      "  cmake_policy(POP)\n"
      "Each PUSH must have a matching POP to erase any changes.  "
      "This is useful to make temporary changes to policy settings."
      "\n"
      "Functions and macros record policy settings when they are created "
      "and use the pre-record policies when they are invoked.  "
      "If the function or macro implementation sets policies, the changes "
      "automatically propagate up through callers until they reach the "
      "closest nested policy stack entry."
      ;
    }
  
  cmTypeMacro(cmCMakePolicyCommand, cmCommand);
private:
  bool HandleSetMode(std::vector<std::string> const& args);
  bool HandleGetMode(std::vector<std::string> const& args);
  bool HandleVersionMode(std::vector<std::string> const& args);
};



#endif
