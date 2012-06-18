/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
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
  virtual bool InitialPass(std::vector<std::string> const& args,
                           cmExecutionStatus &status);

  /**
   * This determines if the command is invoked when in script mode.
   */
  virtual bool IsScriptable() const { return true; }

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() const {return "set";}
  
  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() const
    {
    return "Set a CMake, cache or environment variable to a given value.";
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation() const
    {
    return
      "  set(<variable> <value>\n"
      "      [[CACHE <type> <docstring> [FORCE]] | PARENT_SCOPE])\n"
      "Within CMake sets <variable> to the value <value>.  "
      "<value> is expanded before <variable> is set to it.  "
      "Normally, set will set a regular CMake variable. "
      "If CACHE is present, then the <variable> is put in the cache "
      "instead, unless it is already in the cache. "
      "See section 'Variable types in CMake' below for details of "
      "regular and cache variables and their interactions. "
      "If CACHE is used, <type> and <docstring> are required. <type> is used "
      "by the CMake GUI to choose a widget with which the user sets a value. "
      "The value for <type> may be one of\n"
      "  FILEPATH = File chooser dialog.\n"
      "  PATH     = Directory chooser dialog.\n"
      "  STRING   = Arbitrary string.\n"
      "  BOOL     = Boolean ON/OFF checkbox.\n"
      "  INTERNAL = No GUI entry (used for persistent variables).\n"
      "If <type> is INTERNAL, the cache variable is marked as internal, "
      "and will not be shown to the user in tools like cmake-gui. "
      "This is intended for values that should be persisted in the cache, "
      "but which users should not normally change. INTERNAL implies FORCE."
      "\n"
      "Normally, set(...CACHE...) creates cache variables, but does not "
      "modify them. "
      "If FORCE is specified, the value of the cache variable is set, even "
      "if the variable is already in the cache. This should normally be "
      "avoided, as it will remove any changes to the cache variable's value "
      "by the user.\n"
      "If PARENT_SCOPE is present, the variable will be set in the scope "
      "above the current scope. Each new directory or function creates a new "
      "scope. This command will set the value of a variable into the parent "
      "directory or calling function (whichever is applicable to the case at "
      "hand). PARENT_SCOPE cannot be combined with CACHE.\n"
      "If <value> is not specified then the variable is removed "
      "instead of set.  See also: the unset() command.\n"
      "  set(<variable> <value1> ... <valueN>)\n"
      "In this case <variable> is set to a semicolon separated list of "
      "values.\n"
      "<variable> can be an environment variable such as:\n"
      "  set( ENV{PATH} /home/martink )\n"
      "in which case the environment variable will be set.\n"
      "*** Variable types in CMake ***\n"
      "In CMake there are two types of variables: normal variables and cache "
      "variables. Normal variables are meant for the internal use of the "
      "script (just like variables in most programming languages); they are "
      "not persisted across CMake runs. "
      "Cache variables (unless set with INTERNAL) are mostly intended for "
      "configuration settings where the first CMake run determines a "
      "suitable default value, which the user can then override, by editing "
      "the cache with tools such as ccmake or cmake-gui. "
      "Cache variables are stored in the CMake cache file, and "
      "are persisted across CMake runs. \n"
      "Both types can exist at the same time with the same name "
      "but different values. "
      "When ${FOO} is evaluated, CMake first looks for "
      "a normal variable 'FOO' in scope and uses it if set. "
      "If and only if no normal variable exists then it falls back to the "
      "cache variable 'FOO'.\n"
      "Some examples:\n"
      "The code 'set(FOO \"x\")' sets the normal variable 'FOO'. It does not "
      "touch the cache, but it will hide any existing cache value 'FOO'.\n"
      "The code 'set(FOO \"x\" CACHE ...)' checks for 'FOO' in the cache, "
      "ignoring any normal variable of the same name. If 'FOO' is in the "
      "cache then nothing happens to either the normal variable or the cache "
      "variable. If 'FOO' is not in the cache, then it is added to the "
      "cache.\n"
      "Finally, whenever a cache variable is added or modified by a command, "
      "CMake also *removes* the normal variable of the same name from the "
      "current scope so that an immediately following evaluation of "
      "it will expose the newly cached value.\n"
      "Normally projects should avoid using normal and cache variables of "
      "the same name, as this interaction can be hard to follow. "
      "However, in some situations it can be useful. "
      "One example (used by some projects):"
      "\n"
      "A project has a subproject in its source tree. The child project has "
      "its own CMakeLists.txt, which is included from the parent "
      "CMakeLists.txt using add_subdirectory(). "
      "Now, if the parent and the child project provide the same option "
      "(for example a compiler option), the parent gets the first chance "
      "to add a user-editable option to the cache. "
      "Normally, the child would then use the same value "
      "that the parent uses. "
      "However, it may be necessary to hard-code the value for the child "
      "project's option while still allowing the user to edit the value used "
      "by the parent project. The parent project can achieve this simply by "
      "setting a normal variable with the same name as the option in a scope "
      "sufficient to hide the option's cache variable from the child "
      "completely. The parent has already set the cache variable,  so the "
      "child's set(...CACHE...) will do nothing, and evaluating the option "
      "variable will use the value from the normal variable, which hides the "
      "cache variable.";
    }
  
  cmTypeMacro(cmSetCommand, cmCommand);
};



#endif
