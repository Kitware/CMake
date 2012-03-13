/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmSeparateArgumentsCommand_h
#define cmSeparateArgumentsCommand_h

#include "cmCommand.h"

/** \class cmSeparateArgumentsCommand
 * \brief separate_arguments command
 *
 * cmSeparateArgumentsCommand implements the separate_arguments CMake command
 */
class cmSeparateArgumentsCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone() 
    {
    return new cmSeparateArgumentsCommand;
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
  virtual bool IsScriptable() const { return true; }

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() const {return "separate_arguments";}
  
  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() const
    {
    return 
      "Parse space-separated arguments into a semicolon-separated list.";
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation() const
    {
    return
      "  separate_arguments(<var> <UNIX|WINDOWS>_COMMAND \"<args>\")\n"
      "Parses a unix- or windows-style command-line string \"<args>\" and "
      "stores a semicolon-separated list of the arguments in <var>.  "
      "The entire command line must be given in one \"<args>\" argument."
      "\n"
      "The UNIX_COMMAND mode separates arguments by unquoted whitespace.  "
      "It recognizes both single-quote and double-quote pairs.  "
      "A backslash escapes the next literal character (\\\" is \"); "
      "there are no special escapes (\\n is just n)."
      "\n"
      "The WINDOWS_COMMAND mode parses a windows command-line using the "
      "same syntax the runtime library uses to construct argv at startup.  "
      "It separates arguments by whitespace that is not double-quoted.  "
      "Backslashes are literal unless they precede double-quotes.  "
      "See the MSDN article \"Parsing C Command-Line Arguments\" for details."
      "\n"
      "  separate_arguments(VARIABLE)\n"
      "Convert the value of VARIABLE to a semi-colon separated list.  "
      "All spaces are replaced with ';'.  This helps with generating "
      "command lines.";
    }
  
  cmTypeMacro(cmSeparateArgumentsCommand, cmCommand);
};



#endif
