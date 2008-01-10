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
#ifndef cmVariableWatchCommand_h
#define cmVariableWatchCommand_h

#include "cmCommand.h"

class cmVariableWatchCommandHandler
{
public:
  typedef std::vector<std::string> VectorOfCommands;
  VectorOfCommands Commands;
};

/** \class cmVariableWatchCommand
 * \brief Watch when the variable changes and invoke command
 *
 */
class cmVariableWatchCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone() 
    {
    return new cmVariableWatchCommand;
    }

  //! Default constructor
  cmVariableWatchCommand();

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  virtual bool InitialPass(std::vector<std::string> const& args);

  /**
   * This determines if the command is invoked when in script mode.
   */
  virtual bool IsScriptable() { return true; }

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() { return "variable_watch";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Watch the CMake variable for change.";
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "  variable_watch(<variable name> [<command to execute>])\n"
      "If the specified variable changes, the message will be printed about "
      "the variable being changed. If the command is specified, the command "
      "will be executed. The command will receive the following arguments:"
      " COMMAND(<variable> <access> <value> <current list file> <stack>)";
    }
  
  cmTypeMacro(cmVariableWatchCommand, cmCommand);

  void VariableAccessed(const std::string& variable, int access_type,
    const char* newValue, const cmMakefile* mf);

protected:
  std::map<std::string, cmVariableWatchCommandHandler> Handlers;

  bool InCallback;
};


#endif


