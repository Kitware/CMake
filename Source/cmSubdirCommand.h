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
#ifndef cmSubdirCommand_h
#define cmSubdirCommand_h

#include "cmCommand.h"

/** \class cmSubdirCommand
 * \brief Specify a list of subdirectories to build.
 *
 * cmSubdirCommand specifies a list of subdirectories to process
 * by CMake. For each subdirectory listed, CMake will descend
 * into that subdirectory and process any CMakeLists.txt found.
 */
class cmSubdirCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone() 
    {
    return new cmSubdirCommand;
    }

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  virtual bool InitialPass(std::vector<std::string> const& args);

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() { return "SUBDIRS";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Add a list of subdirectories to the build.";
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "  SUBDIRS(dir1 dir2 ...)\n"
      "Add a list of subdirectories to the build. "
      "This will cause any CMakeLists.txt files in the sub directories "
      "to be processed by CMake.";
    }
  
  cmTypeMacro(cmSubdirCommand, cmCommand);
};



#endif
