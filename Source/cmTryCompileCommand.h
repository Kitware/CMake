/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
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
  virtual bool InitialPass(std::vector<std::string> const& args,
                           cmExecutionStatus &status);

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() { return "try_compile";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Try building some code.";
    }

  /**
   * More documentation.  */
  virtual const char* GetFullDocumentation()
    {
    return
      "  try_compile(RESULT_VAR <bindir> <srcdir>\n"
      "              <projectName> [targetName] [CMAKE_FLAGS flags...]\n"
      "              [OUTPUT_VARIABLE <var>])\n"
      "Try building a project.  In this form, srcdir should contain a "
      "complete CMake project with a CMakeLists.txt file and all sources. "
      "The bindir and srcdir will not be deleted after this command is run. "
      "Specify targetName to build a specific target instead of the 'all' or "
      "'ALL_BUILD' target."
      "\n"
      "  try_compile(RESULT_VAR <bindir> <srcfile>\n"
      "              [CMAKE_FLAGS flags...]\n"
      "              [COMPILE_DEFINITIONS flags...]\n"
      "              [OUTPUT_VARIABLE <var>]\n"
      "              [COPY_FILE <fileName>])\n"
      "Try building a source file into an executable.  "
      "In this form the user need only supply a source file that defines "
      "a 'main'.  "
      "CMake will create a CMakeLists.txt file to build the source "
      "as an executable.  "
      "Specify COPY_FILE to get a copy of the linked executable at the "
      "given fileName."
      "\n"
      "In this version all files in bindir/CMakeFiles/CMakeTmp "
      "will be cleaned automatically. For debugging, --debug-trycompile can "
      "be passed to cmake to avoid this clean. However, multiple sequential "
      "try_compile operations reuse this single output directory. If you "
      "use --debug-trycompile, you can only debug one try_compile call at a "
      "time. The recommended procedure is to configure with cmake all the "
      "way through once, then delete the cache entry associated with "
      "the try_compile call of interest, and then re-run cmake again with "
      "--debug-trycompile."
      "\n"
      "Some extra flags that can be included are,  "
      "INCLUDE_DIRECTORIES, LINK_DIRECTORIES, and LINK_LIBRARIES.  "
      "COMPILE_DEFINITIONS are -Ddefinition that will be passed to the "
      "compile line.  "
      "try_compile creates a CMakeList.txt "
      "file on the fly that looks like this:\n"
      "  add_definitions( <expanded COMPILE_DEFINITIONS from calling "
      "cmake>)\n"
      "  include_directories(${INCLUDE_DIRECTORIES})\n"
      "  link_directories(${LINK_DIRECTORIES})\n"
      "  add_executable(cmTryCompileExec sources)\n"
      "  target_link_libraries(cmTryCompileExec ${LINK_LIBRARIES})\n"
      "In both versions of the command, "
      "if OUTPUT_VARIABLE is specified, then the "
      "output from the build process is stored in the given variable. "
      "Return the success or failure in "
      "RESULT_VAR. CMAKE_FLAGS can be used to pass -DVAR:TYPE=VALUE flags "
      "to the cmake that is run during the build. "
      "Set variable CMAKE_TRY_COMPILE_CONFIGURATION to choose a build "
      "configuration."
      ;
    }
  
  cmTypeMacro(cmTryCompileCommand, cmCoreTryCompile);

};


#endif
