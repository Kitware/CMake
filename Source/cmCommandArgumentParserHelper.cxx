/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmCommandArgumentParserHelper.h"

#include "cmCommandArgumentLexer.h"
#include "cmMakefile.h"
#include "cmState.h"
#include "cmSystemTools.h"

#include <iostream>
#include <sstream>
#include <string.h>

int cmCommandArgument_yyparse(yyscan_t yyscanner);
//
cmCommandArgumentParserHelper::cmCommandArgumentParserHelper()
{
  this->FileLine = -1;
  this->FileName = nullptr;
  this->RemoveEmpty = true;

  this->NoEscapeMode = false;
  this->ReplaceAtSyntax = false;
}

cmCommandArgumentParserHelper::~cmCommandArgumentParserHelper()
{
  this->CleanupParser();
}

void cmCommandArgumentParserHelper::SetLineFile(long line, const char* file)
{
  this->FileLine = line;
  this->FileName = file;
}

const char* cmCommandArgumentParserHelper::AddString(const std::string& str)
{
  if (str.empty()) {
    return "";
  }
  char* stVal = new char[str.size() + 1];
  strcpy(stVal, str.c_str());
  this->Variables.push_back(stVal);
  return stVal;
}

const char* cmCommandArgumentParserHelper::ExpandSpecialVariable(
  const char* key, const char* var)
{
  if (!key) {
    return this->ExpandVariable(var);
  }
  if (!var) {
    return "";
  }
  if (strcmp(key, "ENV") == 0) {
    std::string str;
    if (cmSystemTools::GetEnv(var, str)) {
      if (this->EscapeQuotes) {
        return this->AddString(cmSystemTools::EscapeQuotes(str));
      }
      return this->AddString(str);
    }
    return "";
  }
  if (strcmp(key, "CACHE") == 0) {
    if (const std::string* c =
          this->Makefile->GetState()->GetInitializedCacheValue(var)) {
      if (this->EscapeQuotes) {
        return this->AddString(cmSystemTools::EscapeQuotes(*c));
      }
      return this->AddString(*c);
    }
    return "";
  }
  std::ostringstream e;
  e << "Syntax $" << key << "{} is not supported.  "
    << "Only ${}, $ENV{}, and $CACHE{} are allowed.";
  this->SetError(e.str());
  return nullptr;
}

const char* cmCommandArgumentParserHelper::ExpandVariable(const char* var)
{
  if (!var) {
    return nullptr;
  }
  if (this->FileLine >= 0 && strcmp(var, "CMAKE_CURRENT_LIST_LINE") == 0) {
    std::ostringstream ostr;
    ostr << this->FileLine;
    return this->AddString(ostr.str());
  }
  const char* value = this->Makefile->GetDefinition(var);
  if (!value) {
    this->Makefile->MaybeWarnUninitialized(var, this->FileName);
    if (!this->RemoveEmpty) {
      return nullptr;
    }
  }
  if (this->EscapeQuotes && value) {
    return this->AddString(cmSystemTools::EscapeQuotes(value));
  }
  return this->AddString(value ? value : "");
}

const char* cmCommandArgumentParserHelper::ExpandVariableForAt(const char* var)
{
  if (this->ReplaceAtSyntax) {
    // try to expand the variable
    const char* ret = this->ExpandVariable(var);
    // if the return was 0 and we want to replace empty strings
    // then return an empty string
    if (!ret && this->RemoveEmpty) {
      return this->AddString("");
    }
    // if the ret was not 0, then return it
    if (ret) {
      return ret;
    }
  }
  // at this point we want to put it back because of one of these cases:
  // - this->ReplaceAtSyntax is false
  // - this->ReplaceAtSyntax is true, but this->RemoveEmpty is false,
  //   and the variable was not defined
  std::string ref = "@";
  ref += var;
  ref += "@";
  return this->AddString(ref);
}

const char* cmCommandArgumentParserHelper::CombineUnions(const char* in1,
                                                         const char* in2)
{
  if (!in1) {
    return in2;
  }
  if (!in2) {
    return in1;
  }
  size_t len = strlen(in1) + strlen(in2) + 1;
  char* out = new char[len];
  strcpy(out, in1);
  strcat(out, in2);
  this->Variables.push_back(out);
  return out;
}

void cmCommandArgumentParserHelper::AllocateParserType(
  cmCommandArgumentParserHelper::ParserType* pt, const char* str, int len)
{
  pt->str = nullptr;
  if (len == 0) {
    len = static_cast<int>(strlen(str));
  }
  if (len == 0) {
    return;
  }
  char* out = new char[len + 1];
  memcpy(out, str, len);
  out[len] = 0;
  pt->str = out;
  this->Variables.push_back(out);
}

bool cmCommandArgumentParserHelper::HandleEscapeSymbol(
  cmCommandArgumentParserHelper::ParserType* pt, char symbol)
{
  switch (symbol) {
    case '\\':
    case '"':
    case ' ':
    case '#':
    case '(':
    case ')':
    case '$':
    case '@':
    case '^':
      this->AllocateParserType(pt, &symbol, 1);
      break;
    case ';':
      this->AllocateParserType(pt, "\\;", 2);
      break;
    case 't':
      this->AllocateParserType(pt, "\t", 1);
      break;
    case 'n':
      this->AllocateParserType(pt, "\n", 1);
      break;
    case 'r':
      this->AllocateParserType(pt, "\r", 1);
      break;
    case '0':
      this->AllocateParserType(pt, "\0", 1);
      break;
    default: {
      std::ostringstream e;
      e << "Invalid escape sequence \\" << symbol;
      this->SetError(e.str());
    }
      return false;
  }
  return true;
}

void cmCommandArgument_SetupEscapes(yyscan_t yyscanner, bool noEscapes);

int cmCommandArgumentParserHelper::ParseString(const char* str, int verb)
{
  if (!str) {
    return 0;
  }
  this->Verbose = verb;
  this->InputBuffer = str;
  this->InputBufferPos = 0;
  this->CurrentLine = 0;

  this->Result.clear();

  yyscan_t yyscanner;
  cmCommandArgument_yylex_init(&yyscanner);
  cmCommandArgument_yyset_extra(this, yyscanner);
  cmCommandArgument_SetupEscapes(yyscanner, this->NoEscapeMode);
  int res = cmCommandArgument_yyparse(yyscanner);
  cmCommandArgument_yylex_destroy(yyscanner);
  if (res != 0) {
    return 0;
  }

  this->CleanupParser();

  if (Verbose) {
    std::cerr << "Expanding [" << str << "] produced: [" << this->Result << "]"
              << std::endl;
  }
  return 1;
}

void cmCommandArgumentParserHelper::CleanupParser()
{
  for (char* var : this->Variables) {
    delete[] var;
  }
  this->Variables.erase(this->Variables.begin(), this->Variables.end());
}

int cmCommandArgumentParserHelper::LexInput(char* buf, int maxlen)
{
  if (maxlen < 1) {
    return 0;
  }
  if (this->InputBufferPos < this->InputBuffer.size()) {
    buf[0] = this->InputBuffer[this->InputBufferPos++];
    if (buf[0] == '\n') {
      this->CurrentLine++;
    }
    return (1);
  }
  buf[0] = '\n';
  return (0);
}

void cmCommandArgumentParserHelper::Error(const char* str)
{
  unsigned long pos = static_cast<unsigned long>(this->InputBufferPos);
  std::ostringstream ostr;
  ostr << str << " (" << pos << ")";
  this->SetError(ostr.str());
}

void cmCommandArgumentParserHelper::SetMakefile(const cmMakefile* mf)
{
  this->Makefile = mf;
}

void cmCommandArgumentParserHelper::SetResult(const char* value)
{
  if (!value) {
    this->Result.clear();
    return;
  }
  this->Result = value;
}

void cmCommandArgumentParserHelper::SetError(std::string const& msg)
{
  // Keep only the first error.
  if (this->ErrorString.empty()) {
    this->ErrorString = msg;
  }
}
