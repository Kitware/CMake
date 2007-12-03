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
#ifndef cmRaiseScopeCommand_h
#define cmRaiseScopeCommand_h

#include "cmCommand.h"

/** \class cmRaiseScopeCommand
 * \brief Raise the Scope a CMAKE variable one level up
 *
 * cmRaiseScopeCommand pushes the current state of a variable into
 * the scope above the current scope.
 */
class cmRaiseScopeCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone() 
    {
    return new cmRaiseScopeCommand;
    }

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  virtual bool InitialPass(std::vector<std::string> const& args);

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() {return "raise_scope";}
  
  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Raise the scope of the variables listed.";
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "  raise_scope(VAR VAR2 VAR...)\n"
      "Pushes the current state of a variable into the scope above the "
      "current scope. Each new directory or function creates a new scope. "
      "This command will push the current state of a variable into the "
      "parent directory or calling function (whichever is applicable to "
      "the case at hand)";
    }

  /**
   * This determines if the command is invoked when in script mode.
   * mark_as_advanced() will have no effect in script mode, but this will
   * make many of the modules usable in cmake/ctest scripts, (among them
   * FindUnixMake.cmake used by the CTEST_BUILD command.
  */
  virtual bool IsScriptable() { return true; }

  cmTypeMacro(cmRaiseScopeCommand, cmCommand);
};



#endif
