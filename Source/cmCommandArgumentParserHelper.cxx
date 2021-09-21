/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmCommandArgumentParserHelper.h"

#include <cstring>
#include <iostream>
#include <sstream>
#include <utility>

#include <cm/memory>
#include <cm/optional>
#include <cmext/string_view>

#include "cmCommandArgumentLexer.h"
#include "cmListFileCache.h"
#include "cmMakefile.h"
#include "cmState.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmValue.h"

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
  auto stVal = cm::make_unique<char[]>(str.size() + 1);
  strcpy(stVal.get(), str.c_str());
  this->Variables.push_back(std::move(stVal));
  return this->Variables.back().get();
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
        return this->AddString(cmEscapeQuotes(str));
      }
      return this->AddString(str);
    }
    return "";
  }
  if (strcmp(key, "CACHE") == 0) {
    if (cmValue c =
          this->Makefile->GetState()->GetInitializedCacheValue(var)) {
      if (this->EscapeQuotes) {
        return this->AddString(cmEscapeQuotes(*c));
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
    std::string line;
    cmListFileContext const& top = this->Makefile->GetBacktrace().Top();
    if (top.DeferId) {
      line = cmStrCat("DEFERRED:"_s, *top.DeferId);
    } else {
      line = std::to_string(this->FileLine);
    }
    return this->AddString(line);
  }
  cmValue value = this->Makefile->GetDefinition(var);
  if (!value) {
    this->Makefile->MaybeWarnUninitialized(var, this->FileName);
    if (!this->RemoveEmpty) {
      return nullptr;
    }
  }
  if (this->EscapeQuotes && value) {
    return this->AddString(cmEscapeQuotes(*value));
  }
  return this->AddString(value);
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
  std::string ref = cmStrCat('@', var, '@');
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
  auto out = cm::make_unique<char[]>(len);
  strcpy(out.get(), in1);
  strcat(out.get(), in2);
  this->Variables.push_back(std::move(out));
  return this->Variables.back().get();
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
  auto out = cm::make_unique<char[]>(len + 1);
  memcpy(out.get(), str, len);
  out.get()[len] = 0;
  pt->str = out.get();
  this->Variables.push_back(std::move(out));
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

int cmCommandArgumentParserHelper::ParseString(std::string const& str,
                                               int verb)
{
  if (str.empty()) {
    return 0;
  }
  this->InputSize = str.size();
  this->Verbose = verb;

  this->Result.clear();

  yyscan_t yyscanner;
  cmCommandArgument_yylex_init(&yyscanner);
  auto* scanBuf = cmCommandArgument_yy_scan_string(str.c_str(), yyscanner);
  cmCommandArgument_yyset_extra(this, yyscanner);
  cmCommandArgument_SetupEscapes(yyscanner, this->NoEscapeMode);
  int res = cmCommandArgument_yyparse(yyscanner);
  cmCommandArgument_yy_delete_buffer(scanBuf, yyscanner);
  cmCommandArgument_yylex_destroy(yyscanner);
  if (res != 0) {
    return 0;
  }

  this->CleanupParser();

  if (this->Verbose) {
    std::cerr << "Expanding [" << str << "] produced: [" << this->Result << "]"
              << std::endl;
  }
  return 1;
}

void cmCommandArgumentParserHelper::CleanupParser()
{
  this->Variables.clear();
}

void cmCommandArgumentParserHelper::Error(const char* str)
{
  auto pos = this->InputBufferPos;
  auto const isEof = (this->InputSize < this->InputBufferPos);
  if (!isEof) {
    pos -= this->LastTokenLength;
  }

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

void cmCommandArgumentParserHelper::UpdateInputPosition(int const tokenLength)
{
  this->InputBufferPos += tokenLength;
  this->LastTokenLength = tokenLength;
}
