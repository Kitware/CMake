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
#ifndef cmProjectCommand_h
#define cmProjectCommand_h

#include "cmCommand.h"

/** \class cmProjectCommand
 * \brief Specify the name for this build project.
 *
 * cmProjectCommand is used to specify a name for this build project.
 * It is defined once per set of CMakeList.txt files (including
 * all subdirectories). Currently it just sets the name of the workspace
 * file for Microsoft Visual C++
 */
class cmProjectCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone() 
    {
    return new cmProjectCommand;
    }

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  virtual bool InitialPass(std::vector<std::string> const& args);
  
  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() {return "PROJECT";}

  /**
   * This determines if the command gets propagated down
   * to makefiles located in subdirectories.
   */
  virtual bool IsInherited() 
    {
    return true;
    }

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Set a name for the entire project.";
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "  PROJECT(projectname [CXX] [C] [Java])\n"
      "Sets the name of the project.  "
      "This creates the variables projectname_BINARY_DIR and projectname_SOURCE_DIR.  "
      "Optionally you can specify which languages your project supports.  "
      "By default all languages are supported.  If you do not have a C++ compiler, but want" 
      " to build a c program with cmake, then use this option.";
    }
  
  cmTypeMacro(cmProjectCommand, cmCommand);
};



#endif
