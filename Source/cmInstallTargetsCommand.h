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
#ifndef cmInstallTargetsCommand_h
#define cmInstallTargetsCommand_h

#include "cmStandardIncludes.h"
#include "cmCommand.h"

/** \class cmInstallTargetsCommand
 * \brief Specifies where to install some targets
 *
 * cmInstallTargetsCommand specifies the relative path where a list of
 * targets should be installed. The targets can be executables or
 * libraries.  
 */
class cmInstallTargetsCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone() 
    {
    return new cmInstallTargetsCommand;
    }

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  virtual bool InitialPass(std::vector<std::string> const& args);

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() { return "INSTALL_TARGETS";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Create install rules for targets";
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "INSTALL_TARGETS(path target target)\n"
      "Create rules to install the listed targets into the path. Path is relative to the variable PREFIX";
    }
  
  cmTypeMacro(cmInstallTargetsCommand, cmCommand);
};


#endif
