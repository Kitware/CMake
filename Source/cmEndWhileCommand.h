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
#ifndef cmEndWhileCommand_h
#define cmEndWhileCommand_h

#include "cmCommand.h"

/** \class cmEndWhileCommand
 * \brief ends an if block
 *
 * cmEndWhileCommand ends an if block
 */
class cmEndWhileCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone() 
    {
    return new cmEndWhileCommand;
    }

  /**
   * Override cmCommand::InvokeInitialPass to get arguments before
   * expansion.
   */
  virtual bool InvokeInitialPass(std::vector<cmListFileArgument> const&);
  
  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  virtual bool InitialPass(std::vector<std::string> const&) {return false;}

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
  virtual const char* GetName() { return "ENDWHILE";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Ends a list of commands in a WHILE block.";
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "  ENDWHILE(expression)\n"
      "See WHILE command.";
    }
  
  cmTypeMacro(cmEndWhileCommand, cmCommand);
};


#endif
