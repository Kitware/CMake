/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

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

  bool ParseString(std::string const& value);

  void SetProcessCount(unsigned int count);
  void SetResourceType(std::string const& type);
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
