/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmCommandArgumentParserHelper_h
#define cmCommandArgumentParserHelper_h

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>
#include <vector>

class cmMakefile;

class cmCommandArgumentParserHelper
{
public:
  struct ParserType
  {
    char* str;
  };

  cmCommandArgumentParserHelper();
  ~cmCommandArgumentParserHelper();

  int ParseString(const char* str, int verb);

  // For the lexer:
  void AllocateParserType(cmCommandArgumentParserHelper::ParserType* pt,
                          const char* str, int len = 0);
  bool HandleEscapeSymbol(cmCommandArgumentParserHelper::ParserType* pt,
                          char symbol);

  int LexInput(char* buf, int maxlen);
  void Error(const char* str);

  // For yacc
  char* CombineUnions(char* in1, char* in2);

  char* ExpandSpecialVariable(const char* key, const char* var);
  char* ExpandVariable(const char* var);
  char* ExpandVariableForAt(const char* var);
  void SetResult(const char* value);

  void SetMakefile(const cmMakefile* mf);

  std::string& GetResult() { return this->Result; }

  void SetLineFile(long line, const char* file);
  void SetEscapeQuotes(bool b) { this->EscapeQuotes = b; }
  void SetNoEscapeMode(bool b) { this->NoEscapeMode = b; }
  void SetReplaceAtSyntax(bool b) { this->ReplaceAtSyntax = b; }
  void SetRemoveEmpty(bool b) { this->RemoveEmpty = b; }

  const char* GetError() { return this->ErrorString.c_str(); }
  char EmptyVariable[1];
  char DCURLYVariable[3];
  char RCURLYVariable[3];
  char ATVariable[3];
  char DOLLARVariable[3];
  char LCURLYVariable[3];
  char BSLASHVariable[3];

private:
  cmCommandArgumentParserHelper(cmCommandArgumentParserHelper const&);
  cmCommandArgumentParserHelper& operator=(
    cmCommandArgumentParserHelper const&);

  std::string::size_type InputBufferPos;
  std::string InputBuffer;
  std::vector<char> OutputBuffer;

  void Print(const char* place, const char* str);
  void SafePrintMissing(const char* str, int line, int cnt);

  char* AddString(const std::string& str);

  void CleanupParser();
  void SetError(std::string const& msg);

  std::vector<char*> Variables;
  const cmMakefile* Makefile;
  std::string Result;
  std::string ErrorString;
  const char* FileName;
  long FileLine;
  int CurrentLine;
  int Verbose;
  bool WarnUninitialized;
  bool CheckSystemVars;
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

#endif
