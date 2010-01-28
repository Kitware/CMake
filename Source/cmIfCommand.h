/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmIfCommand_h
#define cmIfCommand_h

#include "cmCommand.h"
#include "cmFunctionBlocker.h"

/** \class cmIfFunctionBlocker
 * \brief subclass of function blocker
 *
 * 
 */
class cmIfFunctionBlocker : public cmFunctionBlocker
{
public:
  cmIfFunctionBlocker() {
    this->HasRun = false; this->ScopeDepth = 0; }
  virtual ~cmIfFunctionBlocker() {}
  virtual bool IsFunctionBlocked(const cmListFileFunction& lff,
                                 cmMakefile &mf,
                                 cmExecutionStatus &);
  virtual bool ShouldRemove(const cmListFileFunction& lff,
                            cmMakefile &mf);
  
  std::vector<cmListFileArgument> Args;
  std::vector<cmListFileFunction> Functions;
  bool IsBlocking;
  bool HasRun;
  unsigned int ScopeDepth;
};

/** \class cmIfCommand
 * \brief starts an if block
 *
 * cmIfCommand starts an if block
 */
class cmIfCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone() 
    {
    return new cmIfCommand;
    }

  /**
   * This overrides the default InvokeInitialPass implementation.
   * It records the arguments before expansion.
   */
  virtual bool InvokeInitialPass(const std::vector<cmListFileArgument>& args,
                                 cmExecutionStatus &);
    
  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  virtual bool InitialPass(std::vector<std::string> const&,
                           cmExecutionStatus &) { return false;};

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() { return "if";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Conditionally execute a group of commands.";
    }
  
  /**
   * This determines if the command is invoked when in script mode.
   */
  virtual bool IsScriptable() { return true; }

  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "  if(expression)\n"
      "    # then section.\n"
      "    COMMAND1(ARGS ...)\n"
      "    COMMAND2(ARGS ...)\n"
      "    ...\n"
      "  elseif(expression2)\n"
      "    # elseif section.\n"
      "    COMMAND1(ARGS ...)\n"
      "    COMMAND2(ARGS ...)\n"
      "    ...\n"
      "  else(expression)\n"
      "    # else section.\n"
      "    COMMAND1(ARGS ...)\n"
      "    COMMAND2(ARGS ...)\n"
      "    ...\n"
      "  endif(expression)\n"
      "Evaluates the given expression.  If the result is true, the commands "
      "in the THEN section are invoked.  Otherwise, the commands in the "
      "else section are invoked.  The elseif and else sections are "
      "optional. You may have multiple elseif clauses. Note that "
      "the expression in the else and endif clause is optional. Long "
      "expressions can be used and there is a traditional order of "
      "precedence. "
      "Parenthetical expressions are evaluated first followed by unary "
      "operators such as EXISTS, COMMAND, and DEFINED. "
      "Then any EQUAL, LESS, GREATER, STRLESS, STRGREATER, STREQUAL, MATCHES "
      "will be evaluated. Then NOT operators and finally AND, OR operators "
      "will be evaluated. Possible expressions are:\n"
      "  if(<constant>)\n"
      "True if the constant is 1, ON, YES, TRUE, Y, or a non-zero number.  "
      "False if the constant is 0, OFF, NO, FALSE, N, IGNORE, \"\", "
      "or ends in the suffix '-NOTFOUND'.  "
      "Named boolean constants are case-insensitive."
      "\n"
      "  if(<variable>)\n"
      "True if the variable's value is not a false constant."
      "\n"
      "  if(NOT <expression>)\n"
      "True if the expression is not true."
      "\n"
      "  if(<expr1> AND <expr2>)\n"
      "True if both expressions would be considered true individually."
      "\n"
      "  if(<expr1> OR <expr2>)\n"
      "True if either expression would be considered true individually."
      "\n"
      "  if(COMMAND command-name)\n"
      "True if the given name is a command, macro or function that can be "
      "invoked.\n"
      "  if(POLICY policy-id)\n"
      "True if the given name is an existing policy "
      "(of the form CMP<NNNN>).\n"
      "  if(TARGET target-name)\n"
      "True if the given name is an existing target, built or imported.\n"
      "  if(EXISTS file-name)\n"
      "  if(EXISTS directory-name)\n"
      "True if the named file or directory exists.  "
      "Behavior is well-defined only for full paths.\n"
      "  if(file1 IS_NEWER_THAN file2)\n"
      "True if file1 is newer than file2 or if one of the two files "
      "doesn't exist. "
      "Behavior is well-defined only for full paths.\n"
      "  if(IS_DIRECTORY directory-name)\n"
      "True if the given name is a directory.  "
      "Behavior is well-defined only for full paths.\n"
      "  if(IS_SYMLINK file-name)\n"
      "True if the given name is a symbolic link.  "
      "Behavior is well-defined only for full paths.\n"
      "  if(IS_ABSOLUTE path)\n"
      "True if the given path is an absolute path.\n"
      "  if(variable MATCHES regex)\n"
      "  if(string MATCHES regex)\n"
      "True if the given string or variable's value matches the given "
      "regular expression.\n"
      "  if(variable LESS number)\n"
      "  if(string LESS number)\n"
      "  if(variable GREATER number)\n"
      "  if(string GREATER number)\n"
      "  if(variable EQUAL number)\n"
      "  if(string EQUAL number)\n"
      "True if the given string or variable's value is a valid number and "
      "the inequality or equality is true.\n"
      "  if(variable STRLESS string)\n"
      "  if(string STRLESS string)\n"
      "  if(variable STRGREATER string)\n"
      "  if(string STRGREATER string)\n"
      "  if(variable STREQUAL string)\n"
      "  if(string STREQUAL string)\n"
      "True if the given string or variable's value is lexicographically "
      "less (or greater, or equal) than the string or variable on the right.\n"
      "  if(version1 VERSION_LESS version2)\n"
      "  if(version1 VERSION_EQUAL version2)\n"
      "  if(version1 VERSION_GREATER version2)\n"
      "Component-wise integer version number comparison (version format is "
      "major[.minor[.patch[.tweak]]]).\n"
      "  if(DEFINED variable)\n"
      "True if the given variable is defined. It does not matter if the "
      "variable is true or false just if it has been set.\n"
      "  if((expression) AND (expression OR (expression)))\n"
      "The expressions inside the parenthesis are evaluated first and "
      "then the remaining expression is evaluated as in the previous "
      "examples. Where there are nested parenthesis the innermost are "
      "evaluated as part of evaluating the expression "
      "that contains them."
      "\n"

      "The if statement was written fairly early in CMake's history "
      "and it has some convenience features that are worth covering. "
      "The if statement reduces operations until there is "
      "a single remaining value, at that point if the case "
      "insensitive value is: ON, 1, YES, TRUE, Y it returns true, if "
      "it is OFF, 0, NO, FALSE, N, NOTFOUND, *-NOTFOUND, IGNORE it "
      "will return false. \n"

      "This is fairly reasonable. The convenience feature that sometimes "
      "throws new authors is how CMake handles values that do not "
      "match the true or false list. Those values are treated as "
      "variables and are dereferenced even though they do not have "
      "the required ${} syntax. This means that if you write\n"

      "  if (boobah)\n"

      "CMake will treat it as if you wrote \n"

      "  if (${boobah})\n"

      "likewise if you write \n"

      "  if (fubar AND sol)\n"

      "CMake will conveniently treat it as \n"

      "  if (\"${fubar}\" AND \"${sol}\")\n"

      "The later is really the correct way to write it, but the "
      "former will work as well. Only some operations in the if "
      "statement have this special handling of arguments. The "
      "specific details follow: \n"

      "1) The left hand argument to MATCHES is first checked to see "
      "if it is a defined variable, if so the variable's value is "
      "used, otherwise the original value is used. \n"

      "2) If the left hand argument to MATCHES is missing it returns "
      "false without error \n"

      "3) Both left and right hand arguments to LESS GREATER EQUAL "
      "are independently tested to see if they are defined variables, "
      "if so their defined values are used otherwise the original "
      "value is used. \n"

      "4) Both left and right hand arguments to STRLESS STREQUAL "
      "STRGREATER are independently tested to see if they are defined "
      "variables, if so their defined values are used otherwise the "
      "original value is used. \n"

      "5) Both left and right hand argumemnts to VERSION_LESS "
      "VERSION_EQUAL VERSION_GREATER are independently tested to see "
      "if they are defined variables, if so their defined values are "
      "used otherwise the original value is used. \n"

      "6) The right hand argument to NOT is tested to see if it is a "
      "boolean constant, if so the value is used, otherwise it is "
      "assumed to be a variable and it is dereferenced. \n"

      "7) The left and right hand arguments to AND OR are "
      "independently tested to see if they are boolean constants, if "
      "so they are used as such, otherwise they are assumed to be "
      "variables and are dereferenced. \n"    
      ;
    }

  // this is a shared function for both If and Else to determine if the
  // arguments were valid, and if so, was the response true. If there is
  // an error, the errorString will be set.
  static bool IsTrue(const std::vector<std::string> &args, 
    std::string &errorString, cmMakefile *mf, 
    cmake::MessageType &status);
  
  // Get a definition from the makefile.  If it doesn't exist,
  // return the original string.
  static const char* GetVariableOrString(const char* str,
                                         const cmMakefile* mf);
  
  cmTypeMacro(cmIfCommand, cmCommand);
};


#endif
