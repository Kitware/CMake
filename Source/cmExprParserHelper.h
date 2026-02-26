/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <cstdint>
#include <string>
#include <vector>

class cmExprParserHelper
{
public:
  struct ParserType
  {
    std::int64_t Number;
  };

  cmExprParserHelper();
  ~cmExprParserHelper();

  int ParseString(char const* str, int verb);

  int LexInput(char* buf, int maxlen);
  void Error(char const* str);

  void SetResult(std::int64_t value);

  std::int64_t GetResult() const { return this->Result; }

  char const* GetError() { return this->ErrorString.c_str(); }

  void UnexpectedChar(char c);

  std::string const& GetWarning() const { return this->WarningString; }

  std::int64_t ShL(std::int64_t l, std::int64_t r);
  std::int64_t ShR(std::int64_t l, std::int64_t r);
  std::int64_t Add(std::int64_t l, std::int64_t r);
  std::int64_t Sub(std::int64_t l, std::int64_t r);
  std::int64_t Mul(std::int64_t l, std::int64_t r);
  std::int64_t Div(std::int64_t l, std::int64_t r);
  std::int64_t Mod(std::int64_t l, std::int64_t r);

private:
  std::string::size_type InputBufferPos;
  std::string InputBuffer;
  std::vector<char> OutputBuffer;
  int CurrentLine;
  int Verbose;

  void Print(char const* place, char const* str);

  void SetError(std::string errorString);

  std::int64_t Result;
  char const* FileName;
  long FileLine;
  std::string ErrorString;
  std::string WarningString;
};

#define YYSTYPE cmExprParserHelper::ParserType
#define YYSTYPE_IS_DECLARED
#define YY_EXTRA_TYPE cmExprParserHelper*
#define YY_DECL int cmExpr_yylex(YYSTYPE* yylvalp, yyscan_t yyscanner)
