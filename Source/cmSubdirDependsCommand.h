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
#ifndef cmSubdirDependsCommand_h
#define cmSubdirDependsCommand_h

#include "cmCommand.h"

/** \class cmSubdirDependsCommand
 * \brief Legacy command.  Do not use.
 *
 * cmSubdirDependsCommand has been left in CMake for compatability with
 * projects already using it.  Its functionality in supporting parallel
 * builds is now automatic.  The command does not do anything.
 */
class cmSubdirDependsCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone() 
    {
    return new cmSubdirDependsCommand;
    }

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  virtual bool InitialPass(std::vector<std::string> const& args);

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() { return "SUBDIR_DEPENDS";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Legacy command.  Does nothing.";
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "  SUBDIR_DEPENDS(subdir dep1 dep2 ...)\n"
      "Does not do anything.  This command used to help projects order "
      "parallel builds correctly.  This functionality is now automatic.";
    }
  
  cmTypeMacro(cmSubdirDependsCommand, cmCommand);
};



#endif
