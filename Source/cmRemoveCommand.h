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
#ifndef cmRemoveCommand_h
#define cmRemoveCommand_h

#include "cmCommand.h"

/** \class cmRemoveCommand
 * \brief Set a CMAKE variable
 *
 * cmRemoveCommand sets a variable to a value with expansion.  
 */
class cmRemoveCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone() 
    {
    return new cmRemoveCommand;
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
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() {return "REMOVE";}
  
  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Remove a value from a list in a variable.";
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "  REMOVE(VAR VALUE VALUE ...)\n"
      "Removes VALUE from the variable VAR.  "
      "This is typically used to remove entries from a vector "
      "(e.g. semicolon separated list).  VALUE is expanded.";
    }
  
  cmTypeMacro(cmRemoveCommand, cmCommand);
};



#endif
