/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <memory>
#include <string>
#include <vector>

class cmMakefile;

class cmCommandArgumentParserHelper
{
public:
  struct ParserType
  {
    const char* str;
  };

  cmCommandArgumentParserHelper();
  ~cmCommandArgumentParserHelper();

  cmCommandArgumentParserHelper(cmCommandArgumentParserHelper const&) = delete;
  cmCommandArgumentParserHelper& operator=(
    cmCommandArgumentParserHelper const&) = delete;

  int ParseString(std::string const& str, int verb);

  // For the lexer:
  void AllocateParserType(cmCommandArgumentParserHelper::ParserType* pt,
                          const char* str, int len = 0);
  bool HandleEscapeSymbol(cmCommandArgumentParserHelper::ParserType* pt,
                          char symbol);

  void Error(const char* str);

  // For yacc
  const char* CombineUnions(const char* in1, const char* in2);

  const char* ExpandSpecialVariable(const char* key, const char* var);
  const char* ExpandVariable(const char* var);
  const char* ExpandVariableForAt(const char* var);
  void SetResult(const char* value);

  void SetMakefile(const cmMakefile* mf);

  void UpdateInputPosition(int tokenLength);

  std::string& GetResult() { return this->Result; }

  void SetLineFile(long line, const char* file);
  void SetEscapeQuotes(bool b) { this->EscapeQuotes = b; }
  void SetNoEscapeMode(bool b) { this->NoEscapeMode = b; }
  void SetReplaceAtSyntax(bool b) { this->ReplaceAtSyntax = b; }
  void SetRemoveEmpty(bool b) { this->RemoveEmpty = b; }

  const char* GetError() { return this->ErrorString.c_str(); }

private:
  std::string::size_type InputBufferPos{ 1 };
  std::string::size_type LastTokenLength{};
  std::string::size_type InputSize{};
  std::vector<char> OutputBuffer;

  void Print(const char* place, const char* str);
  void SafePrintMissing(const char* str, int line, int cnt);

  const char* AddString(const std::string& str);

  void CleanupParser();
  void SetError(std::string const& msg);

  std::vector<std::unique_ptr<char[]>> Variables;
  const cmMakefile* Makefile;
  std::string Result;
  std::string ErrorString;
  const char* FileName;
  long FileLine;
  int Verbose;
  bool EscapeQuotes;
  bool NoEscapeMode;
  bool ReplaceAtSyntax;
  bool RemoveEmpty;
};

#define YYSTYPE cmCommandArgumentParserHelper::ParserType
#define YYSTYPE_IS_DECLARED
#define YY_EXTRA_TYPE cmCommandArgumentParserHelper*
#define YY_DECL                                                               \
  int cmCommandArgument_yylex(YYSTYPE* yylvalp, yyscan_t yyscanner)
