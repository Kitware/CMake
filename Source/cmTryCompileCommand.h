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
#ifndef cmTryCompileCommand_h
#define cmTryCompileCommand_h

#include "cmCoreTryCompile.h"

/** \class cmTryCompileCommand
 * \brief Specifies where to install some files
 *
 * cmTryCompileCommand is used to test if soucre code can be compiled
 */
class cmTryCompileCommand : public cmCoreTryCompile
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone() 
    {
    return new cmTryCompileCommand;
    }

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  virtual bool InitialPass(std::vector<std::string> const& args);

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() { return "TRY_COMPILE";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Try compiling some code.";
    }

  /**
   * More documentation.  */
  virtual const char* GetFullDocumentation()
    {
    return
      "  TRY_COMPILE(RESULT_VAR bindir srcdir\n"
      "              projectName <targetname> <CMAKE_FLAGS <Flags>>\n"
      "              <OUTPUT_VARIABLE var>)\n"
      "Try compiling a program.  In this form, srcdir should contain a complete "
      "CMake project with a CMakeLists.txt file and all sources. The bindir and "
      "srcdir will not be deleted after this command is run. "
      "If <target name> is specified then build just that target "
      "otherwise the all or ALL_BUILD target is built.\n"
      "  TRY_COMPILE(RESULT_VAR bindir srcfile\n"
      "              <CMAKE_FLAGS <Flags>>\n"
      "              <COMPILE_DEFINITIONS <flags> ...>\n"
      "              <OUTPUT_VARIABLE var>)\n"
      "Try compiling a srcfile.  In this case, the user need only supply a "
      "source file.  CMake will create the appropriate CMakeLists.txt file "
      "to build the source. "
      "In this version all files in bindir/CMakeFiles/CMakeTmp, "
      "will be cleaned automatically, for debugging a --debug-trycompile can "
      "be passed to cmake to avoid the clean. Some extra flags that "
      " can be included are,  "
      "INCLUDE_DIRECTORIES, LINK_DIRECTORIES, and LINK_LIBRARIES.  "
      "COMPILE_DEFINITIONS are -Ddefinition that will be passed to the "
      "compile line.  "

      "TRY_COMPILE creates a CMakeList.txt "
      "file on the fly that looks like this:\n"
      "  ADD_DEFINITIONS( <expanded COMPILE_DEFINITIONS from calling "
      "cmake>)\n"
      "  INCLUDE_DIRECTORIES(${INCLUDE_DIRECTORIES})\n"
      "  LINK_DIRECTORIES(${LINK_DIRECTORIES})\n"
      "  ADD_EXECUTABLE(cmTryCompileExec sources)\n"
      "  TARGET_LINK_LIBRARIES(cmTryCompileExec ${LINK_LIBRARIES})\n"
      "In both versions of the command, "
      "if OUTPUT_VARIABLE is specified, then the "
      "output from the build process is stored in the given variable. "
      "Return the success or failure in "
      "RESULT_VAR. CMAKE_FLAGS can be used to pass -DVAR:TYPE=VALUE flags "
      "to the cmake that is run during the build. "
      "";
    }
  
  cmTypeMacro(cmTryCompileCommand, cmCoreTryCompile);

};


#endif
