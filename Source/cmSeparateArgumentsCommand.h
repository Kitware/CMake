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
#ifndef cmSeparateArgumentsCommand_h
#define cmSeparateArgumentsCommand_h

#include "cmCommand.h"

/** \class cmSeparateArgumentsCommand
 * \brief SeparateArguments a CMAKE variable
 *
 * cmSeparateArgumentsCommand sets a variable to a value with expansion.  
 */
class cmSeparateArgumentsCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone() 
    {
    return new cmSeparateArgumentsCommand;
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
  virtual const char* GetName() {return "SEPARATE_ARGUMENTS";}
  
  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Split space separated arguments into a semi-colon separated list.";
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "  SEPARATE_ARGUMENTS(VARIABLE)\n"
      "Convert the value of VARIABLE to a semi-colon separated list.  "
      "All spaces are replaced with ';'.  This helps with generating "
      "command lines.";
    }
  
  cmTypeMacro(cmSeparateArgumentsCommand, cmCommand);
};



#endif
