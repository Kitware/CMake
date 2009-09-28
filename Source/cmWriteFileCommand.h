/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmWriteFileCommand_h
#define cmWriteFileCommand_h

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
  virtual bool InitialPass(std::vector<std::string> const& args,
                           cmExecutionStatus &status);

  /**
   * This determines if the command is invoked when in script mode.
   */
  virtual bool IsScriptable() { return true; }

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() { return "write_file";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Deprecated. Use the file(WRITE ) command instead.";
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "  write_file(filename \"message to write\"... [APPEND])\n"
      "The first argument is the file name, the rest of the arguments are "
      "messages to write. If the argument APPEND is specified, then "
      "the message will be appended.\n"
      "NOTE 1: file(WRITE ... and file(APPEND ... do exactly the same as "
      "this one but add some more functionality.\n"
      "NOTE 2: When using write_file the produced file cannot be used as an "
      "input to CMake (CONFIGURE_FILE, source file ...) because it will "
      "lead to an infinite loop. Use configure_file if you want to generate "
      "input files to CMake.";
    }
  
  /** This command is kept for compatibility with older CMake versions. */
  virtual bool IsDiscouraged()
    {
    return true;
    }

  cmTypeMacro(cmWriteFileCommand, cmCommand);
};


#endif
