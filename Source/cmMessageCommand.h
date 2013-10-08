/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmMessageCommand_h
#define cmMessageCommand_h

#include "cmCommand.h"

/** \class cmMessageCommand
 * \brief Displays a message to the user
 *
 */
class cmMessageCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone()
    {
    return new cmMessageCommand;
    }

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  virtual bool InitialPass(std::vector<std::string> const& args,
                           cmExecutionStatus &status);

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() const { return "message";}

  /**
   * This determines if the command is invoked when in script mode.
   */
  virtual bool IsScriptable() const { return true; }

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() const
    {
    return "Display a message to the user.";
    }

  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation() const
    {
    return
      "  message([<mode>] \"message to display\" ...)\n"
      "The optional <mode> keyword determines the type of message:\n"
      "  (none)         = Important information\n"
      "  STATUS         = Incidental information\n"
      "  WARNING        = CMake Warning, continue processing\n"
      "  AUTHOR_WARNING = CMake Warning (dev), continue processing\n"
      "  SEND_ERROR     = CMake Error, continue processing,\n"
      "                                but skip generation\n"
      "  FATAL_ERROR    = CMake Error, stop processing and generation\n"
      "  DEPRECATION    = CMake Deprecation Error or Warning if variable\n"
      "                   CMAKE_ERROR_DEPRECATED or CMAKE_WARN_DEPRECATED\n"
      "                   is enabled, respectively, else no message.\n"
      "The CMake command-line tool displays STATUS messages on stdout "
      "and all other message types on stderr.  "
      "The CMake GUI displays all messages in its log area.  "
      "The interactive dialogs (ccmake and CMakeSetup) show STATUS messages "
      "one at a time on a status line and other messages in interactive "
      "pop-up boxes."
      "\n"
      "CMake Warning and Error message text displays using a simple "
      "markup language.  "
      "Non-indented text is formatted in line-wrapped paragraphs delimited "
      "by newlines.  "
      "Indented text is considered pre-formatted."
      ;
    }

  cmTypeMacro(cmMessageCommand, cmCommand);
};


#endif
