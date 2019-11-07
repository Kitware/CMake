/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmCTestResourceGroupsLexerHelper_h
#define cmCTestResourceGroupsLexerHelper_h

#include <string>
#include <vector>

#include "cmCTestTestHandler.h"

class cmCTestResourceGroupsLexerHelper
{
public:
  struct ParserType
  {
  };

  cmCTestResourceGroupsLexerHelper(
    std::vector<
      std::vector<cmCTestTestHandler::cmCTestTestResourceRequirement>>&
      output);
  ~cmCTestResourceGroupsLexerHelper() = default;

  bool ParseString(const std::string& value);

  void SetProcessCount(unsigned int count);
  void SetResourceType(const std::string& type);
  void SetNeededSlots(int count);
  void WriteRequirement();
  void WriteProcess();

private:
  std::vector<std::vector<cmCTestTestHandler::cmCTestTestResourceRequirement>>&
    Output;

  unsigned int ProcessCount = 1;
  std::string ResourceType;
  int NeededSlots;
  std::vector<cmCTestTestHandler::cmCTestTestResourceRequirement> Process;
};

#define YY_EXTRA_TYPE cmCTestResourceGroupsLexerHelper*

#endif
