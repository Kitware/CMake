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
#ifndef cmSourceGroupCommand_h
#define cmSourceGroupCommand_h

#include "cmStandardIncludes.h"
#include "cmCommand.h"

/** \class cmSourceGroupCommand
 * \brief Adds a cmSourceGroup to the cmMakefile.
 *
 * cmSourceGroupCommand is used to define cmSourceGroups which split up
 * source files in to named, organized groups in the generated makefiles.
 */
class cmSourceGroupCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone() 
    {
    return new cmSourceGroupCommand;
    }

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  virtual bool InitialPass(std::vector<std::string> const& args);
  
  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() {return "SOURCE_GROUP";}

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
    return "Define a grouping for sources in the makefile.";
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "  SOURCE_GROUP(name [REGULAR_EXPRESSION regex] [FILES src1 src2 ...])\n"
      "Defines a group into which sources will be placed in project files.  "
      "This is mainly used to setup file tabs in Visual Studio.  "
      "Any file whose name is listed or matches the regular expression will "
      "be placed in this group.  If a file matches multiple groups, the LAST "
      "group that explicitly lists the file will be favored, if any.  If no "
      "group explicitly lists the file, the LAST group whose regular "
      "expression matches the file will be favored.";
    }
  
  cmTypeMacro(cmSourceGroupCommand, cmCommand);
};



#endif
