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
#ifndef cmBuildNameCommand_h
#define cmBuildNameCommand_h

#include "cmStandardIncludes.h"
#include "cmCommand.h"

/** \class cmBuildNameCommand
 * \brief BuildName a CMAKE variable
 *
 * cmBuildNameCommand sets a variable to a value with expansion.  
 */
class cmBuildNameCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone() 
    {
    return new cmBuildNameCommand;
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
  virtual const char* GetName() {return "BUILD_NAME";}
  
  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Depricated command.  Use ${CMAKE_SYSTEM} and ${CMAKE_CXX_COMPILER} instead..";
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "BUILD_NAME(NAME)\n"
      "Within CMAKE sets NAME to the build type.";
    }
  
  cmTypeMacro(cmBuildNameCommand, cmCommand);
};



#endif
