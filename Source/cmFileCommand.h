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
   * This determines if the command is invoked when in script mode.
   */
  virtual bool IsScriptable() { return true; }

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
      "  FILE(GLOB variable [globbing expressions]...)\n"
      "  FILE(GLOB_RECURSE variable [globbing expressions]...)\n"
      "  FILE(MAKE_DIRECTORY [directory]...)\n"
      "WRITE will write a message into a file called 'filename'. It "
      "overwrites the file if it already exists, and creates the file "
      "if it does not exists.\n"
      "APPEND will write a message into a file same as WRITE, except "
      "it will append it to the end of the file\n"
      "NOTE: When using FILE WRITE and FILE APPEND, the produced file "
      "cannot be used as an input to CMake (CONFIGURE_FILE, source file ...) "
      "because it will lead to infinite loop. Use CONFIGURE_FILE if you "
      "want to generate input files to CMake.\n"
      "READ will read the content of a file and store it into the "
      "variable.\n"
      "GLOB will generate a list of all files that match the globbing "
      "expressions and store it into the variable. Globbing expressions "
      "are similar to regular expressions, but much simpler..\n"
      "Examples of globbing expressions:\n"
      "   *.cxx      - match all files with extension cxx\n"
      "   *.vt?      - match all files with extension vta, vtb, ... vtz\n"
      "   f[3-5].txt - match files f3.txt, f4.txt, f5.txt\n"
      "GLOB_RECURSE will generate similar list as the regular GLOB, except "
      "it will traverse all the subdirectories of the matched directory and "
      "match the files.\n"
      "Example of recursive globbing:\n"
      "   /dir/*.py  - match all python files /dir and subdirectories\n"
      "MAKE_DIRECTORY will create a directory at the specified location"; 
    }
  
  cmTypeMacro(cmFileCommand, cmCommand);

protected:
  bool HandleWriteCommand(std::vector<std::string> const& args, bool append);
  bool HandleReadCommand(std::vector<std::string> const& args);
  bool HandleGlobCommand(std::vector<std::string> const& args, bool recurse);
  bool HandleMakeDirectoryCommand(std::vector<std::string> const& args);
  bool HandleInstallCommand(std::vector<std::string> const& args);
};


#endif
