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
#ifndef cmWriteFileCommand_h
#define cmWriteFileCommand_h

#include "cmStandardIncludes.h"
#include "cmCommand.h"

/** \class cmWriteFileCommand
 * \brief Writes a message to a file
 *
 */
class cmWriteFileCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone() 
    {
    return new cmWriteFileCommand;
    }

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  virtual bool InitialPass(std::vector<std::string> const& args);

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() { return "WRITE_FILE";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Write a message to a file.";
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "  WRITE_FILE(filename \"message to write\"... [APPEND])\n"
      "The first argument is the file name, the rest of the arguments are "
      "messages to write. If the argument APPEND is specified, then "
      "the message will be appended.";
    }
  
  cmTypeMacro(cmWriteFileCommand, cmCommand);
};


#endif
