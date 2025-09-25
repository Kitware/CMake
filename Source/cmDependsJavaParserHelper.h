/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <memory>
#include <string>
#include <vector>

/** \class cmDependsJavaParserHelper
 * \brief Helper class for parsing java source files
 *
 * Finds dependencies for java file and list of outputs
 */
class cmDependsJavaParserHelper
{
public:
  struct ParserType
  {
    char* str;
  };

  cmDependsJavaParserHelper();
  ~cmDependsJavaParserHelper();

  cmDependsJavaParserHelper(cmDependsJavaParserHelper const&) = delete;
  cmDependsJavaParserHelper& operator=(cmDependsJavaParserHelper const&) =
    delete;

  int ParseString(char const* str, int verb);
  int ParseFile(char const* file);

  // For the lexer:
  void AllocateParserType(cmDependsJavaParserHelper::ParserType* pt,
                          char const* str, int len = 0);

  int LexInput(char* buf, int maxlen);
  void Error(char const* str);

  // For yacc
  void AddClassFound(char const* sclass);
  void PrepareElement(ParserType* me);
  void DeallocateParserType(char** pt);
  void CheckEmpty(int line, int cnt, ParserType* pt);
  void StartClass(char const* cls);
  void EndClass();
  void AddPackagesImport(char const* sclass);
  void SetCurrentPackage(char const* pkg) { this->CurrentPackage = pkg; }
  char const* GetCurrentPackage() { return this->CurrentPackage.c_str(); }
  void SetCurrentCombine(char const* cmb) { this->CurrentCombine = cmb; }
  char const* GetCurrentCombine() { return this->CurrentCombine.c_str(); }
  void UpdateCombine(char const* str1, char const* str2);

  std::vector<std::string>& GetClassesFound() { return this->ClassesFound; }

  std::vector<std::string> GetFilesProduced();

private:
  class CurrentClass
  {
  public:
    std::string Name;
    std::vector<CurrentClass> NestedClasses;
    void AddFileNamesForPrinting(std::vector<std::string>* files,
                                 char const* prefix, char const* sep) const;
  };
  std::string CurrentPackage;
  std::string::size_type InputBufferPos;
  std::string InputBuffer;
  std::vector<char> OutputBuffer;
  std::vector<std::string> ClassesFound;
  std::vector<std::string> PackagesImport;
  std::string CurrentCombine;

  std::vector<CurrentClass> ClassStack;

  int CurrentLine;
  int UnionsAvailable;
  int LastClassId;
  int CurrentDepth;
  int Verbose;

  std::vector<std::unique_ptr<char[]>> Allocates;

  void PrintClasses();

  void Print(char const* place, char const* str) const;
  void CombineUnions(char** out, char const* in1, char** in2, char const* sep);
  void SafePrintMissing(char const* str, int line, int cnt);

  void CleanupParser();
};

#define YYSTYPE cmDependsJavaParserHelper::ParserType
#define YYSTYPE_IS_DECLARED
#define YY_EXTRA_TYPE cmDependsJavaParserHelper*
#define YY_DECL int cmDependsJava_yylex(YYSTYPE* yylvalp, yyscan_t yyscanner)
