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
#ifndef cmDependenciessCommand_h
#define cmDependenciessCommand_h

#include "cmCommand.h"

/** \class cmAddDependenciesCommand
 * \brief Add a dependency to a target
 *
 * cmAddDependenciesCommand adds a dependency to a target
 */
class cmAddDependenciesCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone() 
    {
    return new cmAddDependenciesCommand;
    }

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  virtual bool InitialPass(std::vector<std::string> const& args);

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() { return "ADD_DEPENDENCIES";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Add an dependency to a target";
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "  ADD_DEPENDENCIES(target-name depend-target1\n"
      "                   depend-target2 ...)\n"
      "Add a dependency to a target.  This is only used to add dependencies "
      "between targets that cannot be inferred from the library/executable "
      "links that are specified.  Regular build dependencies are "
      "handled automatically.";
    }
  
  cmTypeMacro(cmAddDependenciesCommand, cmCommand);
};


#endif
