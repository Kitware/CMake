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
#ifndef cmCustomCommand_h
#define cmCustomCommand_h

#include "cmStandardIncludes.h"
class cmMakefile;

/** \class cmCustomCommand
 * \brief A class to encapsulate a custom command
 *
 * cmCustomCommand encapsulates the properties of a custom command
 */
class cmCustomCommand
{
public:
  cmCustomCommand(const char *command,
                  const char* arguments,
                  std::vector<std::string> dep,
                  const char *out);
  cmCustomCommand(const char *command,
                  const char* arguments);
  cmCustomCommand() {};
  cmCustomCommand(const cmCustomCommand& r);
  
  /**
   * Use the cmMakefile's Expand commands to expand any variables in
   * this objects members.
   */
  void ExpandVariables(const cmMakefile &);

  ///! Return the command to execute with arguments
  std::string GetCommandAndArguments() const
    {return m_Command + " " + m_Arguments;}
  
  ///! Return the command to execute
  std::string GetCommand() const {return m_Command;}
  void SetCommand(const char *cmd) {m_Command = cmd;}

  ///! Return the output
  std::string GetOutput() const {return m_Output;}
  void SetOutput(const char *cm) {m_Output = cm;}

  ///! Return the comment
  std::string GetComment() const {return m_Comment;}
  void SetComment(const char *cm) {m_Comment = cm;}

  ///! Return the commands arguments
  std::string GetArguments() const {return m_Arguments;}
  void SetArguments(const char *arg) {m_Arguments = arg;}
  
  /**
   * Return the vector that holds the list of dependencies
   */
  const std::vector<std::string> &GetDepends() const {return m_Depends;}
  std::vector<std::string> &GetDepends() {return m_Depends;}
  
private:
  std::string m_Command;
  std::string m_Arguments;
  std::string m_Comment;
  std::string m_Output;
  std::vector<std::string> m_Depends;
};


#endif
