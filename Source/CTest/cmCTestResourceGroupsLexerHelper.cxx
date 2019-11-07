/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmCTestResourceGroupsLexerHelper.h"

#include "cmCTestResourceGroupsLexer.h"
#include "cmCTestTestHandler.h"

cmCTestResourceGroupsLexerHelper::cmCTestResourceGroupsLexerHelper(
  std::vector<std::vector<cmCTestTestHandler::cmCTestTestResourceRequirement>>&
    output)
  : Output(output)
{
}

bool cmCTestResourceGroupsLexerHelper::ParseString(const std::string& value)
{
  yyscan_t lexer;
  cmCTestResourceGroups_yylex_init_extra(this, &lexer);

  auto state = cmCTestResourceGroups_yy_scan_string(value.c_str(), lexer);
  int retval = cmCTestResourceGroups_yylex(lexer);
  cmCTestResourceGroups_yy_delete_buffer(state, lexer);

  cmCTestResourceGroups_yylex_destroy(lexer);
  return retval == 0;
}

void cmCTestResourceGroupsLexerHelper::SetProcessCount(unsigned int count)
{
  this->ProcessCount = count;
}

void cmCTestResourceGroupsLexerHelper::SetResourceType(const std::string& type)
{
  this->ResourceType = type;
}

void cmCTestResourceGroupsLexerHelper::SetNeededSlots(int count)
{
  this->NeededSlots = count;
}

void cmCTestResourceGroupsLexerHelper::WriteRequirement()
{
  this->Process.push_back({ this->ResourceType, this->NeededSlots, 1 });
}

void cmCTestResourceGroupsLexerHelper::WriteProcess()
{
  for (unsigned int i = 0; i < this->ProcessCount; ++i) {
    this->Output.push_back(this->Process);
  }
  this->Process.clear();
  this->ProcessCount = 1;
}
