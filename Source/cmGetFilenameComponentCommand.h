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
      "GET_FILENAME_COMPONENT(VarName FileName PATH|NAME|EXT|NAME_WE|PROGRAM [PROGRAM_ARGS ArgVarName] [CACHE])\n"
      "Set VarName to be the path (PATH), file name (NAME), file "
      "extension (EXT) or file name without extension (NAME_WE) of FileName.\n"
      "Note that the path is converted to Unix slashes format and has no "
      "trailing slashes. The longest file extension is always considered.\n"
      "Warning: as a utility command, the resulting value is not put in the "
      "cache but in the definition list, unless you add the optional CACHE "
      "parameter."
      "For PROGRAM, the program in FileName will be found in the path or if it is "
      "a full path.   If PROGRAM_ARGS is present with PROGRAM, then the arguments "
      "are split from the program.   This is used to separate a program from its "
      "arguments.";
    }
  
  cmTypeMacro(cmGetFilenameComponentCommand, cmCommand);
};



#endif
