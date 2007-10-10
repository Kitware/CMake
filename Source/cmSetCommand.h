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
#ifndef cmSetCommand_h
#define cmSetCommand_h

#include "cmCommand.h"

/** \class cmSetCommand
 * \brief Set a CMAKE variable
 *
 * cmSetCommand sets a variable to a value with expansion.  
 */
class cmSetCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone() 
    {
    return new cmSetCommand;
    }

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
  virtual const char* GetName() {return "set";}
  
  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Set a CMAKE variable to a given value.";
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "  set(VAR [VALUE] [CACHE TYPE DOCSTRING [FORCE]])\n"
      "Within CMake sets VAR to the value VALUE.  VALUE is expanded before "
      "VAR is set to it.  If CACHE is present, then the VAR is put in the "
      "cache. TYPE and DOCSTRING are required. TYPE is used by the CMake GUI "
      "to choose a widget with which the user sets a value.  The value for "
      "TYPE may be one of\n"
      "  FILEPATH = File chooser dialog.\n"
      "  PATH     = Directory chooser dialog.\n"
      "  STRING   = Arbitrary string.\n"
      "  BOOL     = Boolean ON/OFF checkbox.\n"
      "  INTERNAL = No GUI entry (used for persistent variables).\n"
      "If TYPE is INTERNAL, then the VALUE is always written into the cache, "
      "replacing any values existing in the cache.  If it is not a cache "
      "variable, then this always writes into the current makefile. The "
      "FORCE option will overwrite the cache value removing any changes by "
      "the user.\n"
      "  set(VAR VALUE1 ... VALUEN).\n"
      "In this case VAR is set to a semicolon separated list of values.\n"
      "VAR can be an environment variable such as:\n"
      "  set( ENV{PATH} /home/martink )\n"
      "in which case the environment variable will be set.";
    }
  
  cmTypeMacro(cmSetCommand, cmCommand);
};



#endif
