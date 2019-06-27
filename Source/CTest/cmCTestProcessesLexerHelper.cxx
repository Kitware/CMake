/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmCTestProcessesLexerHelper.h"

#include "cmCTestProcessesLexer.h"
#include "cmCTestTestHandler.h"

cmCTestProcessesLexerHelper::cmCTestProcessesLexerHelper(
  std::vector<std::vector<cmCTestTestHandler::cmCTestTestResourceRequirement>>&
    output)
  : Output(output)
{
}

bool cmCTestProcessesLexerHelper::ParseString(const std::string& value)
{
  yyscan_t lexer;
  cmCTestProcesses_yylex_init_extra(this, &lexer);

  auto state = cmCTestProcesses_yy_scan_string(value.c_str(), lexer);
  int retval = cmCTestProcesses_yylex(lexer);
  cmCTestProcesses_yy_delete_buffer(state, lexer);

  cmCTestProcesses_yylex_destroy(lexer);
  return retval == 0;
}

void cmCTestProcessesLexerHelper::SetProcessCount(unsigned int count)
{
  this->ProcessCount = count;
}

void cmCTestProcessesLexerHelper::SetResourceType(const std::string& type)
{
  this->ResourceType = type;
}

void cmCTestProcessesLexerHelper::SetNeededSlots(int count)
{
  this->NeededSlots = count;
}

void cmCTestProcessesLexerHelper::WriteRequirement()
{
  this->Process.push_back({ this->ResourceType, this->NeededSlots, 1 });
}

void cmCTestProcessesLexerHelper::WriteProcess()
{
  for (unsigned int i = 0; i < this->ProcessCount; ++i) {
    this->Output.push_back(this->Process);
  }
  this->Process.clear();
  this->ProcessCount = 1;
}
