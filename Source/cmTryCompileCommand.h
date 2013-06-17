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
  virtual const char* GetName() const { return "try_compile";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() const
    {
    return "Try building some code.";
    }

  /**
   * More documentation.  */
  virtual const char* GetFullDocumentation() const
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
      "  try_compile(RESULT_VAR <bindir> <srcfile|SOURCES srcfile...>\n"
      "              [CMAKE_FLAGS flags...]\n"
      "              [COMPILE_DEFINITIONS flags...]\n"
      "              [LINK_LIBRARIES libs...]\n"
      "              [OUTPUT_VARIABLE <var>]\n"
      "              [COPY_FILE <fileName>])\n"
      "Try building an executable from one or more source files.  "
      "In this form the user need only supply one or more source files "
      "that include a definition for 'main'.  "
      "CMake will create a CMakeLists.txt file to build the source(s) "
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
      "compile line.\n"
      "The srcfile signature also accepts a LINK_LIBRARIES argument which "
      "may contain a list of libraries or IMPORTED targets which will be "
      "linked to in the generated project.  If LINK_LIBRARIES is specified "
      "as a parameter to try_compile, then any LINK_LIBRARIES passed as "
      "CMAKE_FLAGS will be ignored.\n"
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
      "The success or failure of the try_compile, i.e. TRUE or FALSE "
      "respectively, is returned in "
      "RESULT_VAR. CMAKE_FLAGS can be used to pass -DVAR:TYPE=VALUE flags "
      "to the cmake that is run during the build. "
      "Set variable CMAKE_TRY_COMPILE_CONFIGURATION to choose a build "
      "configuration."
      ;
    }

  cmTypeMacro(cmTryCompileCommand, cmCoreTryCompile);

};


#endif
