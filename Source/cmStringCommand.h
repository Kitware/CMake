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
#ifndef cmStringCommand_h
#define cmStringCommand_h

#include "cmStandardIncludes.h"
#include "cmCommand.h"

/** \class cmStringCommand
 * \brief Common string operations
 *
 */
class cmStringCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone() 
    {
    return new cmStringCommand;
    }

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  virtual bool InitialPass(std::vector<std::string> const& args);

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() { return "STRING";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "String operations.";
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "STRING(REGEX MATCH <regular_expression> <output variable> <input> [<input>...])\n"
      "STRING(REGEX MATCHALL <regular_expression> <output variable> <input> [<input>...])\n"
      "STRING(REGEX REPLACE <regular_expression> <replace_expression> <output variable> <input> [<input>...])\n"
      "STRING(COMPARE EQUAL <string1> <string2> <output variable>)\n"
      "STRING(COMPARE NOTEQUAL <string1> <string2> <output variable>)\n"
      "STRING(COMPARE LESS <string1> <string2> <output variable>)\n"
      "STRING(COMPARE GREATER <string1> <string2> <output variable>)\n"
      "STRING(ASCII <number> [<number> ...] <output variable>)\n"
      "REGEX MATCH will match the regular expression once and store the match in the output variable.\n"  
      "REGEX MATCHALL will match the regular expression as many times as possible and store the matches\n"
      "               in the output variable as a list.\n"
      "REGEX REPLACE will match the regular expression as many times as possible and substitute the\n"
      "              replacement expression for the match in the output.\n"
      "COMPARE EQUAL/NOTEQUAL/LESS/GREATER will compare the strings and store true or false in the output variable.\n"
      "ASCII will convert all numbers into corresponding ASCII characters.\n";
    }
  
  cmTypeMacro(cmStringCommand, cmCommand);
protected:
  bool HandleAsciiCommand(std::vector<std::string> const& args);
  bool HandleRegexCommand(std::vector<std::string> const& args);
  bool RegexMatch(std::vector<std::string> const& args);
  bool RegexMatchAll(std::vector<std::string> const& args);
  bool RegexReplace(std::vector<std::string> const& args);
  bool HandleCompareCommand(std::vector<std::string> const& args);
  
  class RegexReplacement
  {
  public:
    RegexReplacement(const char* s): number(-1), value(s) {}
    RegexReplacement(const std::string& s): number(-1), value(s) {}
    RegexReplacement(int n): number(n), value() {}
    int number;
    std::string value;
  };
};


#endif
