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
#ifndef cmWhileCommand_h
#define cmWhileCommand_h

#include "cmCommand.h"
#include "cmFunctionBlocker.h"
#include "cmListFileCache.h"

/** \class cmWhileFunctionBlocker
 * \brief subclass of function blocker
 *
 * 
 */
class cmWhileFunctionBlocker : public cmFunctionBlocker
{
public:
  cmWhileFunctionBlocker() {m_Executing = false;}
  virtual ~cmWhileFunctionBlocker() {}
  virtual bool IsFunctionBlocked(const cmListFileFunction& lff,
                                 cmMakefile &mf);
  virtual bool ShouldRemove(const cmListFileFunction& lff, cmMakefile &mf);
  virtual void ScopeEnded(cmMakefile &mf);
  
  std::vector<cmListFileArgument> m_Args;
  std::vector<cmListFileFunction> m_Functions;
  bool m_Executing;
};

/** \class cmWhileCommand
 * \brief starts a while loop
 *
 * cmWhileCommand starts a while loop
 */
class cmWhileCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone() 
    {
    return new cmWhileCommand;
    }

  /**
   * This overrides the default InvokeInitialPass implementation.
   * It records the arguments before expansion.
   */
  virtual bool InvokeInitialPass(const std::vector<cmListFileArgument>& args);
    
  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  virtual bool InitialPass(std::vector<std::string> const&) { return false; }

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
  virtual const char* GetName() { return "WHILE";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Evaluate a group of commands while a condition is true";
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "  WHILE(condition)\n"
      "    COMMAND1(ARGS ...)\n"
      "    COMMAND2(ARGS ...)\n"
      "    ...\n"
      "  ENDWHILE(condition)\n"
      "All commands between WHILE and the matching ENDWHILE are recorded "
      "without being invoked.  Once the ENDWHILE is evaluated, the "
      "recorded list of commands is invoked as long as the condition "
      "is true. The condition is evaulated using the same logic as the "
      "FOR command.";
    }
  
  cmTypeMacro(cmWhileCommand, cmCommand);
};


#endif
