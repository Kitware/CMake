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
#ifndef cmEnableLanguageCommand_h
#define cmEnableLanguageCommand_h

#include "cmCommand.h"

/** \class cmEnableLanguageCommand
 * \brief Specify the name for this build project.
 *
 * cmEnableLanguageCommand is used to specify a name for this build project.
 * It is defined once per set of CMakeList.txt files (including
 * all subdirectories). Currently it just sets the name of the workspace
 * file for Microsoft Visual C++
 */
class cmEnableLanguageCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone() 
    {
    return new cmEnableLanguageCommand;
    }

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  virtual bool InitialPass(std::vector<std::string> const& args);
  
  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() {return "enable_language";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Enable a language (CXX/C/Fortran/etc)";
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "  enable_language(languageName [OPTIONAL] )\n"
      "This command enables support for the named language in CMake. "
      "This is the same as the project command but does not create "
      "any of the extra variables that are created by the project command. "
      "Example languages are CXX, C, Fortran.\n"
      "If OPTIONAL is used, use the CMAKE_<languageName>_COMPILER_WORKS "
      "variable to check whether the language has been enabled successfully.";
    }
  
  cmTypeMacro(cmEnableLanguageCommand, cmCommand);
};



#endif
