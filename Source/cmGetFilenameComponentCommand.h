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
#ifndef cmGetFilenameComponentCommand_h
#define cmGetFilenameComponentCommand_h

#include "cmCommand.h"

/** \class cmGetFilenameComponentCommand
 * \brief Get a specific component of a filename.
 *
 * cmGetFilenameComponentCommand is a utility command used to get the path,
 * name, extension or name without extension of a full filename. 
 */
class cmGetFilenameComponentCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone() 
    {
    return new cmGetFilenameComponentCommand;
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
  virtual bool IsInherited() { return true;  }

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() { return "GET_FILENAME_COMPONENT";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Get a specific component of a full filename.";
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "  GET_FILENAME_COMPONENT(VarName FileName\n"
      "                         PATH|ABSOLUTE|NAME|EXT|NAME_WE\n"
      "                         [CACHE])\n"
      "Set VarName to be the path (PATH), file name (NAME), file "
      "extension (EXT), file name without extension (NAME_WE) of FileName, "
      "or the full absolute (ABSOLUTE) file name without symlinks.  "
      "Note that the path is converted to Unix slashes format and has no "
      "trailing slashes. The longest file extension is always considered.  If "
      "the optional CACHE argument is specified, the result variable is "
      "added to the cache.\n"
      "  GET_FILENAME_COMPONENT(VarName FileName\n"
      "                         PROGRAM [PROGRAM_ARGS ArgVar]\n"
      "                         [CACHE])\n"
      "The program in FileName will be found in the system search path or "
      "left as a full path.  If PROGRAM_ARGS is present with PROGRAM, then "
      "any command-line arguments present in the FileName string are split "
      "from the program name and stored in ArgVar.  This is used to separate "
      "a program name from its arguments in a command line string.";
    }
  
  cmTypeMacro(cmGetFilenameComponentCommand, cmCommand);
};



#endif
