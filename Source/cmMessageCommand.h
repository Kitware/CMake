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
#ifndef cmMessageCommand_h
#define cmMessageCommand_h

#include "cmStandardIncludes.h"
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
  virtual bool InitialPass(std::vector<std::string> const& args);

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() { return "MESSAGE";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Display a message to the user.";
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "MESSAGE([SEND_ERROR | STATUS | FATAL_ERROR] \"message to display\"...)\n"
      "The arguments are messages to display. If the first argument is SEND_ERROR then an error is raised. If the first argument is STATUS then the message is diaplyed in the progress line for the GUI";
    }
  
  cmTypeMacro(cmMessageCommand, cmCommand);
};


#endif
