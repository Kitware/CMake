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
#ifndef cmFileCommand_h
#define cmFileCommand_h

#include "cmStandardIncludes.h"
#include "cmCommand.h"

/** \class cmFileCommand
 * \brief Command for manipulation of files
 *
 */
class cmFileCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone() 
    {
    return new cmFileCommand;
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
  virtual const char* GetName() { return "FILE";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "File manipulation command.";
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "  FILE(WRITE filename \"message to write\"... )\n"
      "  FILE(APPEND filename \"message to write\"... )\n"
      "  FILE(READ filename variable)\n"
      "WRITE will write a message into a file called 'filename'. It "
      "overwrites the file if it already exists, and creates the file "
      "if it does not exists.\n\n"
      "APPEND will write a message into a file same as WRITE, except "
      "it will append it to the end of the file\n\n"
      "READ will read the content of a file and store it into a "
      "variable.\n\n"; }
  
  cmTypeMacro(cmFileCommand, cmCommand);

protected:
  bool HandleWriteCommand(std::vector<std::string> const& args, bool append);
  bool HandleReadCommand(std::vector<std::string> const& args);
};


#endif
