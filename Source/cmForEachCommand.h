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
#ifndef cmForEachCommand_h
#define cmForEachCommand_h

#include "cmStandardIncludes.h"
#include "cmCommand.h"
#include "cmFunctionBlocker.h"
#include "cmListFileCache.h"

/** \class cmForEachFunctionBlocker
 * \brief subclass of function blocker
 *
 * 
 */
class cmForEachFunctionBlocker : public cmFunctionBlocker
{
public:
  cmForEachFunctionBlocker() {m_Executing = false;}
  virtual ~cmForEachFunctionBlocker() {}
  virtual bool IsFunctionBlocked(const cmListFileFunction& lff,
                                 cmMakefile &mf);
  virtual bool ShouldRemove(const cmListFileFunction& lff, cmMakefile &mf);
  virtual void ScopeEnded(cmMakefile &mf);
  
  std::vector<std::string> m_Args;
  std::vector<cmListFileFunction> m_Functions;
  bool m_Executing;
};

/** \class cmForEachCommand
 * \brief starts an if block
 *
 * cmForEachCommand starts an if block
 */
class cmForEachCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone() 
    {
    return new cmForEachCommand;
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
  virtual const char* GetName() { return "FOREACH";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Evaluate a group of commands for each value in a list.";
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "  FOREACH(loop_var arg1 arg2 ...)\n"
      "    COMMAND1(ARGS ...)\n"
      "    COMMAND2(ARGS ...)\n"
      "    ...\n"
      "  ENDFOREACH(loop_var)\n"
      "All commands between FOREACH and the matching ENDFOREACH are recorded "
      "without being invoked.  Once the ENDFOREACH is evaluated, the "
      "recorded list of commands is invoked once for each argument listed "
      "in the original FOREACH command.  Each recorded command is modified "
      "before invocation to replace any occurrence of \"${loop_var}\" with "
      "the current value in the list.";
    }
  
  cmTypeMacro(cmForEachCommand, cmCommand);
};


#endif
